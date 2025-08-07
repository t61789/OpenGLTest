import os
import tomllib
from shader_compiler import ShaderCompiler
from pathlib import Path

with open(Path(os.path.abspath(__file__)).parent / "config.toml", "rb") as f:
    c = tomllib.load(f)

sc = ShaderCompiler(c)
sc.start()

