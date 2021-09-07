#ifndef __NBL_C_OPENGL_COMMAND_BUFFER_H_INCLUDED__
#define __NBL_C_OPENGL_COMMAND_BUFFER_H_INCLUDED__

#include "nbl/core/declarations.h"

#include <variant>

#include "nbl/video/IGPUCommandBuffer.h"
#include "nbl/video/IOpenGL_FunctionTable.h"
#include "nbl/video/SOpenGLContextLocalCache.h"
#include "nbl/video/IGPUMeshBuffer.h"
#include "nbl/video/IQueryPool.h"
#include "nbl/video/COpenGLCommandPool.h"

namespace nbl::video
{

namespace impl
{
#define _NBL_COMMAND_TYPES_LIST \
    ECT_BIND_INDEX_BUFFER,\
    ECT_DRAW,\
    ECT_DRAW_INDEXED,\
    ECT_DRAW_INDIRECT,\
    ECT_DRAW_INDEXED_INDIRECT,\
\
    ECT_SET_VIEWPORT,\
\
    ECT_SET_LINE_WIDTH,\
    ECT_SET_DEPTH_BIAS,\
\
    ECT_SET_BLEND_CONSTANTS,\
\
    ECT_COPY_BUFFER,\
    ECT_COPY_IMAGE,\
    ECT_COPY_BUFFER_TO_IMAGE,\
    ECT_COPY_IMAGE_TO_BUFFER,\
    ECT_BLIT_IMAGE,\
    ECT_RESOLVE_IMAGE,\
\
    ECT_BIND_VERTEX_BUFFERS,\
\
    ECT_SET_SCISSORS,\
    ECT_SET_DEPTH_BOUNDS,\
    ECT_SET_STENCIL_COMPARE_MASK,\
    ECT_SET_STENCIL_WRITE_MASK,\
    ECT_SET_STENCIL_REFERENCE,\
\
    ECT_DISPATCH,\
    ECT_DISPATCH_INDIRECT,\
    ECT_DISPATCH_BASE,\
\
    ECT_SET_EVENT,\
    ECT_RESET_EVENT,\
    ECT_WAIT_EVENTS,\
\
    ECT_PIPELINE_BARRIER,\
\
    ECT_BEGIN_RENDERPASS,\
    ECT_NEXT_SUBPASS,\
    ECT_END_RENDERPASS,\
\
    ECT_SET_DEVICE_MASK,\
\
    ECT_BIND_GRAPHICS_PIPELINE,\
    ECT_BIND_COMPUTE_PIPELINE,\
\
    ECT_RESET_QUERY_POOL,\
    ECT_BEGIN_QUERY,\
    ECT_END_QUERY,\
    ECT_COPY_QUERY_POOL_RESULTS,\
    ECT_WRITE_TIMESTAMP,\
    ECT_BEGIN_QUERY_INDEXED,\
    ECT_END_QUERY_INDEXED,\
\
    ECT_BIND_DESCRIPTOR_SETS,\
\
    ECT_PUSH_CONSTANTS,\
\
    ECT_CLEAR_COLOR_IMAGE,\
    ECT_CLEAR_DEPTH_STENCIL_IMAGE,\
    ECT_CLEAR_ATTACHMENTS,\
    ECT_FILL_BUFFER,\
    ECT_UPDATE_BUFFER,\
    ECT_EXECUTE_COMMANDS,\
    ECT_REGENERATE_MIPMAPS

    enum E_COMMAND_TYPE
    {
        _NBL_COMMAND_TYPES_LIST
    };

    template <E_COMMAND_TYPE ECT>
    struct SCmd_base
    {
        static inline constexpr E_COMMAND_TYPE type = ECT;
    };
    template <E_COMMAND_TYPE ECT>
    struct SCmd : SCmd_base<ECT>
    {

    };
#define _NBL_DEFINE_SCMD_SPEC(ECT) template<> struct SCmd<ECT> : SCmd_base<ECT>
    _NBL_DEFINE_SCMD_SPEC(ECT_BIND_INDEX_BUFFER)
    {
        core::smart_refctd_ptr<const IGPUBuffer> buffer;
        size_t offset;
        asset::E_INDEX_TYPE indexType;
    };
    _NBL_DEFINE_SCMD_SPEC(ECT_DRAW)
    {
        uint32_t vertexCount;
        uint32_t instanceCount;
        uint32_t firstVertex;
        uint32_t firstInstance;
    };
    _NBL_DEFINE_SCMD_SPEC(ECT_DRAW_INDEXED)
    {
        uint32_t indexCount;
        uint32_t instanceCount;
        uint32_t firstIndex;
        uint32_t vertexOffset;
        uint32_t firstInstance;
    };
    _NBL_DEFINE_SCMD_SPEC(ECT_DRAW_INDIRECT)
    {
        core::smart_refctd_ptr<const IGPUBuffer> buffer;
        size_t offset;
        uint32_t drawCount;
        uint32_t stride;
    };
    _NBL_DEFINE_SCMD_SPEC(ECT_DRAW_INDEXED_INDIRECT)
    {
        core::smart_refctd_ptr<const IGPUBuffer> buffer;
        size_t offset;
        uint32_t drawCount;
        uint32_t stride;
    };
    _NBL_DEFINE_SCMD_SPEC(ECT_SET_VIEWPORT)
    {
        uint32_t firstViewport;
        uint32_t viewportCount;
        const asset::SViewport* viewports;
    };
    _NBL_DEFINE_SCMD_SPEC(ECT_SET_LINE_WIDTH)
    {
        float lineWidth;
    };
    _NBL_DEFINE_SCMD_SPEC(ECT_SET_DEPTH_BIAS)
    {
        float depthBiasConstantFactor;
        float depthBiasClamp;
        float depthBiasSlopeFactor;
    };
    _NBL_DEFINE_SCMD_SPEC(ECT_SET_BLEND_CONSTANTS)
    {
        float blendConstants[4];
    };
    _NBL_DEFINE_SCMD_SPEC(ECT_COPY_BUFFER)
    {
        core::smart_refctd_ptr<const IGPUBuffer> srcBuffer;
        core::smart_refctd_ptr<IGPUBuffer> dstBuffer;
        uint32_t regionCount;
        const asset::SBufferCopy* regions;
    };
    _NBL_DEFINE_SCMD_SPEC(ECT_COPY_IMAGE)
    {
        core::smart_refctd_ptr<const IGPUImage> srcImage;
        asset::E_IMAGE_LAYOUT srcImageLayout;
        core::smart_refctd_ptr<IGPUImage> dstImage;
        asset::E_IMAGE_LAYOUT dstImageLayout;
        uint32_t regionCount;
        const asset::IImage::SImageCopy* regions;
    };
    _NBL_DEFINE_SCMD_SPEC(ECT_COPY_BUFFER_TO_IMAGE)
    {
        core::smart_refctd_ptr<const IGPUBuffer> srcBuffer;
        core::smart_refctd_ptr<IGPUImage> dstImage;
        asset::E_IMAGE_LAYOUT dstImageLayout;
        uint32_t regionCount;
        const asset::IImage::SBufferCopy* regions;
    };
    _NBL_DEFINE_SCMD_SPEC(ECT_COPY_IMAGE_TO_BUFFER)
    {
        core::smart_refctd_ptr<const IGPUImage> srcImage;
        asset::E_IMAGE_LAYOUT srcImageLayout;
        core::smart_refctd_ptr<IGPUBuffer> dstBuffer;
        uint32_t regionCount;
        const asset::IImage::SBufferCopy* regions;
    };
    _NBL_DEFINE_SCMD_SPEC(ECT_BLIT_IMAGE)
    {
        core::smart_refctd_ptr<const IGPUImage> srcImage;
        asset::E_IMAGE_LAYOUT srcImageLayout;
        core::smart_refctd_ptr<IGPUImage> dstImage;
        asset::E_IMAGE_LAYOUT dstImageLayout;
        uint32_t regionCount;
        const asset::SImageBlit* regions;
        asset::ISampler::E_TEXTURE_FILTER filter;
    };
    _NBL_DEFINE_SCMD_SPEC(ECT_RESOLVE_IMAGE)
    {
        core::smart_refctd_ptr<const IGPUImage> srcImage;
        asset::E_IMAGE_LAYOUT srcImageLayout;
        core::smart_refctd_ptr<IGPUImage> dstImage;
        asset::E_IMAGE_LAYOUT dstImageLayout;
        uint32_t regionCount;
        const asset::SImageResolve* regions;
    };
    _NBL_DEFINE_SCMD_SPEC(ECT_BIND_VERTEX_BUFFERS)
    {
        uint32_t first;
        uint32_t count;
        core::smart_refctd_ptr<const IGPUBuffer> buffers[asset::SVertexInputParams::MAX_ATTR_BUF_BINDING_COUNT];
        size_t offsets[asset::SVertexInputParams::MAX_ATTR_BUF_BINDING_COUNT];
    };
    _NBL_DEFINE_SCMD_SPEC(ECT_SET_SCISSORS)
    {
        uint32_t firstScissor;
        uint32_t scissorCount;
        VkRect2D* scissors;
    };
    _NBL_DEFINE_SCMD_SPEC(ECT_SET_DEPTH_BOUNDS)
    {
        float minDepthBounds;
        float maxDepthBounds;
    };
    _NBL_DEFINE_SCMD_SPEC(ECT_SET_STENCIL_COMPARE_MASK)
    {
        asset::E_STENCIL_FACE_FLAGS faceMask;
        uint32_t cmpMask;
    };
    _NBL_DEFINE_SCMD_SPEC(ECT_SET_STENCIL_WRITE_MASK)
    {
        asset::E_STENCIL_FACE_FLAGS faceMask;
        uint32_t writeMask;
    };
    _NBL_DEFINE_SCMD_SPEC(ECT_SET_STENCIL_REFERENCE)
    {
        asset::E_STENCIL_FACE_FLAGS faceMask;
        uint32_t reference;
    };
    _NBL_DEFINE_SCMD_SPEC(ECT_DISPATCH)
    {
        uint32_t groupCountX, groupCountY, groupCountZ;
    };
    _NBL_DEFINE_SCMD_SPEC(ECT_DISPATCH_INDIRECT)
    {
        core::smart_refctd_ptr<const IGPUBuffer> buffer;
        size_t offset;
    };
    _NBL_DEFINE_SCMD_SPEC(ECT_DISPATCH_BASE)
    {
        uint32_t baseGroupX, baseGroupY, baseGroupZ;
        uint32_t groupCountX, groupCountY, groupCountZ;
    };
    _NBL_DEFINE_SCMD_SPEC(ECT_SET_EVENT)
    {
        core::smart_refctd_ptr<IGPUEvent> event;
        GLbitfield barrierBits;
    };
    _NBL_DEFINE_SCMD_SPEC(ECT_RESET_EVENT)
    {
        core::smart_refctd_ptr<IGPUEvent> event;
        asset::E_PIPELINE_STAGE_FLAGS stageMask;
    };
    _NBL_DEFINE_SCMD_SPEC(ECT_WAIT_EVENTS)
    {
        GLbitfield barrier;
    };
    _NBL_DEFINE_SCMD_SPEC(ECT_PIPELINE_BARRIER)
    {
        GLbitfield barrier;
    };
    _NBL_DEFINE_SCMD_SPEC(ECT_BEGIN_RENDERPASS)
    {
        IGPUCommandBuffer::SRenderpassBeginInfo renderpassBegin;
        asset::E_SUBPASS_CONTENTS content;
    };
    _NBL_DEFINE_SCMD_SPEC(ECT_NEXT_SUBPASS)
    {
        asset::E_SUBPASS_CONTENTS contents;
    };
    _NBL_DEFINE_SCMD_SPEC(ECT_END_RENDERPASS)
    {
        // no parameters
    };
    _NBL_DEFINE_SCMD_SPEC(ECT_SET_DEVICE_MASK)
    {
        uint32_t deviceMask;
    };
    _NBL_DEFINE_SCMD_SPEC(ECT_BIND_GRAPHICS_PIPELINE)
    {
        core::smart_refctd_ptr<const IGPUGraphicsPipeline> pipeline;
    };
    _NBL_DEFINE_SCMD_SPEC(ECT_BIND_COMPUTE_PIPELINE)
    {
        core::smart_refctd_ptr<const IGPUComputePipeline> pipeline;
    };
    _NBL_DEFINE_SCMD_SPEC(ECT_RESET_QUERY_POOL)
    {
        core::smart_refctd_ptr<const IQueryPool> queryPool;
        uint32_t query;
        uint32_t queryCount;
    };
    _NBL_DEFINE_SCMD_SPEC(ECT_BEGIN_QUERY)
    {
        core::smart_refctd_ptr<const IQueryPool> queryPool;
        uint32_t query;
        IQueryPool::E_QUERY_CONTROL_FLAGS flags;
    };
    _NBL_DEFINE_SCMD_SPEC(ECT_END_QUERY)
    {
        core::smart_refctd_ptr<const IQueryPool> queryPool;
        uint32_t query;
    };
    _NBL_DEFINE_SCMD_SPEC(ECT_COPY_QUERY_POOL_RESULTS)
    {
        core::smart_refctd_ptr<const IQueryPool> queryPool;
        uint32_t firstQuery;
        uint32_t queryCount;
        core::smart_refctd_ptr<const IGPUBuffer> dstBuffer;
        size_t dstOffset;
        size_t stride;
        IQueryPool::E_QUERY_RESULTS_FLAGS flags;
    };
    _NBL_DEFINE_SCMD_SPEC(ECT_WRITE_TIMESTAMP)
    {
        core::smart_refctd_ptr<const IQueryPool> queryPool;
        asset::E_PIPELINE_STAGE_FLAGS pipelineStage;
        uint32_t query;
    };
    _NBL_DEFINE_SCMD_SPEC(ECT_BEGIN_QUERY_INDEXED)
    {
        core::smart_refctd_ptr<const IQueryPool> queryPool;
        uint32_t query;
        uint32_t index;
        IQueryPool::E_QUERY_CONTROL_FLAGS flags;
    };
    _NBL_DEFINE_SCMD_SPEC(ECT_END_QUERY_INDEXED)
    {
        core::smart_refctd_ptr<const IQueryPool> queryPool;
        uint32_t query;
        uint32_t index;
    };
    _NBL_DEFINE_SCMD_SPEC(ECT_BIND_DESCRIPTOR_SETS)
    {
        asset::E_PIPELINE_BIND_POINT pipelineBindPoint;
        core::smart_refctd_ptr<const IGPUPipelineLayout> layout;
        uint32_t firstSet;
        uint32_t dsCount;
        core::smart_refctd_ptr<const IGPUDescriptorSet> descriptorSets[IGPUPipelineLayout::DESCRIPTOR_SET_COUNT];
        core::smart_refctd_dynamic_array<uint32_t> dynamicOffsets;

        SCmd() = default;
        SCmd<ECT_BIND_DESCRIPTOR_SETS>& operator=(SCmd<ECT_BIND_DESCRIPTOR_SETS>&& rhs)
        {
            pipelineBindPoint = rhs.pipelineBindPoint;
            layout = std::move(rhs.layout);
            firstSet = rhs.firstSet;
            dsCount = rhs.dsCount;
            dynamicOffsets = std::move(dynamicOffsets);
            for (uint32_t i = 0u; i < IGPUPipelineLayout::DESCRIPTOR_SET_COUNT; ++i)
                descriptorSets[i] = std::move(rhs.descriptorSets[i]);
            return *this;
        }
        SCmd(SCmd<ECT_BIND_DESCRIPTOR_SETS>&& rhs)
        {
            operator=(std::move(rhs));
        }
    };
    _NBL_DEFINE_SCMD_SPEC(ECT_PUSH_CONSTANTS)
    {
        constexpr static inline uint32_t MAX_PUSH_CONSTANT_BYTESIZE = 128u;

        core::smart_refctd_ptr<const IGPUPipelineLayout> layout;
        std::underlying_type_t<asset::ISpecializedShader::E_SHADER_STAGE> stageFlags;
        uint32_t offset;
        uint32_t size;
        uint8_t values[MAX_PUSH_CONSTANT_BYTESIZE];
    };
    _NBL_DEFINE_SCMD_SPEC(ECT_CLEAR_COLOR_IMAGE)
    {
        core::smart_refctd_ptr<IGPUImage> image;
        asset::E_IMAGE_LAYOUT imageLayout;
        asset::SClearColorValue color;
        uint32_t rangeCount;
        const asset::IImage::SSubresourceRange* ranges;
    };
    _NBL_DEFINE_SCMD_SPEC(ECT_CLEAR_DEPTH_STENCIL_IMAGE)
    {
        core::smart_refctd_ptr<IGPUImage> image;
        asset::E_IMAGE_LAYOUT imageLayout;
        asset::SClearDepthStencilValue depthStencil;
        uint32_t rangeCount;
        const asset::IImage::SSubresourceRange* ranges;
    };
    _NBL_DEFINE_SCMD_SPEC(ECT_CLEAR_ATTACHMENTS)
    {
        uint32_t attachmentCount;
        const asset::SClearAttachment* attachments;
        uint32_t rectCount;
        const asset::SClearRect* rects;
    };
    _NBL_DEFINE_SCMD_SPEC(ECT_FILL_BUFFER)
    {
        core::smart_refctd_ptr<IGPUBuffer> dstBuffer;
        size_t dstOffset;
        size_t size;
        uint32_t data;
    };
    _NBL_DEFINE_SCMD_SPEC(ECT_UPDATE_BUFFER)
    {
        core::smart_refctd_ptr<IGPUBuffer> dstBuffer;
        size_t dstOffset;
        size_t dataSize;
        const void* data;
    };
    _NBL_DEFINE_SCMD_SPEC(ECT_EXECUTE_COMMANDS)
    {
        core::smart_refctd_ptr<IGPUCommandBuffer> cmdbuf; // secondary!!!
    };
    _NBL_DEFINE_SCMD_SPEC(ECT_REGENERATE_MIPMAPS)
    {
        core::smart_refctd_ptr<IGPUImageView> imgview;
    };

#undef _NBL_DEFINE_SCMD_SPEC
} //namespace impl

class COpenGLCommandBuffer final : public IGPUCommandBuffer
{
protected:
    void freeSpaceInCmdPool();

    ~COpenGLCommandBuffer();

    template <impl::E_COMMAND_TYPE ECT>
    using SCmd = impl::SCmd<ECT>;
    system::logger_opt_smart_ptr m_logger;

    //NBL_FOREACH(NBL_SYSTEM_DECLARE_DYNLIB_FUNCPTR,__VA_ARGS__);
#define _NBL_SCMD_TYPE_FOR_ECT(ECT) SCmd<impl::ECT>,
    struct SCommand
    {
        impl::E_COMMAND_TYPE type;
        std::variant<
            NBL_FOREACH(_NBL_SCMD_TYPE_FOR_ECT, _NBL_COMMAND_TYPES_LIST)
            int
        > variant;

        template <impl::E_COMMAND_TYPE ECT>
        explicit SCommand(SCmd<ECT>&& cmd) : type(ECT), variant(std::move(cmd)) {}

        template <impl::E_COMMAND_TYPE ECT>
        SCmd<ECT>& get() { return std::get<SCmd<ECT>>(variant); }
        template <impl::E_COMMAND_TYPE ECT>
        const SCmd<ECT>& get() const { return std::get<SCmd<ECT>>(variant); }
    };
#undef _NBL_SCMD_TYPE_FOR_ECT
#undef _NBL_COMMAND_TYPES_LIST

    static void copyBufferToImage(const SCmd<impl::ECT_COPY_BUFFER_TO_IMAGE>& c, IOpenGL_FunctionTable* gl, SOpenGLContextLocalCache* ctxlocal, uint32_t ctxid);

    static void copyImageToBuffer(const SCmd<impl::ECT_COPY_IMAGE_TO_BUFFER>& c, IOpenGL_FunctionTable* gl, SOpenGLContextLocalCache* ctxlocal, uint32_t ctxid);

    static void beginRenderpass_clearAttachments(IOpenGL_FunctionTable* gl, const SRenderpassBeginInfo& info, GLuint fbo, const system::logger_opt_ptr logger);

    static void clearAttachments(IOpenGL_FunctionTable* gl, SOpenGLContextLocalCache* ctxlocal, uint32_t count, const asset::SClearAttachment* attachments);

    static bool pushConstants_validate(const IGPUPipelineLayout* _layout, uint32_t _stages, uint32_t _offset, uint32_t _size, const void* _values);

    static void blit(IOpenGL_FunctionTable* gl, GLuint src, GLuint dst, const asset::VkOffset3D srcOffsets[2], const asset::VkOffset3D dstOffsets[2], asset::ISampler::E_TEXTURE_FILTER filter);

    static inline GLbitfield barriersToMemBarrierBits(
        uint32_t memoryBarrierCount, const asset::SMemoryBarrier* pMemoryBarriers,
        uint32_t bufferMemoryBarrierCount, const SBufferMemoryBarrier* pBufferMemoryBarriers,
        uint32_t imageMemoryBarrierCount, const SImageMemoryBarrier* pImageMemoryBarriers
    ) {
        constexpr GLbitfield bufferBits = GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT | GL_ELEMENT_ARRAY_BARRIER_BIT | GL_UNIFORM_BARRIER_BIT | GL_COMMAND_BARRIER_BIT | GL_PIXEL_BUFFER_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT;
        constexpr GLbitfield imageBits = GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_UPDATE_BARRIER_BIT;

        // ignoring source access flags
        std::underlying_type_t<asset::E_ACCESS_FLAGS> bufaccess = 0u;
        for (uint32_t i = 0u; i < bufferMemoryBarrierCount; ++i)
            bufaccess |= pBufferMemoryBarriers[i].barrier.dstAccessMask;// | pBufferMemoryBarriers[i].barrier.srcAccessMask;
        std::underlying_type_t<asset::E_ACCESS_FLAGS> imgaccess = 0u;
        for (uint32_t i = 0u; i < imageMemoryBarrierCount; ++i)
            imgaccess |= pImageMemoryBarriers[i].barrier.dstAccessMask;// | pImageMemoryBarriers[i].barrier.srcAccessMask;
        std::underlying_type_t<asset::E_ACCESS_FLAGS> memaccess = 0u;
        for (uint32_t i = 0u; i < memoryBarrierCount; ++i)
            memaccess |= pMemoryBarriers[i].dstAccessMask;// | pMemoryBarriers[i].srcAccessMask;

        GLbitfield bufbarrier = bufferBits & accessFlagsToMemoryBarrierBits(static_cast<asset::E_ACCESS_FLAGS>(bufaccess));
        GLbitfield imgbarrier = imageBits & accessFlagsToMemoryBarrierBits(static_cast<asset::E_ACCESS_FLAGS>(imgaccess));
        GLbitfield membarrier = accessFlagsToMemoryBarrierBits(static_cast<asset::E_ACCESS_FLAGS>(memaccess));

        return bufbarrier | imgbarrier | membarrier;
    }

    static inline GLenum getGLprimitiveType(asset::E_PRIMITIVE_TOPOLOGY pt)
    {
        using namespace asset;
        switch (pt)
        {
        case EPT_POINT_LIST:
            return GL_POINTS;
        case EPT_LINE_LIST:
            return GL_LINES;
        case EPT_LINE_STRIP:
            return GL_LINE_STRIP;
        case EPT_TRIANGLE_LIST:
            return GL_TRIANGLES;
        case EPT_TRIANGLE_STRIP:
            return GL_TRIANGLE_STRIP;
        case EPT_TRIANGLE_FAN:
            return GL_TRIANGLE_FAN;
        case EPT_LINE_LIST_WITH_ADJACENCY:
            return GL_LINES_ADJACENCY;
        case EPT_LINE_STRIP_WITH_ADJACENCY:
            return GL_LINE_STRIP_ADJACENCY;
        case EPT_TRIANGLE_LIST_WITH_ADJACENCY:
            return GL_TRIANGLES_ADJACENCY;
        case EPT_TRIANGLE_STRIP_WITH_ADJACENCY:
            return GL_TRIANGLE_STRIP_ADJACENCY;
        case EPT_PATCH_LIST:
            return GL_PATCHES;
        default:
            return GL_INVALID_ENUM;
        }
    }

    COpenGLCommandPool* getGLCommandPool() const { return static_cast<COpenGLCommandPool*>(m_cmdpool.get()); }

    template <impl::E_COMMAND_TYPE ECT>
    void pushCommand(SCmd<ECT>&& cmd)
    {
        m_commands.emplace_back(std::move(cmd));
    }
    core::vector<SCommand> m_commands; // TODO: embed in the command pool via the use of linked list

public:
    void executeAll(IOpenGL_FunctionTable* gl, SOpenGLContextLocalCache* ctxlocal, uint32_t ctxid) const;


    COpenGLCommandBuffer(core::smart_refctd_ptr<const ILogicalDevice>&& dev, E_LEVEL lvl, IGPUCommandPool* _cmdpool, system::logger_opt_smart_ptr&& logger) : IGPUCommandBuffer(std::move(dev), lvl, _cmdpool), m_logger(std::move(logger)) {}

    inline void begin(uint32_t _flags) override final
    {
        IGPUCommandBuffer::begin(_flags);
        reset(_flags);
    }
    bool reset(uint32_t _flags) override final;


    bool bindIndexBuffer(const buffer_t* buffer, size_t offset, asset::E_INDEX_TYPE indexType) override
    {
        if (!this->isCompatibleDevicewise(buffer))
            return false;

        SCmd<impl::ECT_BIND_INDEX_BUFFER> cmd;
        cmd.buffer = core::smart_refctd_ptr<const buffer_t>(buffer);
        cmd.indexType = indexType;
        cmd.offset = offset;
        pushCommand<impl::ECT_BIND_INDEX_BUFFER>(std::move(cmd));
        return true;
    }
    bool draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) override
    {
        SCmd<impl::ECT_DRAW> cmd;
        cmd.vertexCount = vertexCount;
        cmd.instanceCount = instanceCount;
        cmd.firstVertex = firstVertex;
        cmd.firstInstance = firstInstance;
        pushCommand(std::move(cmd));
        return true;
    }
    bool drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) override
    {
        SCmd<impl::ECT_DRAW_INDEXED> cmd;
        cmd.indexCount = indexCount;
        cmd.instanceCount = instanceCount;
        cmd.firstIndex = firstIndex;
        cmd.vertexOffset = vertexOffset;
        cmd.firstInstance = firstInstance;
        pushCommand(std::move(cmd));
        return true;
    }
    bool drawIndirect(const buffer_t* buffer, size_t offset, uint32_t drawCount, uint32_t stride) override
    {
        SCmd<impl::ECT_DRAW_INDIRECT> cmd;
        cmd.buffer = core::smart_refctd_ptr<const buffer_t>(buffer);
        cmd.offset = offset;
        cmd.drawCount = drawCount;
        cmd.stride = stride;
        pushCommand(std::move(cmd));
        return true;
    }
    bool drawIndexedIndirect(const buffer_t* buffer, size_t offset, uint32_t drawCount, uint32_t stride) override
    {
        SCmd<impl::ECT_DRAW_INDEXED_INDIRECT> cmd;
        cmd.buffer = core::smart_refctd_ptr<const buffer_t>(buffer);
        cmd.offset = offset;
        cmd.drawCount = drawCount;
        cmd.stride = stride;
        pushCommand(std::move(cmd));
        return true;
    }

    inline bool drawMeshBuffer(const nbl::video::IGPUMeshBuffer* meshBuffer) override
    {
        if (meshBuffer && !meshBuffer->getInstanceCount())
            return false;

        const auto* pipeline = meshBuffer->getPipeline();
        const auto bindingFlags = pipeline->getVertexInputParams().enabledBindingFlags;
        auto vertexBufferBindings = meshBuffer->getVertexBufferBindings();
        auto indexBufferBinding = meshBuffer->getIndexBufferBinding();
        const auto indexType = meshBuffer->getIndexType();

        const nbl::video::IGPUBuffer* gpuBufferBindings[nbl::asset::SVertexInputParams::MAX_ATTR_BUF_BINDING_COUNT];
        {
            for (size_t i = 0; i < nbl::asset::SVertexInputParams::MAX_ATTR_BUF_BINDING_COUNT; ++i)
                gpuBufferBindings[i] = vertexBufferBindings[i].buffer.get();
        }

        size_t bufferBindingsOffsets[nbl::asset::SVertexInputParams::MAX_ATTR_BUF_BINDING_COUNT];
        {
            for (size_t i = 0; i < nbl::asset::SVertexInputParams::MAX_ATTR_BUF_BINDING_COUNT; ++i)
                bufferBindingsOffsets[i] = vertexBufferBindings[i].offset;
        }

        bindVertexBuffers(0, nbl::asset::SVertexInputParams::MAX_ATTR_BUF_BINDING_COUNT, gpuBufferBindings, bufferBindingsOffsets);
        bindIndexBuffer(indexBufferBinding.buffer.get(), indexBufferBinding.offset, indexType);

        const bool isIndexed = indexType != nbl::asset::EIT_UNKNOWN;
        
        const size_t instanceCount = meshBuffer->getInstanceCount();
        const size_t firstInstance = meshBuffer->getBaseInstance();
        const size_t firstVertex = meshBuffer->getBaseVertex();

        if (isIndexed)
        {
            const size_t& indexCount = meshBuffer->getIndexCount();
            const size_t firstIndex = 0; // I don't think we have utility telling us this one
            const size_t& vertexOffset = firstVertex;

            return drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
        }
        else
        {
            const size_t& vertexCount = meshBuffer->getIndexCount();
       
            return draw(vertexCount, instanceCount, firstVertex, firstInstance);
        }
    }

    bool setViewport(uint32_t firstViewport, uint32_t viewportCount, const asset::SViewport* pViewports) override
    {
        if (viewportCount == 0u)
            return false;

        SCmd<impl::ECT_SET_VIEWPORT> cmd;
        cmd.firstViewport = firstViewport;
        cmd.viewportCount = viewportCount;
        auto* viewports = getGLCommandPool()->emplace_n<asset::SViewport>(cmd.viewportCount, pViewports[0]);
        if (!viewports)
            return false;
        for (uint32_t i = 0u; i < viewportCount; ++i)
            viewports[i] = pViewports[i];
        cmd.viewports = viewports;
        pushCommand(std::move(cmd));
        return true;
    }

    bool setLineWidth(float lineWidth) override
    {
        SCmd<impl::ECT_SET_LINE_WIDTH> cmd;
        cmd.lineWidth = lineWidth;
        pushCommand(std::move(cmd));
        return true;
    }
    bool setDepthBias(float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor) override
    {
        SCmd<impl::ECT_SET_DEPTH_BIAS> cmd;
        cmd.depthBiasConstantFactor;
        cmd.depthBiasClamp = depthBiasClamp;
        cmd.depthBiasSlopeFactor = depthBiasSlopeFactor;
        pushCommand(std::move(cmd));
        return true;
    }

    bool setBlendConstants(const float blendConstants[4]) override
    {
        SCmd<impl::ECT_SET_BLEND_CONSTANTS> cmd;
        memcpy(cmd.blendConstants, blendConstants, 4*sizeof(float));
        pushCommand(std::move(cmd));
        return true;
    }

    bool copyBuffer(const buffer_t* srcBuffer, buffer_t* dstBuffer, uint32_t regionCount, const asset::SBufferCopy* pRegions) override
    {
        if (!this->isCompatibleDevicewise(srcBuffer))
            return false;
        if (!this->isCompatibleDevicewise(dstBuffer))
            return false;
        if (regionCount == 0u)
            return false;
        SCmd<impl::ECT_COPY_BUFFER> cmd;
        cmd.srcBuffer = core::smart_refctd_ptr<const buffer_t>(srcBuffer);
        cmd.dstBuffer = core::smart_refctd_ptr<buffer_t>(dstBuffer);
        cmd.regionCount = regionCount;
        auto* regions = getGLCommandPool()->emplace_n<asset::SBufferCopy>(regionCount, pRegions[0]);
        if (!regions)
            return false;
        for (uint32_t i = 0u; i < regionCount; ++i)
            regions[i] = pRegions[i];
        cmd.regions = regions;
        pushCommand(std::move(cmd));
        return true;
    }
    bool copyImage(const image_t* srcImage, asset::E_IMAGE_LAYOUT srcImageLayout, image_t* dstImage, asset::E_IMAGE_LAYOUT dstImageLayout, uint32_t regionCount, const asset::IImage::SImageCopy* pRegions) override
    {
        if (!this->isCompatibleDevicewise(srcImage))
            return false;
        if (!this->isCompatibleDevicewise(dstImage))
            return false;
        SCmd<impl::ECT_COPY_IMAGE> cmd;
        cmd.srcImage = core::smart_refctd_ptr<const image_t>(srcImage);
        cmd.srcImageLayout = srcImageLayout;
        cmd.dstImage = core::smart_refctd_ptr<image_t>(dstImage);
        cmd.dstImageLayout = dstImageLayout;
        cmd.regionCount = regionCount;
        auto* regions = getGLCommandPool()->emplace_n<asset::IImage::SImageCopy>(regionCount, pRegions[0]);
        if (!regions)
            return false;
        for (uint32_t i = 0u; i < regionCount; ++i)
            regions[i] = pRegions[i];
        cmd.regions = regions;
        pushCommand(std::move(cmd));
        return true;
    }
    bool copyBufferToImage(const buffer_t* srcBuffer, image_t* dstImage, asset::E_IMAGE_LAYOUT dstImageLayout, uint32_t regionCount, const asset::IImage::SBufferCopy* pRegions) override
    {
        if (!this->isCompatibleDevicewise(srcBuffer))
            return false;
        if (!this->isCompatibleDevicewise(dstImage))
            return false;
        SCmd<impl::ECT_COPY_BUFFER_TO_IMAGE> cmd;
        cmd.srcBuffer = core::smart_refctd_ptr<const buffer_t>(srcBuffer);
        cmd.dstImage = core::smart_refctd_ptr<image_t>(dstImage);
        cmd.dstImageLayout = dstImageLayout;
        cmd.regionCount = regionCount;
        auto* regions = getGLCommandPool()->emplace_n<asset::IImage::SBufferCopy>(regionCount, pRegions[0]);
        if (!regions)
            return false;
        for (uint32_t i = 0u; i < regionCount; ++i)
            regions[i] = pRegions[i];
        cmd.regions = regions;
        pushCommand(std::move(cmd));
        return true;
    }
    bool copyImageToBuffer(const image_t* srcImage, asset::E_IMAGE_LAYOUT srcImageLayout, buffer_t* dstBuffer, uint32_t regionCount, const asset::IImage::SBufferCopy* pRegions) override
    {
        if (!this->isCompatibleDevicewise(srcImage))
            return false;
        if (!this->isCompatibleDevicewise(dstBuffer))
            return false;
        SCmd<impl::ECT_COPY_IMAGE_TO_BUFFER> cmd;
        cmd.srcImage = core::smart_refctd_ptr<const image_t>(srcImage);
        cmd.srcImageLayout = srcImageLayout;
        cmd.dstBuffer = core::smart_refctd_ptr<buffer_t>(dstBuffer);
        cmd.regionCount = regionCount;
        auto* regions = getGLCommandPool()->emplace_n<asset::IImage::SBufferCopy>(regionCount, pRegions[0]);
        if (!regions)
            return false;
        for (uint32_t i = 0u; i < regionCount; ++i)
            regions[i] = pRegions[i];
        cmd.regions = regions;
        pushCommand(std::move(cmd));
        return true;
    }
    bool blitImage(const image_t* srcImage, asset::E_IMAGE_LAYOUT srcImageLayout, image_t* dstImage, asset::E_IMAGE_LAYOUT dstImageLayout, uint32_t regionCount, const asset::SImageBlit* pRegions, asset::ISampler::E_TEXTURE_FILTER filter) override
    {
        if (!this->isCompatibleDevicewise(srcImage))
            return false;
        SCmd<impl::ECT_BLIT_IMAGE> cmd;
        cmd.srcImage = core::smart_refctd_ptr<const image_t>(srcImage);
        cmd.srcImageLayout = srcImageLayout;
        cmd.dstImage = core::smart_refctd_ptr<image_t>(dstImage);
        cmd.regionCount = regionCount;
        auto* regions = getGLCommandPool()->emplace_n<asset::SImageBlit>(regionCount, pRegions[0]);
        if (!regions)
            return false;
        for (uint32_t i = 0u; i < regionCount; ++i)
            regions[i] = pRegions[i];
        cmd.regions = regions;
        cmd.filter = filter;
        pushCommand(std::move(cmd));
        return true;
    }
    bool resolveImage(const image_t* srcImage, asset::E_IMAGE_LAYOUT srcImageLayout, image_t* dstImage, asset::E_IMAGE_LAYOUT dstImageLayout, uint32_t regionCount, const asset::SImageResolve* pRegions) override
    {
        if (!this->isCompatibleDevicewise(srcImage))
            return false;
        SCmd<impl::ECT_RESOLVE_IMAGE> cmd;
        cmd.srcImage = core::smart_refctd_ptr<const image_t>(srcImage);
        cmd.srcImageLayout = srcImageLayout;
        cmd.dstImage = core::smart_refctd_ptr<image_t>(dstImage);
        cmd.dstImageLayout = dstImageLayout;
        cmd.regionCount = regionCount;
        auto* regions = getGLCommandPool()->emplace_n<asset::SImageResolve>(regionCount, pRegions[0]);
        if (!regions)
            return false;
        for (uint32_t i = 0u; i < regionCount; ++i)
            regions[i] = pRegions[i];
        cmd.regions = regions;
        pushCommand(std::move(cmd));
        return true;
    }

    bool bindVertexBuffers(uint32_t firstBinding, uint32_t bindingCount, const buffer_t*const *const pBuffers, const size_t* pOffsets) override
    {
        for (uint32_t i = 0u; i < bindingCount; ++i)
            if(pBuffers[i])
                if (!this->isCompatibleDevicewise(pBuffers[i]))
                    return false;
        SCmd<impl::ECT_BIND_VERTEX_BUFFERS> cmd;
        cmd.first = firstBinding;
        cmd.count = bindingCount;
        for (uint32_t i = 0u; i < cmd.count; ++i)
        {
            const buffer_t* b = pBuffers[i];
            cmd.buffers[i] = core::smart_refctd_ptr<const buffer_t>(b);
            cmd.offsets[i] = pOffsets[i];
        }
        pushCommand(std::move(cmd));
        return true;
    }

    bool setScissor(uint32_t firstScissor, uint32_t scissorCount, const VkRect2D* pScissors) override
    {
        SCmd<impl::ECT_SET_SCISSORS> cmd;
        cmd.firstScissor = firstScissor;
        cmd.scissorCount = scissorCount;
        auto* scissors = getGLCommandPool()->emplace_n<VkRect2D>(scissorCount, pScissors[0]);
        if (!scissors)
            return false;
        for (uint32_t i = 0u; i < scissorCount; ++i)
            scissors[i] = pScissors[i];
        cmd.scissors = scissors;
        pushCommand(std::move(cmd));
        return true;
    }
    bool setDepthBounds(float minDepthBounds, float maxDepthBounds) override
    {
        SCmd<impl::ECT_SET_DEPTH_BOUNDS> cmd;
        cmd.minDepthBounds = minDepthBounds;
        cmd.maxDepthBounds = maxDepthBounds;
        pushCommand(std::move(cmd));
        return true;
    }
    bool setStencilCompareMask(asset::E_STENCIL_FACE_FLAGS faceMask, uint32_t compareMask) override
    {
        SCmd<impl::ECT_SET_STENCIL_COMPARE_MASK> cmd;
        cmd.faceMask = faceMask;
        cmd.cmpMask = compareMask;
        pushCommand(std::move(cmd));
        return true;
    }
    bool setStencilWriteMask(asset::E_STENCIL_FACE_FLAGS faceMask, uint32_t writeMask) override
    {
        SCmd<impl::ECT_SET_STENCIL_WRITE_MASK> cmd;
        cmd.faceMask = faceMask;
        cmd.writeMask = writeMask;
        pushCommand(std::move(cmd));
        return true;
    }
    bool setStencilReference(asset::E_STENCIL_FACE_FLAGS faceMask, uint32_t reference) override
    {
        SCmd<impl::ECT_SET_STENCIL_REFERENCE> cmd;
        cmd.faceMask = faceMask;
        cmd.reference = reference;
        pushCommand(std::move(cmd));
        return true;
    }

    bool dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override
    {
        SCmd<impl::ECT_DISPATCH> cmd;
        cmd.groupCountX = groupCountX;
        cmd.groupCountY = groupCountY;
        cmd.groupCountZ = groupCountZ;
        pushCommand(std::move(cmd));
        return true;
    }
    bool dispatchIndirect(const buffer_t* buffer, size_t offset) override
    {
        if (!this->isCompatibleDevicewise(buffer))
            return false;
        SCmd<impl::ECT_DISPATCH_INDIRECT> cmd;
        cmd.buffer = core::smart_refctd_ptr<const buffer_t>(buffer);
        cmd.offset = offset;
        pushCommand(std::move(cmd));
        return true;
    }
    bool dispatchBase(uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override
    {
        SCmd<impl::ECT_DISPATCH_BASE> cmd;
        cmd.baseGroupX = baseGroupX;
        cmd.baseGroupY = baseGroupY;
        cmd.baseGroupZ = baseGroupZ;
        cmd.groupCountX = groupCountX;
        cmd.groupCountY = groupCountY;
        cmd.groupCountZ = groupCountZ;
        pushCommand(std::move(cmd));
        return true;
    }

    bool setEvent(event_t* event, const SDependencyInfo& depInfo) override
    {
        if (!this->isCompatibleDevicewise(event))
            return false;
        SCmd<impl::ECT_SET_EVENT> cmd;
        cmd.event = core::smart_refctd_ptr<event_t>(event);
        cmd.barrierBits = barriersToMemBarrierBits(depInfo.memBarrierCount, depInfo.memBarriers, depInfo.bufBarrierCount, depInfo.bufBarriers, depInfo.imgBarrierCount, depInfo.imgBarriers);
        pushCommand(std::move(cmd));
        return true;
    }
    bool resetEvent(event_t* event, asset::E_PIPELINE_STAGE_FLAGS stageMask) override
    {
        if (!this->isCompatibleDevicewise(event))
            return false;
        SCmd<impl::ECT_RESET_EVENT> cmd;
        cmd.event = core::smart_refctd_ptr<event_t>(event);
        cmd.stageMask = stageMask;
        pushCommand(std::move(cmd));
        return true;
    }

    bool waitEvents(uint32_t eventCount, event_t*const *const pEvents, const SDependencyInfo* depInfos) override
    {
        if (eventCount == 0u)
            return false;
        for (uint32_t i = 0u; i < eventCount; ++i)
            if (!this->isCompatibleDevicewise(pEvents[i]))
                return false;
        SCmd<impl::ECT_WAIT_EVENTS> cmd;
        cmd.barrier = 0;
        for (uint32_t i = 0u; i < eventCount; ++i)
        {
            auto& dep = depInfos[i];
            cmd.barrier |= barriersToMemBarrierBits(dep.memBarrierCount, dep.memBarriers, dep.bufBarrierCount, dep.bufBarriers, dep.imgBarrierCount, dep.imgBarriers);
        }
        pushCommand(std::move(cmd));
        return true;
    }

    bool pipelineBarrier(std::underlying_type_t<asset::E_PIPELINE_STAGE_FLAGS> srcStageMask, std::underlying_type_t<asset::E_PIPELINE_STAGE_FLAGS> dstStageMask,
        std::underlying_type_t<asset::E_DEPENDENCY_FLAGS> dependencyFlags,
        uint32_t memoryBarrierCount, const asset::SMemoryBarrier* pMemoryBarriers,
        uint32_t bufferMemoryBarrierCount, const SBufferMemoryBarrier* pBufferMemoryBarriers,
        uint32_t imageMemoryBarrierCount, const SImageMemoryBarrier* pImageMemoryBarriers) override
    {
        GLbitfield barrier = pipelineStageFlagsToMemoryBarrierBits(static_cast<asset::E_PIPELINE_STAGE_FLAGS>(srcStageMask), static_cast<asset::E_PIPELINE_STAGE_FLAGS>(dstStageMask));

        SCmd<impl::ECT_PIPELINE_BARRIER> cmd;
        cmd.barrier = barrier & barriersToMemBarrierBits(memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
        pushCommand(std::move(cmd));
        return true;
    }

    bool beginRenderPass(const SRenderpassBeginInfo* pRenderPassBegin, asset::E_SUBPASS_CONTENTS content) override
    {
        if (!this->isCompatibleDevicewise(pRenderPassBegin->framebuffer.get()))
            return false;
        SCmd<impl::ECT_BEGIN_RENDERPASS> cmd;
        cmd.renderpassBegin = pRenderPassBegin[0];
        if (cmd.renderpassBegin.clearValueCount > 0u)
        {
            auto* clearVals = getGLCommandPool()->emplace_n<asset::SClearValue>(cmd.renderpassBegin.clearValueCount, cmd.renderpassBegin.clearValues[0]);
            memcpy(clearVals, pRenderPassBegin->clearValues, cmd.renderpassBegin.clearValueCount*sizeof(asset::SClearValue));
            cmd.renderpassBegin.clearValues = clearVals;
        }
        cmd.content = content;
        pushCommand(std::move(cmd));
        return true;
    }
    bool nextSubpass(asset::E_SUBPASS_CONTENTS contents) override
    {
        SCmd<impl::ECT_NEXT_SUBPASS> cmd;
        cmd.contents = contents;
        pushCommand(std::move(cmd));
        return true;
    }
    bool endRenderPass() override
    {
        SCmd<impl::ECT_END_RENDERPASS> cmd;
        pushCommand(std::move(cmd));
        return true;
    }

    bool setDeviceMask(uint32_t deviceMask) override
    { 
        // theres no need to add this command to buffer in GL backend
        assert(false); //make calling this an error
        return IGPUCommandBuffer::setDeviceMask(deviceMask);
    }

    bool bindGraphicsPipeline(const graphics_pipeline_t* pipeline) override
    {
        if (!this->isCompatibleDevicewise(pipeline))
            return false;
        SCmd<impl::ECT_BIND_GRAPHICS_PIPELINE> cmd;
        cmd.pipeline = core::smart_refctd_ptr<const graphics_pipeline_t>(pipeline);
        pushCommand(std::move(cmd));
        return true;
    }
    bool bindComputePipeline(const compute_pipeline_t* pipeline) override
    {
        if (!this->isCompatibleDevicewise(pipeline))
            return false;
        SCmd<impl::ECT_BIND_COMPUTE_PIPELINE> cmd;
        cmd.pipeline = core::smart_refctd_ptr<const compute_pipeline_t>(pipeline);
        pushCommand(std::move(cmd));
        return true;
    }
        
    bool resetQueryPool(IQueryPool* queryPool, uint32_t firstQuery, uint32_t queryCount) override
    {
        // If multiple queries are issued using the same query object id before calling glGetQueryObject or glGetQueryBufferObject:
        // the results of the most recent query will be returned. In this case, when issuing a new query, the results of the previous query are discarded.
        // So just ignore :)
        return true;
    }
    bool beginQuery(IQueryPool* queryPool, uint32_t query, IQueryPool::E_QUERY_CONTROL_FLAGS flags = static_cast<IQueryPool::E_QUERY_CONTROL_FLAGS>(0)) override
    {
        if (!this->isCompatibleDevicewise(queryPool))
            return false;
        SCmd<impl::ECT_BEGIN_QUERY> cmd;
        cmd.queryPool = core::smart_refctd_ptr<const IQueryPool>(queryPool);
        cmd.query = query;
        cmd.flags = flags;
        pushCommand(std::move(cmd));
        return true;
    }
    bool endQuery(IQueryPool* queryPool, uint32_t query) override
    {
        if (!this->isCompatibleDevicewise(queryPool))
            return false;
        SCmd<impl::ECT_END_QUERY> cmd;
        cmd.queryPool = core::smart_refctd_ptr<const IQueryPool>(queryPool);
        cmd.query = query;
        pushCommand(std::move(cmd));
        return true;
    }
    bool copyQueryPoolResults(IQueryPool* queryPool, uint32_t firstQuery, uint32_t queryCount, buffer_t* dstBuffer, size_t dstOffset, size_t stride, IQueryPool::E_QUERY_RESULTS_FLAGS flags) override
    {
        if (!this->isCompatibleDevicewise(queryPool))
            return false;
        if (!this->isCompatibleDevicewise(dstBuffer))
            return false;
        SCmd<impl::ECT_COPY_QUERY_POOL_RESULTS> cmd;
        cmd.queryPool = core::smart_refctd_ptr<const IQueryPool>(queryPool);
        cmd.firstQuery = firstQuery;
        cmd.queryCount = queryCount;
        cmd.dstBuffer = core::smart_refctd_ptr<const IGPUBuffer>(dstBuffer);
        cmd.dstOffset = dstOffset;
        cmd.stride = stride;
        cmd.flags = flags;
        pushCommand(std::move(cmd));
        return true;
    }
    bool writeTimestamp(asset::E_PIPELINE_STAGE_FLAGS pipelineStage, IQueryPool* queryPool, uint32_t query) override
    {
        if (!this->isCompatibleDevicewise(queryPool))
            return false;
        SCmd<impl::ECT_WRITE_TIMESTAMP> cmd;
        cmd.queryPool = core::smart_refctd_ptr<const IQueryPool>(queryPool);
        cmd.pipelineStage = pipelineStage;
        cmd.query = query;
        pushCommand(std::move(cmd));
        return true;
    }
    // TRANSFORM_FEEDBACK_STREAM
    bool beginQueryIndexed(IQueryPool* queryPool, uint32_t query, uint32_t index, IQueryPool::E_QUERY_CONTROL_FLAGS flags = static_cast<IQueryPool::E_QUERY_CONTROL_FLAGS>(0))
    {
        if (!this->isCompatibleDevicewise(queryPool))
            return false;
        SCmd<impl::ECT_BEGIN_QUERY_INDEXED> cmd;
        cmd.queryPool = core::smart_refctd_ptr<const IQueryPool>(queryPool);
        cmd.query = query;
        cmd.flags = flags;
        cmd.index = index;
        pushCommand(std::move(cmd));
        return true;
    }
    bool endQueryIndexed(IQueryPool* queryPool, uint32_t query, uint32_t index) override
    {
        if (!this->isCompatibleDevicewise(queryPool))
            return false;
        SCmd<impl::ECT_END_QUERY_INDEXED> cmd;
        cmd.queryPool = core::smart_refctd_ptr<const IQueryPool>(queryPool);
        cmd.query = query;
        cmd.index = index;
        pushCommand(std::move(cmd));
        return true;
    }

    bool writeAccelerationStructureProperties(const core::SRange<IGPUAccelerationStructure>& pAccelerationStructures, IQueryPool::E_QUERY_TYPE queryType, IQueryPool* queryPool, uint32_t firstQuery)
    {
        return false;
    }

    bool bindDescriptorSets(asset::E_PIPELINE_BIND_POINT pipelineBindPoint, const pipeline_layout_t* layout, uint32_t firstSet, uint32_t descriptorSetCount,
        const descriptor_set_t*const *const pDescriptorSets, core::smart_refctd_dynamic_array<uint32_t> dynamicOffsets
    ) override
    {
        if (!this->isCompatibleDevicewise(layout))
            return false;
        for (uint32_t i = 0u; i < descriptorSetCount; ++i)
            if (!this->isCompatibleDevicewise(pDescriptorSets[i]))
                return false;
        SCmd<impl::ECT_BIND_DESCRIPTOR_SETS> cmd;
        cmd.pipelineBindPoint = pipelineBindPoint;
        cmd.layout = core::smart_refctd_ptr<const pipeline_layout_t>(layout);
        cmd.firstSet = firstSet;
        cmd.dsCount = descriptorSetCount;
        for (uint32_t i = 0u; i < cmd.dsCount; ++i)
            cmd.descriptorSets[i] = core::smart_refctd_ptr<const IGPUDescriptorSet>(pDescriptorSets[i]);
        cmd.dynamicOffsets = std::move(dynamicOffsets);
        pushCommand(std::move(cmd));
        return true;
    }
    bool pushConstants(const pipeline_layout_t* layout, std::underlying_type_t<asset::ISpecializedShader::E_SHADER_STAGE> stageFlags, uint32_t offset, uint32_t size, const void* pValues) override
    {
        SCmd<impl::ECT_PUSH_CONSTANTS> cmd;
        cmd.layout = core::smart_refctd_ptr<const pipeline_layout_t>(layout);
        cmd.stageFlags = stageFlags;
        cmd.offset = offset;
        cmd.size = size;
        memcpy(cmd.values, pValues, size);
        pushCommand(std::move(cmd));
        return true;
    }

    bool clearColorImage(image_t* image, asset::E_IMAGE_LAYOUT imageLayout, const asset::SClearColorValue* pColor, uint32_t rangeCount, const asset::IImage::SSubresourceRange* pRanges) override
    {
        if (!this->isCompatibleDevicewise(image))
            return false;
        SCmd<impl::ECT_CLEAR_COLOR_IMAGE> cmd;
        cmd.image = core::smart_refctd_ptr<image_t>(image);
        cmd.imageLayout = imageLayout;
        cmd.color = pColor[0];
        cmd.rangeCount = rangeCount;
        auto* ranges = getGLCommandPool()->emplace_n<asset::IImage::SSubresourceRange>(rangeCount, pRanges[0]);
        if (!ranges)
            return false;
        for (uint32_t i = 0u; i < rangeCount; ++i)
            ranges[i] = pRanges[i];
        cmd.ranges = ranges;
        pushCommand(std::move(cmd));
        return true;
    }
    bool clearDepthStencilImage(image_t* image, asset::E_IMAGE_LAYOUT imageLayout, const asset::SClearDepthStencilValue* pDepthStencil, uint32_t rangeCount, const asset::IImage::SSubresourceRange* pRanges) override
    {
        if (!this->isCompatibleDevicewise(image))
            return false;
        SCmd<impl::ECT_CLEAR_DEPTH_STENCIL_IMAGE> cmd;
        cmd.image = core::smart_refctd_ptr<image_t>(image);
        cmd.imageLayout = imageLayout;
        cmd.depthStencil = pDepthStencil[0];
        cmd.rangeCount = rangeCount;
        auto* ranges = getGLCommandPool()->emplace_n<asset::IImage::SSubresourceRange>(rangeCount, pRanges[0]);
        if (!ranges)
            return false;
        for (uint32_t i = 0u; i < rangeCount; ++i)
            ranges[i] = pRanges[i];
        cmd.ranges = ranges;
        pushCommand(std::move(cmd));
        return true;
    }
    bool clearAttachments(uint32_t attachmentCount, const asset::SClearAttachment* pAttachments, uint32_t rectCount, const asset::SClearRect* pRects) override
    {
        if (attachmentCount==0u || rectCount==0u)
            return false;
        SCmd<impl::ECT_CLEAR_ATTACHMENTS> cmd;
        cmd.attachmentCount = attachmentCount;
        auto* attachments = getGLCommandPool()->emplace_n<asset::SClearAttachment>(attachmentCount, pAttachments[0]);
        if (!attachments)
            return false;
        for (uint32_t i = 0u; i < attachmentCount; ++i)
            attachments[i] = pAttachments[i];
        cmd.attachments = attachments;
        cmd.rectCount = rectCount;
        auto* rects = getGLCommandPool()->emplace_n<asset::SClearRect>(rectCount, pRects[0]);
        if (!rects)
            return false;
        for (uint32_t i = 0u; i < rectCount; ++i)
            rects[i] = pRects[i];
        cmd.rects = rects;
        pushCommand(std::move(cmd));
        return true;
    }
    bool fillBuffer(buffer_t* dstBuffer, size_t dstOffset, size_t size, uint32_t data) override
    {
        if (!this->isCompatibleDevicewise(dstBuffer))
            return false;
        SCmd<impl::ECT_FILL_BUFFER> cmd;
        cmd.dstBuffer = core::smart_refctd_ptr<buffer_t>(dstBuffer);
        cmd.dstOffset = dstOffset;
        cmd.size = size;
        cmd.data = data;
        pushCommand(std::move(cmd));
        return true;
    }
    bool updateBuffer(buffer_t* dstBuffer, size_t dstOffset, size_t dataSize, const void* pData) override
    {
        if (!this->isCompatibleDevicewise(dstBuffer))
            return false;
        if ((dstOffset & 0x03ull) != 0ull)
            return false;
        if ((dataSize & 0x03ull) != 0ull)
            return false;
        if (dataSize > 65536ull)
            return false;
        if (!dstBuffer->canUpdateSubRange())
            return false;

        SCmd<impl::ECT_UPDATE_BUFFER> cmd;
        uint8_t* data = getGLCommandPool()->emplace_n<uint8_t>(dataSize);
        if (!data)
            return false;
        memcpy(data, pData, dataSize);
        cmd.dstBuffer = core::smart_refctd_ptr<buffer_t>(dstBuffer);
        cmd.dstOffset = dstOffset;
        cmd.dataSize = dataSize;
        cmd.data = data;
        pushCommand(std::move(cmd));
        return true;
    }
    bool executeCommands(uint32_t count, IGPUCommandBuffer*const *const cmdbufs) override
    {
        if (!IGPUCommandBuffer::executeCommands(count, cmdbufs))
            return false;
        for (uint32_t i = 0u; i < count; ++i)
            if (this->isCompatibleDevicewise(cmdbufs[i]))
                return false;

        for (uint32_t i = 0u; i < count; ++i)
        {
            SCmd<impl::ECT_EXECUTE_COMMANDS> cmd;
            cmd.cmdbuf = core::smart_refctd_ptr<IGPUCommandBuffer>(cmdbufs[i]);

            pushCommand(std::move(cmd));
        }
        return true;
    }
    bool regenerateMipmaps(image_view_t* imgview) override
    {
        SCmd<impl::ECT_REGENERATE_MIPMAPS> cmd;
        cmd.imgview = core::smart_refctd_ptr<image_view_t>(imgview);

        pushCommand(std::move(cmd));

        return true;
    }
};

}

#endif
