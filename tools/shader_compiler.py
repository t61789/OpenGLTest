import shutil
import sys
sys.path.append("E:/Programming/PythonLib")
import utils
import os
import base64
import json
import common
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
        
        self.new_console = False

        
    def start(self):
        shader_pack = {}

        utils.log_info(f"[green]开始编译着色器[/]")
        shaders = self.shader_dir.rglob(f"*{self.shader_suffix}")
        for shader_path in shaders:
            relative_shader_path = shader_path.relative_to(self.assets_dir)
            variants = self._load_variants(self._load_meta(shader_path))
            if variants is None:
                return False
            
            shader_data = []
            for variant in variants:
                utils.log_info(f"编译shader [cyan]{relative_shader_path}[/] {variant}")
                vert_spv_str = self.compile(self.vs_entry, "vert", shader_path, variant)
                frag_spv_str = self.compile(self.ps_entry, "frag", shader_path, variant)
                
                if not vert_spv_str or not frag_spv_str:
                    utils.log_error("编译失败，可以查看相关输出")
                    return False
                
                shader_data.append({
                    "variant": variant,
                    "vert": vert_spv_str,
                    "frag": frag_spv_str
                })

            shader_pack[str(relative_shader_path).replace("\\", "/")] = shader_data
            
        with open(self.shader_pack_path, "w") as f:
            json.dump(shader_pack, f, indent=4)
            
        return True

            
    def compile(self, entry, stage, shader_path, variant):
        temp_file_path = Path(os.path.abspath(__file__)).parent / "temp.file"
        with open(shader_path, "r", encoding="utf-8") as shader_file:
            with open(temp_file_path, "w", encoding="utf-8") as temp_file:
                for macro in variant:
                    temp_file.write(f"#define {macro}\n")
                temp_file.write(shader_file.read())
        
        cmd = [
            "glslang", "-e", entry, "-S", stage, "-D", "-V", 
            str(temp_file_path), f"-I{str(self.assets_dir)}", "-o", str(self.temp_spv_path)
        ]   
        
        def print_stdout(msg):
            if msg == str(temp_file_path):
                return
            utils.log_info(msg)
            
        def print_stderr(msg):
            if msg == str(temp_file_path):
                return
            utils.log_error(msg)

        succeed = utils.execute_cmd(
            cmd,
            str(self.assets_dir),
            on_stdout=print_stdout,
            on_stderr=print_stderr
        )
        
        if not succeed:
            return None
        
        with open(self.temp_spv_path, "rb") as f:
            binary_data = f.read()
            base64_str = base64.b64encode(binary_data).decode("utf-8")
        self.temp_spv_path.unlink()
            
        return base64_str
    
    
    @staticmethod
    def _load_variants(meta):
        marco_lines = meta.get("variants", [])
        marco_lines = [m for m in marco_lines if len(m) > 0]
        
        for i in range(len(marco_lines)):
            macro_line = marco_lines[i]
                
            if len(macro_line) == 1:
                if macro_line[0] == "_":
                    utils.log_error(f"存在仅有空值的宏行: {macro_line}")
                    return None
                macro_line.insert(0, "_")
            
            if len(macro_line) != len(set(macro_line)):
                utils.log_error(f"同一行中存在相同的宏: {macro_line}")
                return None
                
            for macro in macro_line:
                if macro == "_":
                    continue
                
                for l in marco_lines:
                    if l != macro_line and macro in l:
                        utils.log_error(f"宏定义重复: {macro}")
                        return None
                    
        variants = []
        cur_variant = []
        def load(index):
            if index >= len(marco_lines):
                variants.append(cur_variant[:])
                return
            
            for macro in marco_lines[index]:
                if macro != "_":
                    cur_variant.append(macro)
                load(index + 1)
                if macro != "_":
                    cur_variant.pop()
        
        load(0)
        if len(variants) == 0:
            variants.append([])
        return variants
            
    
    @staticmethod
    def _load_meta(shader_path):
        meta = {}
        meta_path = Path(str(shader_path) + ".meta")
        if not meta_path.exists():
            return meta
        with open(meta_path, "r") as f:
            meta = json.load(f)
        return meta
    
    
if __name__ == "__main__":
    config = common.load_config()
    result = ShaderCompiler(config).start()
    if not result:
        input()
        