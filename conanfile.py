from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.tools.files import copy
import os


class GradusConan(ConanFile):
    name = "gradus"
    version = "1.0.0"
    
    # Метаданные
    license = "MIT"
    author = "Your Name <your.email@example.com>"
    url = "https://github.com/yourusername/gradus"
    description = "Temperature conversion utility with array processing and visualization"
    topics = ("temperature", "conversion", "cli", "utility")
    
    # Настройки
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "enable_tests": [True, False],
        "enable_sanitizers": [True, False]
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "enable_tests": True,
        "enable_sanitizers": False
    }
    
    # Экспортируемые файлы
    exports_sources = "CMakeLists.txt", "gradus.c", "*.h"
    
    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC
    
    def configure(self):
        if self.options.shared:
            self.options.rm_safe("fPIC")
    
    def layout(self):
        cmake_layout(self)
    
    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.variables["BUILD_TESTS"] = self.options.enable_tests
        tc.variables["ENABLE_ASAN"] = self.options.enable_sanitizers
        tc.variables["ENABLE_UBSAN"] = self.options.enable_sanitizers
        tc.generate()
    
    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
    
    def package(self):
        copy(self, "LICENSE", dst=os.path.join(self.package_folder, "licenses"), src=self.source_folder)
        cmake = CMake(self)
        cmake.install()
    
    def package_info(self):
        self.cpp_info.libs = ["gradus"]
        self.cpp_info.names["pkg_config"] = "gradus"
        
        # Информация для consumers
        self.cpp_info.set_property("cmake_find_mode", "both")
        self.cpp_info.set_property("cmake_target_name", "gradus::gradus")
        self.cpp_info.set_property("pkg_config_name", "gradus")
        
        # Инструменты
        bin_path = os.path.join(self.package_folder, "bin")
        self.output.info(f"Appending PATH environment variable: {bin_path}")
        self.env_info.PATH.append(bin_path)
