cmake_minimum_required(VERSION 3.4...3.18)
project(audioutils)

add_subdirectory(../submodules/pybind11 ${CMAKE_CURRENT_BINARY_DIR}/pybind11)
pybind11_add_module(audioutils src/main.cpp)

# EXAMPLE_VERSION_INFO is defined by setup.py and passed into the C++ code as a
# define (VERSION_INFO) here.
set(EXAMPLE_VERSION_INFO 0.0.1)
target_compile_definitions(audioutils PRIVATE VERSION_INFO=${EXAMPLE_VERSION_INFO})

# audio_utils source files
set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)


target_sources(audioutils PRIVATE ${SOURCE_FILES})

add_compile_definitions(INCLUDE_PYTHON_BINDING=true)