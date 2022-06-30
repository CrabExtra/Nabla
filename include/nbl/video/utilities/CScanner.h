// Copyright (C) 2018-2020 - DevSH Graphics Programming Sp. z O.O.
// This file is part of the "Nabla Engine".
// For conditions of distribution and use, see copyright notice in nabla.h

#ifndef _NBL_VIDEO_C_SCANNER_H_INCLUDED_
#define _NBL_VIDEO_C_SCANNER_H_INCLUDED_


#include "nbl/video/IPhysicalDevice.h"
#include "nbl/video/utilities/IDescriptorSetCache.h"


namespace nbl::video
{

#include "nbl/builtin/glsl/scan/parameters_struct.glsl"
#include "nbl/builtin/glsl/scan/default_scheduler.glsl"
static_assert(NBL_BUILTIN_MAX_SCAN_LEVELS&0x1,"NBL_BUILTIN_MAX_SCAN_LEVELS must be odd!");

/**
Utility class to help you perform the equivalent of `std::inclusive_scan` and `std::exclusive_scan` with data on the GPU.

The basic building block is a Blelloch-Scan, the `nbl_glsl_workgroup{Add/Mul/And/Xor/Or/Min/Max}{Exclusive/Inclusive}`:
https://developer.nvidia.com/gpugems/gpugems3/part-vi-gpu-computing/chapter-39-parallel-prefix-sum-scan-cuda
https://classes.engineering.wustl.edu/cse231/core/index.php/Scan

The workgroup scan is itself probably built out of Hillis-Steele subgroup scans, we use `KHR_shader_subgroup_arithmetic` whenever available,
but fall back to our own "software" emulation of subgroup arithmetic using Hillis-Steele and some scratch shared memory.

The way the workgroup scans are combined is amongst the most advanced in its class, because it performs the scan as a single dispatch
via some clever scheduling which allows it to also be used in "indirect mode", which is when you don't know the number of elements
that you'll be scanning on the CPU side. This is why it provides two flavours of the compute shader.

The scheduling relies on two principles:
- Virtual and Persistent Workgroups
- Atomic Counters as Sempahores

# Virtual Workgroups TODO: Move this Paragraph somewhere else.
Generally speaking, launching a new workgroup has non-trivial overhead.

Also most IHVs, especially AMD have silly limits on the ranges of dispatches (like 64k workgroups), which also apply to 1D dispatches.

It becomes impossible to keep a simple 1 invocation to 1 data element relationship when processing a large buffer without reusing workgroups.

Virtual Persistent Workgroups is a $20 term for a $0.25 idea, its simply to do the following:
1. Launch a 1D dispatch "just big enough" to saturate your GPU, `SPhysicalDeviceLimits::maxResidentInvocations` helps you figure out this number
2. Make a single workgroup perform the task of multiple workgroups by repeating itself
3. Instead of relying on `gl_WorkGroupID` or `gl_GlobalInvocationID` to find your work items, use your own ID unique for the virtual workgroup

This usually has the form of
```glsl
for (uint virtualWorkgroupIndex=gl_GlobalInvocationID.x; virtualWorkgroupIndex<virtualWorkgroupCount; virtualWorkgroupIndex++)
{
   // do actual work for a single workgroup
}
```

This actually opens some avenues to abusing the system to achieve customized scheduling.

The GLSL and underlying spec give no guarantees and explicitly warn AGAINST assuming that a workgroup with a lower ID will begin executing
no later than a workgroup with a higher ID. Actually attempting to enforce this, such as this
```glsl
layout() buffer coherent Sched
{
   uint nextWorkgroup; // initial value is 0 before the dispatch
};

while (nextWorkgroup!=gl_GlobalInvocationID.x) {}
atomicMax(nextWorkgroup,gl_GlobalInvocationID.x+1);
```
has the potential to deadlock and TDR your GPU.

However if you use a global counter of dispatched workgroups in an SSBO and `atomicAdd` to assign the `virtualWorkgroupIndex`
```glsl
uint virtualWorkgroupIndex;
for ((virtualWorkgroupIndex=atomicAdd(nextWorkgroup,1u))<virtualWorkgroupCount)
{
   // do actual work for a single workgroup
}
```
the ordering of starting work is now enforced (still wont guarantee the order of completion).

# Atomic Counters as Semaphores
**/
class NBL_API CScanner final : public core::IReferenceCounted
{
	public:		
		enum E_SCAN_TYPE : uint8_t
		{
			 // computes output[n] = Sum_{i<=n}(input[i])
			 EST_INCLUSIVE = _NBL_GLSL_SCAN_TYPE_INCLUSIVE_,
			 // computes output[n] = Sum_{i<n}(input[i]), meaning first element is identity
			 EST_EXCLUSIVE = _NBL_GLSL_SCAN_TYPE_EXCLUSIVE_,
			 EST_COUNT
		};
		// Only 4 byte wide data types supported due to need to trade the via shared memory,
		// different combinations of data type and operator have different identity elements.
		// `EDT_INT` and `EO_MIN` will have `INT_MAX` as identity, while `EDT_UINT` would have `UINT_MAX`  
		enum E_DATA_TYPE : uint8_t
		{
			EDT_UINT=0u,
			EDT_INT,
			EDT_FLOAT,
			EDT_COUNT
		};
		enum E_OPERATOR : uint8_t
		{
			EO_AND = _NBL_GLSL_SCAN_OP_AND_,
			EO_XOR = _NBL_GLSL_SCAN_OP_XOR_,
			EO_OR = _NBL_GLSL_SCAN_OP_OR_,
			EO_ADD = _NBL_GLSL_SCAN_OP_ADD_,
			EO_MUL = _NBL_GLSL_SCAN_OP_MUL_,
			EO_MIN = _NBL_GLSL_SCAN_OP_MIN_,
			EO_MAX = _NBL_GLSL_SCAN_OP_MAX_,
			EO_COUNT = _NBL_GLSL_SCAN_OP_COUNT_
		};

		//
		struct Parameters : nbl_glsl_scan_Parameters_t
		{
			static inline constexpr uint32_t MaxScanLevels = NBL_BUILTIN_MAX_SCAN_LEVELS;

			Parameters()
			{
				std::fill_n(lastElement,MaxScanLevels/2+1,0u);
				std::fill_n(temporaryStorageOffset,MaxScanLevels/2,0u);
			}
			Parameters(const uint32_t _elementCount, const uint32_t workgroupSize) : Parameters()
			{
				assert(_elementCount!=0u && "Input element count can't be 0!");
				const auto maxReductionLog2 = core::findMSB(workgroupSize)*(MaxScanLevels/2u+1u);
				assert(maxReductionLog2>=32u||((_elementCount-1u)>>maxReductionLog2)==0u && "Can't scan this many elements with such small workgroups!");

				lastElement[0u] = _elementCount-1u;
				for (topLevel=0u; lastElement[topLevel]>=workgroupSize;)
					temporaryStorageOffset[topLevel-1u] = lastElement[++topLevel] = lastElement[topLevel]/workgroupSize;
				
				std::exclusive_scan(temporaryStorageOffset,temporaryStorageOffset+sizeof(temporaryStorageOffset)/sizeof(uint32_t),temporaryStorageOffset,0u);
			}

			inline uint32_t getScratchSize(uint32_t ssboAlignment=256u)
			{
				uint32_t uint_count = 1u; // workgroup enumerator
				uint_count += temporaryStorageOffset[MaxScanLevels/2u-1u]; // last scratch offset
				uint_count += lastElement[topLevel]+1u; // and its size
				return core::roundUp<uint32_t>(uint_count*sizeof(uint32_t),ssboAlignment);
			}
		};
		struct SchedulerParameters : nbl_glsl_scan_DefaultSchedulerParameters_t
		{
			SchedulerParameters()
			{
				std::fill_n(finishedFlagOffset,Parameters::MaxScanLevels-1,0u);
				std::fill_n(cumulativeWorkgroupCount,Parameters::MaxScanLevels,0u);
			}
			SchedulerParameters(Parameters& outScanParams, const uint32_t _elementCount, const uint32_t workgroupSize) : SchedulerParameters()
			{
				outScanParams = Parameters(_elementCount,workgroupSize);
				const auto topLevel = outScanParams.topLevel;

				std::copy_n(outScanParams.lastElement+1u,topLevel,cumulativeWorkgroupCount);
				for (auto i=0u; i<=topLevel; i++)
					cumulativeWorkgroupCount[i] += 1u;
				std::reverse_copy(cumulativeWorkgroupCount,cumulativeWorkgroupCount+topLevel,cumulativeWorkgroupCount+topLevel+1u);

				std::copy_n(cumulativeWorkgroupCount+1u,topLevel,finishedFlagOffset);
				std::copy_n(cumulativeWorkgroupCount+topLevel,topLevel,finishedFlagOffset+topLevel);

				const auto finishedFlagCount = sizeof(finishedFlagOffset)/sizeof(uint32_t);
				const auto finishedFlagsSize = std::accumulate(finishedFlagOffset,finishedFlagOffset+finishedFlagCount,0u);
				std::exclusive_scan(finishedFlagOffset,finishedFlagOffset+finishedFlagCount,finishedFlagOffset,0u);
				for (auto i=0u; i<sizeof(Parameters::temporaryStorageOffset)/sizeof(uint32_t); i++)
					outScanParams.temporaryStorageOffset[i] += finishedFlagsSize;
					
				std::inclusive_scan(cumulativeWorkgroupCount,cumulativeWorkgroupCount+Parameters::MaxScanLevels,cumulativeWorkgroupCount);
			}
		};
		struct DefaultPushConstants
		{
			Parameters scanParams;
			SchedulerParameters schedulerParams;
		};
		struct DispatchInfo
		{
			DispatchInfo() : wg_count(0u)
			{
			}
			DispatchInfo(const IPhysicalDevice::SLimits& limits, const uint32_t elementCount, const uint32_t workgroupSize)
			{
				constexpr auto workgroupSpinningProtection = 4u;
				wg_count = limits.computeOptimalPersistentWorkgroupDispatchSize(elementCount,workgroupSize,workgroupSpinningProtection);
			}

			uint32_t wg_count;
		};

		//
		CScanner(core::smart_refctd_ptr<ILogicalDevice>&& device) : CScanner(std::move(device),core::roundDownToPoT(device->getPhysicalDevice()->getLimits().maxOptimallyResidentWorkgroupInvocations)) {}
		//
		CScanner(core::smart_refctd_ptr<ILogicalDevice>&& device, const uint32_t workgroupSize) : m_device(std::move(device)), m_workgroupSize(workgroupSize)
		{
			assert(core::isPoT(m_workgroupSize));

			const asset::SPushConstantRange pc_range = { asset::IShader::ESS_COMPUTE,0u,sizeof(DefaultPushConstants) };
			const IGPUDescriptorSetLayout::SBinding bindings[2] = {
				{ 0u, asset::EDT_STORAGE_BUFFER, 1u, video::IGPUShader::ESS_COMPUTE, nullptr }, // main buffer
				{ 1u, asset::EDT_STORAGE_BUFFER, 1u, video::IGPUShader::ESS_COMPUTE, nullptr } // scratch
			};

			m_ds_layout = m_device->createDescriptorSetLayout(bindings,bindings+sizeof(bindings)/sizeof(IGPUDescriptorSetLayout::SBinding));
			m_pipeline_layout = m_device->createPipelineLayout(&pc_range,&pc_range+1,core::smart_refctd_ptr(m_ds_layout));
		}

		//
		inline auto getDefaultDescriptorSetLayout() const { return m_ds_layout.get(); }

		//
		inline auto getDefaultPipelineLayout() const { return m_pipeline_layout.get(); }

		// You need to override this shader with your own defintion of `nbl_glsl_scan_getIndirectElementCount` for it to even compile, so we always give you a new shader
		core::smart_refctd_ptr<asset::ICPUShader> getIndirectShader(const E_SCAN_TYPE scanType, const E_DATA_TYPE dataType, const E_OPERATOR op) const
		{
			return createShader(true,scanType,dataType,op);
		}

		//
		inline asset::ICPUShader* getDefaultShader(const E_SCAN_TYPE scanType, const E_DATA_TYPE dataType, const E_OPERATOR op)
		{
			if (!m_shaders[scanType][dataType][op])
				m_shaders[scanType][dataType][op] = createShader(false,scanType,dataType,op);
			return m_shaders[scanType][dataType][op].get();
		}
		//
		inline IGPUSpecializedShader* getDefaultSpecializedShader(const E_SCAN_TYPE scanType, const E_DATA_TYPE dataType, const E_OPERATOR op)
		{
			if (!m_specialized_shaders[scanType][dataType][op])
			{
				auto cpuShader = core::smart_refctd_ptr<asset::ICPUShader>(getDefaultShader(scanType,dataType,op));
				cpuShader->setFilePathHint("nbl/builtin/glsl/scan/direct.comp");
				cpuShader->setShaderStage(asset::IShader::ESS_COMPUTE);

				auto gpushader = m_device->createShader(std::move(cpuShader));

				m_specialized_shaders[scanType][dataType][op] = m_device->createSpecializedShader(
					gpushader.get(),{nullptr,nullptr,"main"});
				// , asset::IShader::ESS_COMPUTE, "nbl/builtin/glsl/scan/direct.comp"
			}
			return m_specialized_shaders[scanType][dataType][op].get();
		}

		//
		inline auto getDefaultPipeline(const E_SCAN_TYPE scanType, const E_DATA_TYPE dataType, const E_OPERATOR op)
		{
			// ondemand
			if (!m_pipelines[scanType][dataType][op])
				m_pipelines[scanType][dataType][op] = m_device->createComputePipeline(
					nullptr,core::smart_refctd_ptr(m_pipeline_layout),
					core::smart_refctd_ptr<IGPUSpecializedShader>(getDefaultSpecializedShader(scanType,dataType,op))
				);
			return m_pipelines[scanType][dataType][op].get();
		}

		//
		inline uint32_t getWorkgroupSize() const {return m_workgroupSize;}

		//
		inline void buildParameters(const uint32_t elementCount, DefaultPushConstants& pushConstants, DispatchInfo& dispatchInfo)
		{
			pushConstants.schedulerParams = SchedulerParameters(pushConstants.scanParams,elementCount,m_workgroupSize);
			dispatchInfo = DispatchInfo(m_device->getPhysicalDevice()->getLimits(),elementCount,m_workgroupSize);
		}

		//
		static inline void updateDescriptorSet(ILogicalDevice* device, IGPUDescriptorSet* set, const asset::SBufferRange<IGPUBuffer>& input_range, const asset::SBufferRange<IGPUBuffer>& scratch_range)
		{
			IGPUDescriptorSet::SDescriptorInfo infos[2];
			infos[0].desc = input_range.buffer;
			infos[0].buffer = {input_range.offset,input_range.size};
			infos[1].desc = scratch_range.buffer;
			infos[1].buffer = {scratch_range.offset,scratch_range.size};

			video::IGPUDescriptorSet::SWriteDescriptorSet writes[2];
			for (auto i=0u; i<2u; i++)
			{
				writes[i].dstSet = set;
				writes[i].binding = i;
				writes[i].arrayElement = 0u;
				writes[i].count = 1u;
				writes[i].descriptorType = asset::EDT_STORAGE_BUFFER;
				writes[i].info = infos+i;
			}

			device->updateDescriptorSets(2,writes,0u,nullptr);
		}
		inline void updateDescriptorSet(IGPUDescriptorSet* set, const asset::SBufferRange<IGPUBuffer>& input_range, const asset::SBufferRange<IGPUBuffer>& scratch_range)
		{
			updateDescriptorSet(m_device.get(),set,input_range,scratch_range);
		}

		// Half and sizeof(uint32_t) of the scratch buffer need to be cleared to 0s
		static inline void dispatchHelper(
			IGPUCommandBuffer* cmdbuf, const video::IGPUPipelineLayout* pipeline_layout, const DefaultPushConstants& pushConstants, const DispatchInfo& dispatchInfo,
			const asset::E_PIPELINE_STAGE_FLAGS srcStageMask, const uint32_t srcBufferBarrierCount, const IGPUCommandBuffer::SBufferMemoryBarrier* srcBufferBarriers,
			const asset::E_PIPELINE_STAGE_FLAGS dstStageMask, const uint32_t dstBufferBarrierCount, const IGPUCommandBuffer::SBufferMemoryBarrier* dstBufferBarriers
		)
		{
			cmdbuf->pushConstants(pipeline_layout,asset::IShader::ESS_COMPUTE,0u,sizeof(DefaultPushConstants),&pushConstants);
			if (srcStageMask!=asset::E_PIPELINE_STAGE_FLAGS::EPSF_TOP_OF_PIPE_BIT&&srcBufferBarrierCount)
				cmdbuf->pipelineBarrier(srcStageMask,asset::EPSF_COMPUTE_SHADER_BIT,asset::EDF_NONE,0u,nullptr,srcBufferBarrierCount,srcBufferBarriers,0u,nullptr);
			cmdbuf->dispatch(dispatchInfo.wg_count,1u,1u);
			if (dstStageMask!=asset::E_PIPELINE_STAGE_FLAGS::EPSF_BOTTOM_OF_PIPE_BIT&&dstBufferBarrierCount)
				cmdbuf->pipelineBarrier(asset::EPSF_COMPUTE_SHADER_BIT,dstStageMask,asset::EDF_NONE,0u,nullptr,dstBufferBarrierCount,dstBufferBarriers,0u,nullptr);
		}

		inline ILogicalDevice* getDevice() const {return m_device.get();}

    protected:
		~CScanner()
		{
			// all drop themselves automatically
		}

		core::smart_refctd_ptr<asset::ICPUShader> createShader(const bool indirect, const E_SCAN_TYPE scanType, const E_DATA_TYPE dataType, const E_OPERATOR op) const;


		core::smart_refctd_ptr<ILogicalDevice> m_device;
		core::smart_refctd_ptr<IGPUDescriptorSetLayout> m_ds_layout;
		core::smart_refctd_ptr<IGPUPipelineLayout> m_pipeline_layout;
		core::smart_refctd_ptr<asset::ICPUShader> m_shaders[EST_COUNT][EDT_COUNT][EO_COUNT];
		core::smart_refctd_ptr<IGPUSpecializedShader> m_specialized_shaders[EST_COUNT][EDT_COUNT][EO_COUNT];
		core::smart_refctd_ptr<IGPUComputePipeline> m_pipelines[EST_COUNT][EDT_COUNT][EO_COUNT];
		const uint32_t m_workgroupSize;
};


}

#endif
