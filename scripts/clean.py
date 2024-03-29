"""Remove build files"""

import os
from shutil import rmtree


def remove(file: str) -> None:
    """Remove a file and ignore errors"""
    try:
        os.remove(file)
    except FileNotFoundError:
        pass


def clean() -> None:
    """Remove build files"""
    rmtree("build", ignore_errors=True)
    remove("compile_commands.json")
    remove(os.path.join("src", "version.hpp"))


if __name__ == "__main__":
    clean()
