cmake_minimum_required(VERSION 3.14)
project(maybe-seventeen VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    string(REGEX REPLACE "/EH[a-z]+" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHs-c-")
    add_definitions(-D_HAS_EXCEPTIONS=0)
else()
    string(REGEX REPLACE "-fexceptions" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
endif()

add_library(maybe-seventeen INTERFACE)

target_include_directories(maybe-seventeen INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>
)

install(FILES include/maybe.hpp DESTINATION include)

install(TARGETS maybe-seventeen
	EXPORT maybe-seventeen-targets
	INCLUDES DESTINATION include
)

install(EXPORT maybe-seventeen-targets
	FILE maybe-seventeen-targets.cmake
	NAMESPACE maybe-seventeen::
	DESTINATION lib/cmake/maybe-seventeen
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/maybe-seventeen-config.cmake.in
	${CMAKE_CURRENT_BINARY_DIR}/maybe-seventeen-config.cmake
	INSTALL_DESTINATION lib/cmake/maybe-seventeen
)

write_basic_package_version_file(
	${CMAKE_CURRENT_BINARY_DIR}/maybe-seventeen-config-version.cmake
	VERSION ${PROJECT_VERSION}
	COMPATIBILITY SameMajorVersion
)

install(FILES
	${CMAKE_CURRENT_BINARY_DIR}/maybe-seventeen-config.cmake
	${CMAKE_CURRENT_BINARY_DIR}/maybe-seventeen-config-version.cmake
	DESTINATION lib/cmake/maybe-seventeen
)

add_executable(maybe-example example/main.cpp)
target_link_libraries(maybe-example PRIVATE maybe-seventeen)

if(MSVC)
    target_compile_options(maybe-example PRIVATE /EHs-c-)
    target_compile_definitions(maybe-example PRIVATE _HAS_EXCEPTIONS=0)
else()
    target_compile_options(maybe-example PRIVATE -fno-exceptions)
endif()
