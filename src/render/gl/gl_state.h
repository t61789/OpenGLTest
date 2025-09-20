#pragma once
#include <memory>

#include "utils.h"
#include "math/vec.h"

namespace op
{
    class IGlResource;
    class GlBuffer;
    class GlVertexArray;
    class GlTexture;
    class GlRenderTarget;
    class GlShader;
    class RenderingUtils;

    enum class GlTextureType : uint8_t
    {
        TEXTURE_2D,
        TEXTURE_CUBE_MAP,
        TEXTURE_TARGET_COUNT
    };
    #define TEXTURE_TYPE_LIMIT (static_cast<uint8_t>(GlTextureType::TEXTURE_TARGET_COUNT))

    enum class CullMode : uint8_t
    {
        UNSET,
        BACK,
        FRONT,
        NONE,
        ALL
    };

    enum class BlendMode : uint8_t
    {
        UNSET,
        NONE,
        BLEND,
        ADD
    };

    enum class DepthMode : uint8_t
    {
        UNSET,
        DISABLE,
        ALWAYS,
        LESS,
        LESS_EQUAL,
        EQUAL,
        NOT_EQUAL,
        GREATER,
        GREATER_EQUAL
    };

    class GlState : public Singleton<GlState>
    {
        friend class GlBuffer;
        friend class GlVertexArray;
        friend class GlTexture;
        friend class GlRenderTarget;
        friend class GlShader;
        friend class RenderingUtils;
        friend class CBufferLayout;
        friend class BatchRenderUnit;
        
    public:
        void Reset();
        void Check();

        crsp<GlShader> GetShader();
        crsp<GlVertexArray> GetVertexArray();
        crsp<GlBuffer> GetGlobalBuffer(uint32_t type);
        crsp<GlBuffer> GetGlobalBufferBase(uint32_t type, uint32_t slot);
        crvec<int32_t> BindTextures(crvecsp<GlTexture> textures);
        bool BindShader(crsp<GlShader> shader);
        void SetCullMode(CullMode mode, bool hasOddNegativeScale = false);
        void SetBlendMode(BlendMode mode);
        void SetDepthMode(DepthMode mode);

        static CullMode GetCullMode(cr<StringHandle> str);
        static BlendMode GetBlendMode(cr<StringHandle> str);
        static DepthMode GetDepthMode(cr<StringHandle> str);

    private:
        struct GlTextureInfo
        {
            arr<sp<GlTexture>, TEXTURE_SLOT_LIMIT> slots = {};
        };
        
        struct GlBufferBaseInfo
        {
            uint32_t slot = 0;
            sp<GlBuffer> buffer = nullptr;
        };
        
        struct GlBufferInfo
        {
            uint32_t type = 0;
            sp<GlBuffer> buffer = nullptr;
            vec<GlBufferBaseInfo> baseBuffers;
        };

        CullMode m_glCullMode = CullMode::UNSET;
        BlendMode m_glBlendMode = BlendMode::UNSET;
        DepthMode m_glDepthMode = DepthMode::UNSET;
        wp<IGlResource> m_usingResource;
        vec<GlBufferInfo> m_glBufferInfos;
        sp<GlVertexArray> m_glVertexArray = nullptr;
        sp<GlRenderTarget> m_glRenderTarget = nullptr;
        sp<GlShader> m_glShader = nullptr;
        arr<GlTextureInfo, TEXTURE_TYPE_LIMIT> m_glTextures = {};
        
        bool UnBindBuffer(uint32_t type);
        bool UnBindVertexArray();
        bool UnBindRenderTarget(crsp<GlRenderTarget> renderTarget);

        bool BindVertexArray(crsp<GlVertexArray> vao);
        bool BindBuffer(crsp<GlBuffer> buffer);
        bool BindBufferBase(crsp<GlBuffer> buffer, uint32_t slot);
        bool BindTexture(uint32_t slot, crsp<GlTexture> texture);
        bool BindRenderTarget(crsp<GlRenderTarget> frameBuffer);
        
        void UseGlResource(crsp<IGlResource> resource);
        void EndUseGlResource(crsp<IGlResource> resource);

        GlBufferInfo* GetGlBufferInfo(uint32_t type);
        GlRenderTarget* GetGlRenderTarget();
        
        static uint32_t GetGlGlobalBuffer(uint32_t type);
        static uint32_t GetGlBufferBase(uint32_t type, uint32_t slot);
        static uint32_t GetGlBufferBindingType(uint32_t type);
        static uint32_t GlGetUniformBlockIndex(uint32_t programId, cstr name);
        static uint32_t GlGetUniformIndices(uint32_t programId, const char* uniformNames);
        static int32_t GetGlUniformLocation(uint32_t programId, cstr name);
        static void GlGetActiveUniformBlockiv(uint32_t programId, uint32_t uniformBlockIndex, uint32_t param, int32_t* results);
        static void GlGetActiveUniformsiv(uint32_t programId, int32_t uniformCount, const uint32_t* uniformIndices, uint32_t param, int32_t* results);
        static void GlGetActiveUniformName(uint32_t programId, uint32_t uniformIndex, uint32_t bufSize, int32_t* length, char* name);
        
        static uint32_t GlGenVertexArray();
        static uint32_t GlGenBuffer();
        static uint32_t GlGenTexture();
        static uint32_t GlGenFrameBuffer();
        static uint32_t GlGenShader(uint32_t type);
        static uint32_t GlGenProgram();
        static void* GlMapBuffer(uint32_t target, uint32_t access);
        static void GlUnmapBuffer(uint32_t target);
        static void GlSetVertAttrEnable(uint32_t index, bool enable);
        static void GlSetVertAttrLayout(uint32_t index, uint32_t strideF, uint32_t type, uint32_t normalized, uint32_t vertexDataStrideB, uint32_t vertexDataOffsetB);
        static void GlDeleteBuffer(uint32_t id);
        static void GlDeleteVertexArray(uint32_t id);
        static void GlDeleteTexture(uint32_t id);
        static void GlDeleteFrameBuffer(uint32_t id);
        static void GlDeleteShader(uint32_t id);
        static void GlDeleteProgram(uint32_t id);
        static void GlBufferData(uint32_t target, uint32_t sizeB, const void* data, uint32_t usage);
        static void GlBufferSubData(uint32_t target, uint32_t offsetB, uint32_t sizeB, const void* data);
        static void GlActiveTexture(uint32_t slot);
        static void GlBindTexture(GlTextureType type, uint32_t id);
        static void GlTexParameter(GlTextureType type, uint32_t param, uint32_t value);
        static void GlPixelStore(uint32_t param, uint32_t value);
        static void GlTexImage2D(GlTextureType type, uint32_t level, uint32_t internalFormat, uint32_t width, uint32_t height, uint32_t border, uint32_t format, uint32_t dataType, const void* data);
        static void GlTexImageCube(uint32_t cubeIndex, uint32_t level, uint32_t internalFormat, uint32_t width, uint32_t height, uint32_t border, uint32_t format, uint32_t dataType, const void* data);
        static void GlGenerateMipmap(GlTextureType type);
        static void GlBindFrameBuffer(uint32_t id);
        static void GlDrawBuffer(uint32_t attachmentIndex);
        static void GlDrawBuffers(uint32_t count, const uint32_t* attachments);
        static void GlFrameBufferTexture2D(uint32_t attachmentType, uint32_t textureId);
        static void GlClearBufferFv(cr<Vec4> color, uint32_t colorAttachmentIndex);
        static void GlClearDepth(float depth);
        static void GlClear(uint32_t clearBits);
        static void GlViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
        static void GlCheckFramebufferStatus();
        static void GlEnable(uint32_t flag);
        static void GlDisable(uint32_t flag);
        static void GlCullFace(uint32_t flag);
        static void GlBlendFunc(uint32_t sfactor, uint32_t dfactor);
        static void GlDepthFunc(uint32_t flag);
        static void GlDrawElements(uint32_t mode, uint32_t count, uint32_t type, const void* indices);
        static void GlShaderSource(uint32_t shaderId, uint32_t count, const char** source, const int* length);
        static void GlCompileShader(uint32_t shaderId);
        static void GlAttachShader(uint32_t programId, uint32_t shaderId);
        static void GlLinkProgram(uint32_t programId);
        static void GlGetShaderiv(uint32_t shaderId, uint32_t param, int* value);
        static void GlGetShaderInfoLog(uint32_t shaderId, uint32_t bufSize, char* infoLog);
        static void GlGetProgramiv(uint32_t programId, uint32_t param, int* value);
        static void GlGetActiveUniform(uint32_t programId, uint32_t index, uint32_t bufSize, int32_t* length, int* size, uint32_t* type, char* name);
        static void GlMultiDrawElementsIndirect(uint32_t mode, uint32_t type, const void* indirect, uint32_t drawCount, uint32_t stride);

        static void GlUniform1i(uint32_t location, int32_t value);
        static void GlUniform1f(uint32_t location, float value);
        static void GlUniform4f(uint32_t location, float x, float y, float z, float w);
        static void GlUniformMatrix4fv(uint32_t location, uint32_t count, bool transpose, const float* value);
        static void GlUniform1fv(uint32_t location, uint32_t count, const float* value);
        
        
        static void GlCheckError();

        static GLenum ToGl(GlTextureType target);
    };
}
