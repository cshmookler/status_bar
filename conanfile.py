"""status_bar root Conan file"""
from configparser import ConfigParser
from dataclasses import dataclass, field
import os
from typing import List, Dict

from conan import ConanFile
from conan.errors import ConanException
from conan.tools.files import copy as copy_file
from conan.tools.gnu import PkgConfigDeps
from conan.tools.meson import Meson, MesonToolchain
from conan.tools.scm import Git

required_conan_version = ">=2.0.6"


@dataclass
class Module:
    """Dependency module information"""

    name: str
    status: bool


@dataclass
class Dependency:
    """Dependency information"""

    name: str
    version: str
    status: bool
    modules: List[Module] = field(default_factory=list)


class status_bar(ConanFile):
    """status_bar"""

    # Required
    name = "status_bar"

    # Metadata
    license = "Zlib"
    author = "Caden Shmookler (cshmookler@gmail.com)"
    url = "https://github.com/cshmookler/status_bar.git"
    description = (
        "Status bar for dwm. Customizable at runtime and updates instantly."
    )
    topics = []

    # Configuration
    package_type = "application"
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}
    build_policy = "missing"

    # Essential files
    exports_sources = ".git/*", "include/*", "src/*", "meson.build", "LICENSE"

    # Other
    _dependencies: List[str] = ["gtest/1.14.0", "xorg/system"]

    def set_version(self):
        """Get project version from Git"""
        git = Git(self, folder=self.recipe_folder)
        try:
            self.version = git.run("describe --tags").partition("-")[0]
        except ConanException:
            self.version = "0.0.0"

    def config_options(self):
        """Change available options"""
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        """Change behavior based on set options"""
        if self.options.shared:
            self.options.rm_safe("fPIC")

    def build_requirements(self):
        """Declare dependencies of the build system"""
        self.tool_requires("meson/1.2.1")
        self.tool_requires("pkgconf/2.0.3")

    def requirements(self):
        """Declare library dependencies"""
        for dep in self._dependencies:
            self.requires(dep)

    def layout(self):
        """Set the layout of the build files"""
        self.folders.build = os.path.join(self.recipe_folder, "build")
        self.folders.generators = os.path.join(self.folders.build, "generators")
        self._dependency_config = os.path.join(
            self.recipe_folder, "dependencies.ini"
        )

    def _read_dep_config(self) -> List[Dependency]:
        """Reads dependency information from the dependency configuration file"""
        deps: List[Dependency] = []
        parser = ConfigParser()
        parser.read(self._dependency_config)
        section = parser["dependencies"]
        for dep in section:
            name = dep.rsplit("/", 1)[0]
            version = dep.rsplit("/", 1)[1]
            status = True if section[dep] == "yes" else False
            deps.append(Dependency(name, version, status))
        # for section in parser.sections():
        #     name = section.rsplit("/", 1)[0]
        #     version = section.rsplit("/", 1)[1]
        #     deps.append(Dependency(name, version))
        #     for module in parser[section]:
        #         status = parser[section][module]
        #         deps[-1].modules.append(
        #             Module(module, True if status == "yes" else False)
        #         )
        return deps

    def _write_dep_config(self, deps: List[Dependency]) -> None:
        """Write dependency information to the dependency configuration file"""
        parser = ConfigParser()
        parser.add_section("dependencies")
        for dep in deps:
            key = dep.name + "/" + dep.version
            value = "yes" if dep.status else "no"
            parser["dependencies"][key] = value
            # section = dep.name + "/" + dep.version
            # parser.add_section(section)
            # for module in dep.modules:
            #     parser[section][module.name] = "yes" if module.status else "no"
        with open(self._dependency_config, "w") as config_file:
            parser.write(config_file, space_around_delimiters=True)

    def generate(self):
        """Generate the build system"""
        deps = PkgConfigDeps(self)
        deps.generate()

        meson_dep_dict: Dict[str, Dependency] = {}
        for package_conf, content in deps.content.items():
            dep_name = content.split("\nName")[1].split("\n")[0].strip(": ")
            # dep_name = component_info[0]
            # dep_component = component_info[1]
            # try:
            #     meson_dep_dict[dep_name].modules.append(
            #         Module(dep_component, True)
            #     )
            # except KeyError:
            meson_dep_dict[dep_name] = Dependency(
                dep_name,
                content.split("\nVersion")[1].split("\n")[0].strip(": "),
                True,
            )
            # meson_dep_dict[dep_name].modules.append(
            #     Module(dep_component, True)
            # )

        if os.path.isfile(self._dependency_config):
            # Merge dependencies from the configuration file with dependencies from Conan
            user_deps: List[Dependency] = self._read_dep_config()
            for dep in user_deps:
                # TODO: check version before overwriting (also, store version in the dependency configuration file!)
                meson_dep_dict[dep.name] = dep

        meson_dep_list: List[Dependency] = list(meson_dep_dict.values())
        self._write_dep_config(meson_dep_list)

        meson_deps_classless = [
            [
                dep.name,
                [module.name for module in dep.modules if module.status],
                dep.version,
            ]
            for dep in meson_dep_list
            if dep.status
        ]

        toolchain = MesonToolchain(self)
        toolchain.properties = {
            "_name": self.name,
            "_version": self.version,
            "_type": str(self.package_type),
            "_deps": meson_deps_classless,
        }
        toolchain.generate()

    def build(self):
        """Build the test project"""
        self._build_folder = os.path.join(self.recipe_folder, "build")
        meson = Meson(self)
        meson.configure()
        copy_file(
            self,
            "version.hpp",
            self.build_folder,
            self.source_folder + "/src/",
        )
        meson.build()
        copy_file(
            self,
            "compile_commands.json",
            self.build_folder,
            self.source_folder,
        )
        meson.test()

    def package(self):
        """Install project headers and compiled libraries"""
        meson = Meson(self)
        meson.install()

    def package_info(self):
        """Package information"""
        self.cpp_info.libs = [self.name]
        self.cpp_info.includedirs = ["include"]
        self.cpp_info.bindirs = ["bin"]
