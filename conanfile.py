import os

from conan import ConanFile
from conan.tools.cmake import cmake_layout
from conan.tools.files import copy


class ImGuiExample(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"

    def requirements(self):
        self.requires("imgui/1.91.8")
        self.requires("glfw/3.4")
        self.requires("glew/2.2.0")
        self.requires("assimp/5.4.3")
        self.requires("glm/1.0.1")
        self.requires("nlohmann_json/3.12.0")
        self.requires("stb/cci.20230920")
        self.requires("tinyxml2/11.0.0")
        self.requires("tracy/0.12.1")
        self.requires("spirv-cross/1.4.313.0")
        self.requires("boost/1.88.0")
        self.requires("onetbb/2022.3.0")
        

    def generate(self):
        src_binding_path = os.path.join(self.dependencies["imgui"].package_folder, "res", "bindings")
        dst_binding_path = os.path.join(self.source_folder, "lib/imgui")
        
        os.makedirs(dst_binding_path, exist_ok=True)
        
        copy(self, "*glfw*", src_binding_path, dst_binding_path)
        copy(self, "*opengl3*", src_binding_path, dst_binding_path)
        

    def layout(self):
        cmake_layout(self)