cmake_minimum_required(VERSION 3.30)
project(gdnutil)
add_library(gdnutil INTERFACE)
add_library(gdnutil::gdnutil ALIAS gdnutil)
target_sources(gdnutil INTERFACE
	FILE_SET HEADERS
	BASE_DIRS
		${CMAKE_CURRENT_LIST_DIR}/include
	FILES
		${CMAKE_CURRENT_LIST_DIR}/include/gdnutil/action_builder.hpp
		${CMAKE_CURRENT_LIST_DIR}/include/gdnutil/call.hpp
		${CMAKE_CURRENT_LIST_DIR}/include/gdnutil/class_wrapper.hpp
		${CMAKE_CURRENT_LIST_DIR}/include/gdnutil/control_helpers.hpp
		${CMAKE_CURRENT_LIST_DIR}/include/gdnutil/dictionary_helpers.hpp
		${CMAKE_CURRENT_LIST_DIR}/include/gdnutil/dirt.hpp
		${CMAKE_CURRENT_LIST_DIR}/include/gdnutil/enums.hpp
		${CMAKE_CURRENT_LIST_DIR}/include/gdnutil/hacks.hpp
		${CMAKE_CURRENT_LIST_DIR}/include/gdnutil/history.hpp
		${CMAKE_CURRENT_LIST_DIR}/include/gdnutil/hover_status.hpp
		${CMAKE_CURRENT_LIST_DIR}/include/gdnutil/input_handler.hpp
		${CMAKE_CURRENT_LIST_DIR}/include/gdnutil/input_helpers.hpp
		${CMAKE_CURRENT_LIST_DIR}/include/gdnutil/macros.hpp
		${CMAKE_CURRENT_LIST_DIR}/include/gdnutil/memory.hpp
		${CMAKE_CURRENT_LIST_DIR}/include/gdnutil/mvc.hpp
		${CMAKE_CURRENT_LIST_DIR}/include/gdnutil/node_pool.hpp
		${CMAKE_CURRENT_LIST_DIR}/include/gdnutil/objects.hpp
		${CMAKE_CURRENT_LIST_DIR}/include/gdnutil/packed_scene.hpp
		${CMAKE_CURRENT_LIST_DIR}/include/gdnutil/packed_scene_pool.hpp
		${CMAKE_CURRENT_LIST_DIR}/include/gdnutil/placeholder.hpp
		${CMAKE_CURRENT_LIST_DIR}/include/gdnutil/placeholder_control.hpp
		${CMAKE_CURRENT_LIST_DIR}/include/gdnutil/process_when_visible.hpp
		${CMAKE_CURRENT_LIST_DIR}/include/gdnutil/profiling.hpp
		${CMAKE_CURRENT_LIST_DIR}/include/gdnutil/register.hpp
		${CMAKE_CURRENT_LIST_DIR}/include/gdnutil/scene.hpp
		${CMAKE_CURRENT_LIST_DIR}/include/gdnutil/scene_helper.hpp
		${CMAKE_CURRENT_LIST_DIR}/include/gdnutil/string_helpers.hpp
		${CMAKE_CURRENT_LIST_DIR}/include/gdnutil/strings.hpp
		${CMAKE_CURRENT_LIST_DIR}/include/gdnutil/tree.hpp
		${CMAKE_CURRENT_LIST_DIR}/include/gdnutil/vs_helpers.hpp
)
include(CMakePackageConfigHelpers)
install(TARGETS gdnutil EXPORT gdnutil-targets FILE_SET HEADERS)
install(EXPORT gdnutil-targets FILE gdnutil-targets.cmake NAMESPACE gdnutil:: DESTINATION lib/cmake/gdnutil)
configure_package_config_file(
    "${CMAKE_CURRENT_LIST_DIR}/cmake/gdnutil-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/gdnutil-config.cmake"
    INSTALL_DESTINATION lib/cmake/gdnutil
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/gdnutil-config.cmake" DESTINATION lib/cmake/gdnutil)
