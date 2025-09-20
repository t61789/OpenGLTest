#include "gl_state.h"

#include "gl_buffer.h"
#include "gl_render_target.h"
#include "gl_shader.h"
#include "gl_texture.h"
#include "gl_vertex_array.h"

namespace op
{
    bool GlState::UnBindVertexArray()
    {
        if (!m_glVertexArray)
        {
            return false;
        }
        m_glVertexArray.reset();
        
        GetGlBufferInfo(GL_ARRAY_BUFFER)->buffer.reset();
        GetGlBufferInfo(GL_ELEMENT_ARRAY_BUFFER)->buffer.reset();

        glBindVertexArray(GL_NONE);

        GlCheckError();

        return true;
    }

    bool GlState::UnBindBuffer(const uint32_t type)
    {
        auto glBufferInfo = GetGlBufferInfo(type);
        if (!glBufferInfo->buffer)
        {
            return false;
        }

        glBindBuffer(type, GL_NONE);
        glBufferInfo->buffer.reset();

        GlCheckError();

        return true;
    }

    bool GlState::UnBindRenderTarget(crsp<GlRenderTarget> renderTarget)
    {
        assert(m_glRenderTarget == renderTarget);
        
        m_glRenderTarget.reset();
        
        GlBindFrameBuffer(GL_NONE);

        GlCheckError();

        return true;
    }

    bool GlState::BindVertexArray(const std::shared_ptr<GlVertexArray>& vao)
    {
        if (m_glVertexArray == vao)
        {
            return false;
        }
        m_glVertexArray = vao;

        GetGlBufferInfo(GL_ARRAY_BUFFER)->buffer.reset();
        GetGlBufferInfo(GL_ELEMENT_ARRAY_BUFFER)->buffer.reset();

        glBindVertexArray(vao->GetId());

        GlCheckError();

        return true;
    }

    bool GlState::BindBuffer(const std::shared_ptr<GlBuffer>& buffer)
    {
        auto glBufferInfo = GetGlBufferInfo(buffer->GetType());
        if (glBufferInfo->buffer == buffer)
        {
            return false;
        }
        
        if (buffer->GetType() == GL_ARRAY_BUFFER || buffer->GetType() == GL_ELEMENT_ARRAY_BUFFER)
        {
            auto& vao = GetVertexArray();
            if (vao && !vao->IsSettingAttr())
            {
                UnBindVertexArray();
            }
        }
        
        glBufferInfo->buffer = buffer;

        glBindBuffer(buffer->GetType(), buffer->GetId());

        GlCheckError();
        
        return true;
    }

    bool GlState::BindBufferBase(const std::shared_ptr<GlBuffer>& buffer, const uint32_t slot)
    {
        auto glBufferInfo = GetGlBufferInfo(buffer->GetType());
        auto& glBufferBaseInfo = glBufferInfo->baseBuffers[slot];
        if (glBufferBaseInfo.buffer == buffer)
        {
            return false;
        }
        glBufferBaseInfo.buffer = buffer;
        glBufferInfo->buffer.reset();

        glBindBufferBase(buffer->GetType(), slot, buffer->GetId());

        GlCheckError();

        return true;
    }

    bool GlState::BindShader(crsp<GlShader> shader)
    {
        if (m_glShader == shader)
        {
            return false;
        }
        m_glShader = shader;

        glUseProgram(m_glShader->GetId());

        GlCheckError();

        return true;
    }
    
    bool GlState::BindTexture(const uint32_t slot, crsp<GlTexture> texture)
    {
        if (!texture)
        {
            return false;
        }
        
        auto& textureInfo = m_glTextures[static_cast<uint8_t>(texture->GetType())];
        auto& oldTexture = textureInfo.slots[slot];
        if (oldTexture == texture)
        {
            return false;
        }
        oldTexture = texture;
        
        GlActiveTexture(slot);
        GlBindTexture(texture->GetType(), texture->GetId());

        return true;
    }

    bool GlState::BindRenderTarget(crsp<GlRenderTarget> frameBuffer)
    {
        if (m_glRenderTarget == frameBuffer)
        {
            return false;
        }
        m_glRenderTarget = frameBuffer;

        GlBindFrameBuffer(m_glRenderTarget->GetId());

        GlCheckError();

        return true;
    }

    void GlState::Reset()
    {
        assert(m_usingResource.expired());
        
        m_glShader.reset();

        m_glRenderTarget.reset();

        m_glVertexArray.reset();
        for (auto& glBufferInfo : m_glBufferInfos)
        {
            glBufferInfo.buffer.reset();
            for (auto& glBufferBaseInfo : glBufferInfo.baseBuffers)
            {
                glBufferBaseInfo.buffer.reset();
            }
        }

        for (auto& glTextureInfo : m_glTextures)
        {
            for (auto& texture : glTextureInfo.slots)
            {
                texture.reset();
            }
        }

        m_glCullMode = CullMode::UNSET;
        m_glBlendMode = BlendMode::UNSET;
    }

    void GlState::Check()
    {
        // TODO 未检查shader和texture
        
        for (auto& glBufferInfo : m_glBufferInfos)
        {
            auto glGlobalBuffer = GetGlGlobalBuffer(glBufferInfo.type);
            assert(!glBufferInfo.buffer && glGlobalBuffer == 0 || glBufferInfo.buffer->GetId() == glGlobalBuffer);

            for (auto& glBufferBaseInfo : glBufferInfo.baseBuffers)
            {
                auto glBufferBase = GetGlBufferBase(glBufferInfo.type, glBufferBaseInfo.slot);
                assert(!glBufferBaseInfo.buffer && glBufferBase == 0 || glBufferBaseInfo.buffer->GetId() == glBufferBase);
            }
        }
    }

    crsp<GlShader> GlState::GetShader()
    {
        return m_glShader;
    }

    const std::shared_ptr<GlVertexArray>& GlState::GetVertexArray()
    {
        return m_glVertexArray;
    }

    const std::shared_ptr<GlBuffer>& GlState::GetGlobalBuffer(const uint32_t type)
    {
        return GetGlBufferInfo(type)->buffer;
    }

    const std::shared_ptr<GlBuffer>& GlState::GetGlobalBufferBase(const uint32_t type, const uint32_t slot)
    {
        return GetGlBufferInfo(type)->baseBuffers[slot].buffer;
    }

    crvec<int32_t> GlState::BindTextures(crvecsp<GlTexture> textures)
    {
        static vec<int32_t> resultSlots;
        static arr<arr<bool, MAX_SUPPORT_SLOTS> , TEXTURE_TYPE_LIMIT> slotHasBound;
        resultSlots.resize(textures.size());
        for (auto& slots : slotHasBound)
        {
            slots.fill(false);
        }

        // 把已经绑定的texture先记录一下
        for (size_t i = 0; i < textures.size(); ++i)
        {
            auto& texture = textures[i];
            auto textureTypeIndex = static_cast<uint8_t>(texture->GetType());
            auto& destSlots = m_glTextures[textureTypeIndex].slots;
            auto boundSlot = find_index(destSlots, texture);
            resultSlots[i] = boundSlot;

            if (boundSlot != -1)
            { 
                slotHasBound[textureTypeIndex][boundSlot] = true;
            }
        }

        for (size_t i = 0; i < textures.size(); ++i)
        {
            auto& resultSlot = resultSlots[i];
            
            if (resultSlot == -1) // 绑定还没绑定的texture
            {
                auto& texture = textures[i];
                auto textureTypeIndex = static_cast<uint8_t>(texture->GetType());
                for (uint32_t curSlot = 0; curSlot < MAX_SUPPORT_SLOTS; ++curSlot)
                {
                    auto& curSlotHasBound = slotHasBound[textureTypeIndex][curSlot];
                    if (curSlotHasBound) // 找个还没绑定的slot
                    {
                        continue;
                    }
                    curSlotHasBound = true;

                    resultSlot = static_cast<int32_t>(curSlot);
                    auto success = BindTexture(resultSlot, texture);
                    assert(success);
                    break;
                }
            }

            assert(resultSlot != -1);
        }

        return resultSlots;
    }

    void GlState::SetCullMode(const CullMode mode)
    {
        if (m_glCullMode == mode)
        {
            return;
        }

        if (mode == CullMode::NONE)
        {
            GlDisable(GL_CULL_FACE);
        }
        else if (mode != CullMode::UNSET)
        {
            if (m_glCullMode == CullMode::NONE || m_glCullMode == CullMode::UNSET)
            {
                GlEnable(GL_CULL_FACE);
            }

            if (mode == CullMode::FRONT)
            {
                GlCullFace(GL_FRONT);
            }
            else if (mode == CullMode::BACK)
            {
                GlCullFace(GL_BACK);
            }
            else if (mode == CullMode::ALL)
            {
                GlCullFace(GL_FRONT_AND_BACK);
            }
        }

        m_glCullMode = mode;
    }

    void GlState::SetBlendMode(const BlendMode mode)
    {
        if (m_glBlendMode == mode)
        {
            return;
        }

        if (mode == BlendMode::NONE)
        {
            GlDisable(GL_BLEND);
        }
        else if (mode != BlendMode::UNSET)
        {
            if (m_glBlendMode == BlendMode::NONE || m_glBlendMode == BlendMode::UNSET)
            {
                GlEnable(GL_BLEND);
            }

            if (mode == BlendMode::BLEND)
            {
                GlBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            }
            else if (mode == BlendMode::ADD)
            {
                GlBlendFunc(GL_SRC_ALPHA, GL_ONE);
            }
        }

        m_glBlendMode = mode;
    }

    void GlState::SetDepthMode(const DepthMode mode)
    {
        if (m_glDepthMode == mode)
        {
            return;
        }

        if (mode == DepthMode::DISABLE)
        {
            GlDisable(GL_DEPTH_TEST);
        }
        else if (mode != DepthMode::UNSET)
        {
            if (m_glDepthMode == DepthMode::DISABLE || m_glDepthMode == DepthMode::UNSET)
            {
                GlEnable(GL_DEPTH_TEST);
            }
            
            if (mode == DepthMode::ALWAYS)
            {
                GlDepthFunc(GL_ALWAYS);
            }

            if (mode == DepthMode::LESS)
            {
                GlDepthFunc(GL_LESS);
            }

            if (mode == DepthMode::LESS_EQUAL)
            {
                GlDepthFunc(GL_LEQUAL);
            }

            if (mode == DepthMode::EQUAL)
            {
                GlDepthFunc(GL_EQUAL);
            }

            if (mode == DepthMode::NOT_EQUAL)
            {
                GlDepthFunc(GL_NOTEQUAL);
            }

            if (mode == DepthMode::GREATER)
            {
                GlDepthFunc(GL_GREATER);
            }

            if (mode == DepthMode::GREATER_EQUAL)
            {
                GlDepthFunc(GL_GEQUAL);
            }
        }

        m_glDepthMode = mode;
    }

    CullMode GlState::GetCullMode(cr<StringHandle> str)
    {
        static const umap<string_hash, CullMode> CULL_MODE_MAP =
        {
            {StringHandle("Unset"), CullMode::UNSET},
            {StringHandle("None"), CullMode::NONE},
            {StringHandle("Front"), CullMode::FRONT},
            {StringHandle("Back"), CullMode::BACK},
            {StringHandle("All"), CullMode::ALL},
        };

        return CULL_MODE_MAP.at(str);
    }

    BlendMode GlState::GetBlendMode(cr<StringHandle> str)
    {
        static const umap<string_hash, BlendMode> BLEND_MODE_MAP =
        {
            {StringHandle("Unset"), BlendMode::UNSET},
            {StringHandle("None"), BlendMode::NONE},
            {StringHandle("Blend"), BlendMode::BLEND},
            {StringHandle("Add"), BlendMode::ADD},
        };

        return BLEND_MODE_MAP.at(str);
    }

    DepthMode GlState::GetDepthMode(cr<StringHandle> str)
    {
        static const umap<string_hash, DepthMode> DEPTH_MODE_MAP =
        {
            {StringHandle("Unset"), DepthMode::UNSET},
            {StringHandle("Disable"), DepthMode::DISABLE},
            {StringHandle("Always"), DepthMode::ALWAYS},
            {StringHandle("Less"), DepthMode::LESS},
            {StringHandle("LessEqual"), DepthMode::LESS_EQUAL},
            {StringHandle("Equal"), DepthMode::EQUAL},
            {StringHandle("NotEqual"), DepthMode::NOT_EQUAL},
            {StringHandle("Greater"), DepthMode::GREATER},
            {StringHandle("GreaterEqual"), DepthMode::GREATER_EQUAL}
        };

        return DEPTH_MODE_MAP.at(str);
    }

    void GlState::UseGlResource(const std::shared_ptr<IGlResource>& resource)
    {
        assert(m_usingResource.expired());

        m_usingResource = resource;
    }

    void GlState::EndUseGlResource(const std::shared_ptr<IGlResource>& resource)
    {
        assert(!m_usingResource.expired() && m_usingResource.lock() == resource);

        m_usingResource.reset();
    }

    uint32_t GlState::GetGlGlobalBuffer(const uint32_t type)
    {
        int glBuffer = 0;
        glGetIntegerv(GetGlBufferBindingType(type), &glBuffer);

        GlCheckError();
        
        return glBuffer;
    }

    uint32_t GlState::GetGlBufferBase(const uint32_t type, const uint32_t slot)
    {
        int glBuffer = 0;
        glGetIntegeri_v(type, slot, &glBuffer);

        GlCheckError();
        
        return glBuffer;
    }

    GlState::GlBufferInfo* GlState::GetGlBufferInfo(const uint32_t type)
    {
        auto glBufferInfo = find(m_glBufferInfos, &GlBufferInfo::type, type);
        if (glBufferInfo)
        {
            return glBufferInfo;
        }
        
        m_glBufferInfos.push_back({});
        glBufferInfo = &m_glBufferInfos.back();
        glBufferInfo->type = type;
        glBufferInfo->buffer = nullptr;
        glBufferInfo->baseBuffers.resize(MAX_SUPPORT_SLOTS);
        
        for (uint32_t i = 0; i < MAX_SUPPORT_SLOTS; ++i)
        {
            glBufferInfo->baseBuffers[i] = {
                i,
                nullptr
            };
        }
        
        return glBufferInfo;
    }

    GlRenderTarget* GlState::GetGlRenderTarget()
    {
        return m_glRenderTarget.get();
    }

    void GlState::GlSetVertAttrEnable(const uint32_t index, const bool enable)
    {
        if (enable)
        {
            glEnableVertexAttribArray(index);
        }
        else
        {
            glDisableVertexAttribArray(index);
        }

        GlCheckError();
    }

    void GlState::GlSetVertAttrLayout(
        const uint32_t index,
        const uint32_t strideF,
        const uint32_t type,
        const uint32_t normalized,
        const uint32_t vertexDataStrideB,
        const uint32_t vertexDataOffsetB)
    {
        glVertexAttribPointer(
            index,
            static_cast<GLint>(strideF),
            type,
            static_cast<GLboolean>(normalized),
            static_cast<GLsizei>(vertexDataStrideB),
            reinterpret_cast<void*>(vertexDataOffsetB));

        GlCheckError();
    }

    void GlState::GlDeleteBuffer(const uint32_t id)
    {
        glDeleteBuffers(1, &id);

        GlCheckError();
    }

    void GlState::GlDeleteVertexArray(const uint32_t id)
    {
        glDeleteVertexArrays(1, &id);

        GlCheckError();
    }

    void GlState::GlDeleteTexture(const uint32_t id)
    {
        glDeleteTextures(1, &id);

        GlCheckError();
    }

    void GlState::GlDeleteFrameBuffer(const uint32_t id)
    {
        glDeleteFramebuffers(1, &id);

        GlCheckError();
    }

    void GlState::GlDeleteShader(const uint32_t id)
    {
        glDeleteShader(id);

        GlCheckError();
    }

    void GlState::GlDeleteProgram(const uint32_t id)
    {
        glDeleteProgram(id);

        GlCheckError();
    }

    uint32_t GlState::GlGenVertexArray()
    {
        uint32_t id = 0;
        glGenVertexArrays(1, &id);

        GlCheckError();
        
        return id;
    }

    uint32_t GlState::GlGenBuffer()
    {
        uint32_t id = 0;
        glGenBuffers(1, &id);

        GlCheckError();
        
        return id;
    }

    uint32_t GlState::GlGenTexture()
    {
        uint32_t id = 0;
        glGenTextures(1, &id);

        GlCheckError();
        
        return id;
    }

    uint32_t GlState::GlGenFrameBuffer()
    {
        uint32_t id = 0;
        glGenFramebuffers(1, &id);

        GlCheckError();
        
        return id;
    }

    uint32_t GlState::GlGenShader(const uint32_t type)
    {
        auto result = glCreateShader(type);

        GlCheckError();
        
        return result;
    }

    uint32_t GlState::GlGenProgram()
    {
        auto result = glCreateProgram();

        GlCheckError();

        return result;
    }

    void GlState::GlBufferData(const uint32_t target, const uint32_t sizeB, const void* data, const uint32_t usage)
    {
        glBufferData(target, sizeB, data, usage);

        GlCheckError();
    }

    void GlState::GlBufferSubData(const uint32_t target, const uint32_t offsetB, const uint32_t sizeB, const void* data)
    {
        glBufferSubData(target, offsetB, sizeB, data);

        GlCheckError();
    }

    void GlState::GlActiveTexture(const uint32_t slot)
    {
        glActiveTexture(GL_TEXTURE0 + slot);

        GlCheckError();
    }

    void GlState::GlBindTexture(const GlTextureType type, const uint32_t id)
    {
        glBindTexture(ToGl(type), id);
        
        GlCheckError();
    }

    void GlState::GlTexParameter(const GlTextureType type, const uint32_t param, const uint32_t value)
    {
        glTexParameteri(ToGl(type), param, static_cast<int>(value));

        GlCheckError();
    }

    void GlState::GlPixelStore(const uint32_t param, const uint32_t value)
    {
        glPixelStorei(param, static_cast<int>(value));

        GlCheckError();
    }

    void GlState::GlTexImage2D(
        const GlTextureType type,
        const uint32_t level,
        const uint32_t internalFormat,
        const uint32_t width,
        const uint32_t height,
        const uint32_t border,
        const uint32_t format,
        const uint32_t dataType,
        const void* data)
    {
        glTexImage2D(
            ToGl(type),
            static_cast<GLint>(level),
            static_cast<GLint>(internalFormat),
            static_cast<GLsizei>(width),
            static_cast<GLsizei>(height),
            static_cast<GLint>(border),
            static_cast<GLenum>(format),
            static_cast<GLenum>(dataType),
            data);

        GlCheckError();
    }

    void GlState::GlTexImageCube(
        const uint32_t cubeIndex,
        const uint32_t level,
        const uint32_t internalFormat,
        const uint32_t width,
        const uint32_t height,
        const uint32_t border,
        const uint32_t format,
        const uint32_t dataType,
        const void* data)
    {
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + cubeIndex,
            static_cast<int>(level),
            static_cast<int>(internalFormat),
            static_cast<int>(width),
            static_cast<int>(height),
            static_cast<int>(border),
            static_cast<int>(format),
            dataType,
            data);

        GlCheckError();
    }

    void GlState::GlGenerateMipmap(const GlTextureType type)
    {
        glGenerateMipmap(ToGl(type));

        GlCheckError();
    }

    void GlState::GlBindFrameBuffer(const uint32_t id)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, id);
        
        GlCheckError();
    }

    void GlState::GlDrawBuffer(const uint32_t attachmentIndex)
    {
        glDrawBuffer(attachmentIndex);

        GlCheckError();
    }

    void GlState::GlDrawBuffers(const uint32_t count, const uint32_t* attachments)
    {
        glDrawBuffers(static_cast<GLsizei>(count), attachments);

        GlCheckError();
    }

    void GlState::GlFrameBufferTexture2D(const uint32_t attachmentType, const uint32_t textureId)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, GL_TEXTURE_2D, textureId, 0);

        GlCheckError();
    }

    void GlState::GlClearBufferFv(cr<Vec4> color, const uint32_t colorAttachmentIndex)
    {
        glClearBufferfv(GL_COLOR, static_cast<GLint>(colorAttachmentIndex), &color.x);

        GlCheckError();
    }

    void GlState::GlClearDepth(const float depth)
    {
        glClearDepth(depth);

        GlCheckError();
    }

    void GlState::GlClear(const uint32_t clearBits)
    {
        glClear(clearBits);

        GlCheckError();
    }

    void GlState::GlViewport(const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height)
    {
        glViewport(static_cast<GLint>(x), static_cast<GLint>(y), static_cast<GLsizei>(width), static_cast<GLsizei>(height));
        
        GlCheckError();
    }

    void GlState::GlCheckFramebufferStatus()
    {
        auto frameBufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if(frameBufferStatus != GL_FRAMEBUFFER_COMPLETE)
        {
            THROW_ERRORF("FrameBufferAttachment绑定失败：%s", std::to_string(frameBufferStatus).c_str())
        }

        GlCheckError();
    }

    void GlState::GlEnable(const uint32_t flag)
    {
        glEnable(flag);

        GlCheckError();
    }

    void GlState::GlDisable(const uint32_t flag)
    {
        glDisable(flag);

        GlCheckError();
    }

    void GlState::GlCullFace(const uint32_t flag)
    {
        glCullFace(flag);

        GlCheckError();
    }

    void GlState::GlBlendFunc(const uint32_t sfactor, const uint32_t dfactor)
    {
        glBlendFunc(sfactor, dfactor);

        GlCheckError();
    }

    void GlState::GlDepthFunc(const uint32_t flag)
    {
        glDepthFunc(flag);

        GlCheckError();
    }

    void GlState::GlDrawElements(const uint32_t mode, const uint32_t count, const uint32_t type, const void* indices)
    {
        glDrawElements(mode, static_cast<GLsizei>(count), type, indices);

        GlCheckError();
    }

    void GlState::GlShaderSource(const uint32_t shaderId, const uint32_t count, const char** source, const int* length)
    {
        glShaderSource(shaderId, static_cast<GLsizei>(count), source, length);

        GlCheckError();
    }

    void GlState::GlCompileShader(const uint32_t shaderId)
    {
        glCompileShader(shaderId);

        GlCheckError();
    }
    
    void GlState::GlAttachShader(const uint32_t programId, const uint32_t shaderId)
    {
        glAttachShader(programId, shaderId);

        GlCheckError();
    }

    void GlState::GlLinkProgram(const uint32_t programId)
    {
        glLinkProgram(programId);

        GlCheckError();
    }

    void GlState::GlGetShaderiv(const uint32_t shaderId, const uint32_t param, int* value)
    {
        glGetShaderiv(shaderId, param, value);

        GlCheckError();
    }

    void GlState::GlGetShaderInfoLog(const uint32_t shaderId, const uint32_t bufSize, char* infoLog)
    {
        glGetShaderInfoLog(shaderId, static_cast<int>(bufSize), nullptr, infoLog);

        GlCheckError();
    }

    void GlState::GlGetProgramiv(const uint32_t programId, const uint32_t param, int* value)
    {
        glGetProgramiv(programId, param, value);

        GlCheckError();
    }

    void GlState::GlGetActiveUniform(const uint32_t programId, const uint32_t index, const uint32_t bufSize, int32_t* length, int* size, uint32_t* type, char* name)
    {
        glGetActiveUniform(programId, index, static_cast<GLsizei>(bufSize), length, size, type, name);

        GlCheckError();
    }

    void GlState::GlMultiDrawElementsIndirect(const uint32_t mode, const uint32_t type, const void* indirect, const uint32_t drawCount, const uint32_t stride)
    {
        glMultiDrawElementsIndirect(mode, type, indirect, static_cast<GLsizei>(drawCount), static_cast<GLsizei>(stride));

        GlCheckError();
    }

    void GlState::GlUniform1i(const uint32_t location, const int32_t value)
    {
        glUniform1i(static_cast<GLsizei>(location), value);

        GlCheckError();
    }

    void GlState::GlUniform1f(const uint32_t location, const float value)
    {
        glUniform1f(static_cast<GLsizei>(location), value);

        GlCheckError();
    }

    void GlState::GlUniform4f(const uint32_t location, const float x, const float y, const float z, const float w)
    {
        glUniform4f(static_cast<GLsizei>(location), x, y, z, w);

        GlCheckError();
    }

    void GlState::GlUniformMatrix4fv(const uint32_t location, const uint32_t count, const bool transpose, const float* value)
    {
        glUniformMatrix4fv(static_cast<GLsizei>(location), static_cast<GLsizei>(count), transpose, value);

        GlCheckError();
    }

    void GlState::GlUniform1fv(const uint32_t location, const uint32_t count, const float* value)
    {
        glUniform1fv(static_cast<GLsizei>(location), static_cast<GLsizei>(count), value);

        GlCheckError();
    }

    int32_t GlState::GetGlUniformLocation(const uint32_t programId, const cstr name)
    {
        auto result = glGetUniformLocation(programId, name);

        GlCheckError();

        return result;
    }

    void* GlState::GlMapBuffer(const uint32_t target, const uint32_t access)
    {
        auto result = glMapBuffer(target, access);

        GlCheckError();
        
        return result;
    }

    void GlState::GlUnmapBuffer(const uint32_t target)
    {
        glUnmapBuffer(target);

        GlCheckError();
    }
    
    uint32_t GlState::GetGlBufferBindingType(const uint32_t type)
    {
        static const std::map<uint32_t, uint32_t> GL_BUFFER_BINDING_TYPE = {
            { GL_ARRAY_BUFFER, GL_ARRAY_BUFFER_BINDING },
            { GL_ELEMENT_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER_BINDING },
            { GL_UNIFORM_BUFFER, GL_UNIFORM_BUFFER_BINDING },
            { GL_SHADER_STORAGE_BUFFER, GL_SHADER_STORAGE_BUFFER_BINDING },
        };

        return GL_BUFFER_BINDING_TYPE.at(type);
    }

    uint32_t GlState::GlGetUniformBlockIndex(const uint32_t programId, const cstr name)
    {
        auto result = glGetUniformBlockIndex(programId, name);

        GlCheckError();

        return result;
    }
    
    void GlState::GlGetActiveUniformBlockiv(const uint32_t programId, const uint32_t uniformBlockIndex, const uint32_t param, int32_t* results)
    {
        glGetActiveUniformBlockiv(programId, uniformBlockIndex, param, results);

        GlCheckError();
    }

    uint32_t GlState::GlGetUniformIndices(const uint32_t programId, const char* uniformNames)
    {
        GLuint index;

        glGetUniformIndices(programId, 1, &uniformNames, &index);

        GlCheckError();

        return index;
    }

    void GlState::GlGetActiveUniformsiv(const uint32_t programId, const int32_t uniformCount, const uint32_t* uniformIndices, const uint32_t param, int32_t* results)
    {
        glGetActiveUniformsiv(programId, uniformCount, uniformIndices, param, results);

        GlCheckError();
    }

    void GlState::GlGetActiveUniformName(uint32_t programId, uint32_t uniformIndex, uint32_t bufSize, int32_t* length, char* name)
    {
        glGetActiveUniformName(programId, uniformIndex, bufSize, length, name);

        GlCheckError();
    }

    void GlState::GlCheckError()
    {
        if (!ENABLE_GL_CHECK_ERROR)
        {
            return;
        }
        
        const auto& errors = read_gl_error();
        assert(errors.empty());
    }

    GLenum GlState::ToGl(const GlTextureType target)
    {
        static umap<GlTextureType, GLenum> mapper = {
            { GlTextureType::TEXTURE_2D, GL_TEXTURE_2D },
            { GlTextureType::TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP },
        };

        return mapper.at(target);
    }
}
