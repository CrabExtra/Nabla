#ifndef __NBL_I_CPU_COMMAND_BUFFER_H_INCLUDED__
#define __NBL_I_CPU_COMMAND_BUFFER_H_INCLUDED__

#include "nbl/asset/ICommandBuffer.h"
#include "nbl/asset/ICPUBuffer.h"
#include "nbl/asset/ICPUImage.h"
#include "nbl/asset/ICPURenderpass.h"
#include "nbl/asset/ICPUFramebuffer.h"
#include "nbl/asset/ICPURenderpassIndependentPipeline.h"
#include "nbl/asset/ICPUComputePipeline.h"
#include "nbl/asset/ICPUEvent.h"
#include "nbl/asset/ICPUDescriptorSet.h"
#include "nbl/asset/ICPUPipelineLayout.h"

namespace nbl {
namespace asset
{

class ICPUCommandBuffer final :
    public IAsset,
    public ICommandBuffer<
        ICPUBuffer,
        ICPUImage,
        ICPURenderpass,
        ICPUFramebuffer,
        ICPURenderpassIndependentPipeline, // TODO change to ICPUGraphicsPipeline
        ICPUComputePipeline,
        ICPUDescriptorSet,
        ICPUPipelineLayout,
        ICPUEvent
    >
{
public:
    size_t conservativeSizeEstimate() const override { return 0ull; } // TODO

    core::smart_refctd_ptr<IAsset> clone(uint32_t _depth = ~0u) const override
    {
        return nullptr; // TODO
    }

    bool canBeRestoredFrom(const IAsset* _other) const
    {
        return false; // TODO
    }

    E_TYPE getAssetType() const override
    {
        return ET_COMMAND_BUFFER;
    }

    // TODO implement commands

private:
    void restoreFromDummy_impl(IAsset* _other, uint32_t _levelsBelow) override
    {
        // TODO
    }

    void convertToDummyObject(uint32_t referenceLevelsBelowToConvert = 0u) override
    {
        // TODO
    }
};

}
}

#endif
