import sys
sys.path.append("E:/Programming/PythonLib")
import utils
import os
import base64
import tomllib
import json
from pathlib import Path

class ShaderCompiler:
    
    def __init__(self, config):
        self.project_dir = Path(config["project_path"])
        self.assets_dir = self.project_dir / "assets"
        self.shader_dir = self.assets_dir / "shaders"
        
        self.temp_spv_path = Path(os.path.abspath(__file__)).parent / "temp.spv"
        self.shader_pack_path = self.assets_dir / "shaders.pack"
        
        self.shader_suffix = config["shader_compiler"]["shader_suffix"]
        self.vs_entry = config["shader_compiler"]["vs_entry"]
        self.ps_entry = config["shader_compiler"]["ps_entry"]

        
    def start(self):
        shader_pack = {}
        
        shaders = self.shader_dir.rglob(f"*{self.shader_suffix}")
        for shader_path in shaders:
            utils.log_info(f"编译vert：{shader_path}")
            vert_spv_str = self.compile(self.vs_entry, "vert", shader_path)
            utils.log_info(f"编译frag：{shader_path}")
            frag_spv_str = self.compile(self.ps_entry, "frag", shader_path)
            
            if not vert_spv_str or not frag_spv_str:
                utils.log_error("编译失败，可以查看相关输出")
                return
            
            relative_shader_path = shader_path.relative_to(self.assets_dir)
            shader_pack[str(relative_shader_path).replace("\\", "/")] = {
                "vert" : vert_spv_str,
                "frag" : frag_spv_str
            }
            
        with open(self.shader_pack_path, "w") as f:
            json.dump(shader_pack, f, indent=4)

            
    def compile(self, entry, stage, shader_path):
        cmd = [
            "glslang", "-e", entry, "-S", stage, "-D", "-V", 
            str(shader_path), f"-I{str(self.assets_dir)}", "-o", str(self.temp_spv_path)
        ]   
        
        def print_stdout(msg):
            if msg == str(shader_path):
                return
            print(msg)
        
        succeed = utils.execute_cmd(
            cmd,
            str(self.assets_dir),
            on_stdout=print_stdout,
            on_stderr=print
        )
        
        if not succeed:
            return None
        
        with open(self.temp_spv_path, "rb") as f:
            binary_data = f.read()
            base64_str = base64.b64encode(binary_data).decode("utf-8")
        self.temp_spv_path.unlink()
            
        return base64_str
        
        