#include "opengl_test.h"

#include <fstream>
#include <iostream>

#include "utils.h"
#include "shared_object.h"
#include "material.h"
#include "game_framework.h"
#include "string_handle.h"
#include "math/matrix4x4.h"
#include "math/vec.h"
#include "math/quaternion.h"
#include "math/simd_math.h"

#include "spirv_cross/spirv_glsl.hpp"
#include "spirv_cross/spirv_msl.hpp"

using namespace op;

static void ReleaseStaticRes()
{
    Material::ReleaseStaticRes();
}

std::vector<uint32_t> read_spirv_file(const char* filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open SPIR-V file");
    }
    
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    if (size % sizeof(uint32_t) != 0) {
        throw std::runtime_error("Invalid SPIR-V file size");
    }
    
    std::vector<uint32_t> spirv(size / sizeof(uint32_t));
    file.read(reinterpret_cast<char*>(spirv.data()), size);
    return spirv;
}

int main(int argc, char* argv[])
{
    // auto spirvBinary = read_spirv_file("shaders/vert.spv");
    //
    // spirv_cross::CompilerGLSL glsl(std::move(spirvBinary));
    // spirv_cross::ShaderResources resources = glsl.get_shader_resources();
    //
    // for (auto& resource : resources.uniform_buffers)
    // {
    //     auto type = glsl.get_type(resource.base_type_id);
    //
    //     for (unsigned i = 0; i < type.member_types.size(); i++)
    //     {
    //         unsigned set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
    //         unsigned binding = glsl.get_decoration(resource.id, spv::DecorationBinding);
    //         unsigned location = glsl.get_decoration(resource.id, spv::DecorationLocation);
    //         auto& memberType = glsl.get_type(type.member_types[i]);
    //         auto& memberName = glsl.get_member_name(resource.base_type_id, i);
    //         auto offset = glsl.type_struct_member_offset(type, i);
    //         printf("Image %s at set = %u, binding = %u, location = %u, offset = %u\n", memberName.c_str(), set, binding, location, offset);
    //     }
    //
    //
    //     // // Modify the decoration to prepare it for GLSL.
    //     // glsl.unset_decoration(resource.id, spv::DecorationDescriptorSet);
    //     //
    //     // // Some arbitrary remapping if we want.
    //     // glsl.set_decoration(resource.id, spv::DecorationBinding, set * 16 + binding);
    // }
    //
    // // Set some options.
    // spirv_cross::CompilerGLSL::Options options;
    // options.version = 310;
    // options.es = true;
    // glsl.set_common_options(options);
    //
    // // Compile to GLSL, ready to give to GL driver.
    // std::string source = glsl.compile();
    //
    // Utils::Log(Info, "%s", source.c_str());
    //
    // return 0;
    
    auto gf = new GameFramework();
    try
    {
        if(!gf->Init())
        {
            return -1;
        }
        
        gf->GameLoop();
    
        delete gf;
    
        ReleaseStaticRes();
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << "\n";
    }
    
    if (!SharedObject::m_count.empty())
    {
        Utils::LogWarning("SharedObject未完全释放");
    }
    
    return 0;
}
