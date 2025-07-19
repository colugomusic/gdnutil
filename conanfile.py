from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout
from conan.tools.build import check_min_cppstd
from conan.tools.files import copy
from os.path import join

class AdsConanFile(ConanFile):
	name = "gdnutil"
	version = "1.0.0"
	license = "MIT"
	author = "ColugoMusic"
	url = "https://github.com/colugomusic/gdnutil"
	description = "GDNative Utility Library"
	settings = "os", "compiler", "build_type", "arch"
	generators = "CMakeDeps", "CMakeToolchain"
	exports_sources = "CMakeLists.txt", "include/*"

	def layout(self):
		cmake_layout(self)

	def requirements(self):
		pass

	def validate(self):
		if self.settings.get_safe("compiler.cppstd"):
			check_min_cppstd(self, 20)

	def build(self):
		pass

	def package(self):
		copy(self, "*.hpp", src=join(self.source_folder, "include/gdnutil"), dst=join(self.package_folder, "include/gdnutil"))
	
	def package_info(self):
		self.cpp_info.includedirs = ["include"]