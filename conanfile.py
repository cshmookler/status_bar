"""status_bar root Conan file"""

from importlib import import_module
import os
from typing import List, Dict

from conan import ConanFile
from conan.errors import ConanException
from conan.tools.files import copy as copy_file
from conan.tools.gnu import PkgConfigDeps
from conan.tools.gnu.pkgconfigdeps import _PCGenerator, _PCContentGenerator
from conan.tools.meson import Meson, MesonToolchain
from conan.tools.scm import Git


this_dir: str = os.path.dirname(__file__)

placeholder_version: str = "0.0.0"

required_conan_version = ">=2.3.0"


class status_bar(ConanFile):

    # Required
    name = "status_bar"

    # Metadata
    license = "Zlib"
    author = "Caden Shmookler (cshmookler@gmail.com)"
    url = "https://github.com/cshmookler/status_bar.git"
    description = "Status bar for [dwm](https://dwm.suckless.org). Customizable at runtime and updates instantly."
    topics = []

    # Configuration
    package_type = "application"
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "quit_after_generate": [True, False],
    }
    default_options = {
        "quit_after_generate": False,
    }
    build_policy = "missing"

    # Files needed by Conan to resolve version and dependencies
    exports = (
        os.path.join(".git", "*"),
        "update_deps.py",
        "binary_config.json",
    )

    # Files needed by Conan to build from source
    exports_sources = (
        os.path.join("build_scripts", "*"),
        os.path.join("src", "*"),
        "meson.build",
    )

    # External includes
    _binary_config_module = import_module("update_deps")

    def set_version(self):
        """Get project version from Git"""
        git = Git(self, folder=self.recipe_folder)
        try:
            self.version = git.run("describe --tags").partition("-")[0]
        except ConanException:
            # Set a placeholder version if an error is encountered while using Git
            self.version = placeholder_version

    def build_requirements(self):
        """Declare dependencies of the build system"""
        self.tool_requires("meson/1.4.0")
        self.tool_requires("pkgconf/2.2.0")

    def requirements(self):
        """Resolve and declare dependencies"""
        self._binaries = self._binary_config_module.Binaries()
        self._binaries.read()

        for binary in self._binaries:
            for dep in binary.dependencies:
                # Ignore deactivated dependencies
                if not dep.enabled:
                    continue

                # Declare dependencies
                if not dep.link_preference:
                    self.requires(dep.name + "/" + dep.version)
                else:
                    self.requires(
                        dep.name + "/" + dep.version,
                        options={"shared": dep.dynamic},
                    )

    def layout(self):
        """Set the layout of the build files"""
        self.folders.build = os.path.join(self.recipe_folder, "build")
        self.folders.generators = os.path.join(self.folders.build, "generators")

    def generate(self):
        """Generate the build system"""
        # Generate .pc files for dependencies
        deps = PkgConfigDeps(self)
        deps.generate()

        # Resolve dependency components
        resolved_dependencies = {}
        for require, dep in self.dependencies.host.items():
            # Name and version of the dependency
            dependency_name_and_version: str = (
                dep.ref.name + "/" + str(dep.ref.version)
            )

            # Create a dictionary for storing components of this dependency
            if dependency_name_and_version not in resolved_dependencies:
                resolved_dependencies[dependency_name_and_version] = {}

            # This class is used internally by Conan to get dependency component information and generate PkgConfig files
            pc_generator = _PCGenerator(deps, require, dep)

            # Add the name of the package as the sole component if it does not have any components
            if not pc_generator._dep.cpp_info.has_components:
                sole_component_pc_info = (
                    pc_generator._content_generator._get_context(
                        pc_generator._package_info()
                    )
                )
                sole_component_name = str(sole_component_pc_info["name"])
                sole_component_version = str(sole_component_pc_info["version"])
                resolved_dependencies[dependency_name_and_version][
                    sole_component_name
                ] = self._binary_config_module.Component(
                    name=sole_component_name,
                    version=sole_component_version,
                    enabled=True,
                )

            # Accumulate all components of the dependency
            for component_info in pc_generator._components_info():
                component_pc_info = (
                    pc_generator._content_generator._get_context(component_info)
                )
                component_name = str(component_pc_info["name"])
                component_version = str(component_pc_info["version"])
                resolved_dependencies[dependency_name_and_version][
                    component_name
                ] = self._binary_config_module.Component(
                    name=component_name,
                    version=component_version,
                    enabled=True,
                )

        # Explicitly declared binaries and their respective dependencies
        declared_binaries: dict = self._binaries.json()

        # Merge resolved dependency components with dependency components explicitly declared within the binary configuration
        for binary, binary_info in declared_binaries.items():
            for dep_name_and_version, dep_info in binary_info[
                "dependencies"
            ].items():
                if dep_name_and_version not in resolved_dependencies:
                    continue

                # Remove components from the list of declared components that do not exist within the resolved components list
                for component_name, component_info in dep_info[
                    "components"
                ].items():
                    if (
                        component_name
                        not in resolved_dependencies[dep_name_and_version]
                    ):
                        dep_info["components"].pop(component_name)

                # Add missing components to the list of declared components
                # Add a temporary version to components that don't have one
                # Temporary versions are excluded from the JSON output
                for component_name, component_info in resolved_dependencies[
                    dep_name_and_version
                ].items():
                    if component_name not in dep_info["components"]:
                        # Add the missing component with a temporary version
                        dep_info["components"][component_name] = {
                            "version": component_info.version,
                            "enabled": component_info.enabled,
                            "exclude_version_from_json": True,
                        }
                        continue

                    declared_component_info = dep_info["components"][
                        component_name
                    ]
                    if type(declared_component_info) == bool:
                        # Add a temporary version to an existing component
                        dep_info["components"][component_name] = {
                            "version": component_info.version,
                            "enabled": declared_component_info,
                            "exclude_version_from_json": True,
                        }

        self._binaries.structured(
            raw_json=declared_binaries,
            mark_temporary_versions=True,
        )
        self._binaries.write()

        toolchain = MesonToolchain(self)
        toolchain.properties = {
            "_name": self.name,
            "_version": self.version,
            "_binaries": self._binaries.unstructured(),
        }
        toolchain.generate()

        if self.options.quit_after_generate:
            exit(0)

    def build(self):
        """Build this project"""
        self._build_folder = os.path.join(self.recipe_folder, "build")
        meson = Meson(self)
        meson.configure()
        meson.build()
        copy_file(
            self,
            "compile_commands.json",
            self.build_folder,
            self.source_folder,
        )
        meson.test()
