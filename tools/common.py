import os
import toml
from pathlib import Path

def load_config():
    config_path = Path(os.path.abspath(__file__)).parent / "config.toml"
    with open(config_path, "r") as f:
        return toml.load(f)