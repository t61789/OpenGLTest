#pragma once

#include "utils.h"
#include "math/matrix4x4.h"
#include "math/vec.h"

namespace op
{
    class GlShader : public std::enable_shared_from_this<GlShader>
    {
    public:
        struct UniformInfo
        {
            uint32_t location;
            uint32_t elemNum;
            uint32_t type;
            StringHandle name;
            bool hasInitGlVal = false;
        };
        
        GlShader(cr<std::string> preparedVert, cr<std::string> preparedFrag);
        ~GlShader();
        GlShader(const GlShader& other) = delete;
        GlShader(GlShader&& other) noexcept = delete;
        GlShader& operator=(const GlShader& other) = delete;
        GlShader& operator=(GlShader&& other) noexcept = delete;

        uint32_t GetId() const { return m_id;}
        UniformInfo* GetUniformInfo(string_hash name);

        void Use();

        void SetInt(string_hash name, int32_t val);
        void SetFloat(string_hash name, float val);
        void SetFloat4(string_hash name, const float* val);
        void SetMatrix(string_hash name, const float* val);
        void SetFloatArr(string_hash name, const float* val, uint32_t count);

    private:
        uint32_t m_id;
        umap<string_hash, UniformInfo> m_uniforms;
        
        static void CheckShaderCompilation(GLuint vertexShader, cr<str> source);
        static umap<string_hash, UniformInfo> LoadUniforms(GLuint program);
    };
}
