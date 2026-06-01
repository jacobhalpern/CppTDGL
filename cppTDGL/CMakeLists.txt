cmake_minimum_required(VERSION 3.24)

project(CppTDGL
    VERSION 0.1.0
    DESCRIPTION "Native C++ TDGL simulation application"
    LANGUAGES CXX
)

# ----------------------------------------------------------------------
# Global C++ configuration
# ----------------------------------------------------------------------

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Make Visual Studio solution folders cleaner.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# ----------------------------------------------------------------------
# Source discovery
# ----------------------------------------------------------------------

file(GLOB_RECURSE CPPTDGL_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

file(GLOB_RECURSE CPPTDGL_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
)

# ----------------------------------------------------------------------
# Core library target
# ----------------------------------------------------------------------

add_library(cppTDGL STATIC
    ${CPPTDGL_SOURCES}
    ${CPPTDGL_HEADERS}
)

add_library(CppTDGL::cppTDGL ALIAS cppTDGL)

target_include_directories(cppTDGL
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/core"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/io"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/simulation"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/ui"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/visualization"
)

target_compile_features(cppTDGL PUBLIC cxx_std_20)

if (MSVC)
    target_compile_options(cppTDGL PRIVATE /W4 /permissive-)
else()
    target_compile_options(cppTDGL PRIVATE -Wall -Wextra -Wpedantic)
endif()

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" PREFIX "Source Files" FILES ${CPPTDGL_SOURCES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" PREFIX "Header Files" FILES ${CPPTDGL_HEADERS})

# ----------------------------------------------------------------------
# Tests
# ----------------------------------------------------------------------

enable_testing()

add_executable(CoreModelSmokeTest
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/core_tests/CoreModelSmokeTest.cpp"
)

target_link_libraries(CoreModelSmokeTest
    PRIVATE
        CppTDGL::cppTDGL
)

add_test(
    NAME CoreModelSmokeTest
    COMMAND CoreModelSmokeTest
)

add_executable(MeshSmokeTest
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/core_tests/MeshSmokeTest.cpp"
)

target_link_libraries(MeshSmokeTest
    PRIVATE
        CppTDGL::cppTDGL
)

add_test(
    NAME MeshSmokeTest
    COMMAND MeshSmokeTest
)

add_executable(MeshOperatorsSmokeTest
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/core_tests/MeshOperatorsSmokeTest.cpp"
)

target_link_libraries(MeshOperatorsSmokeTest
    PRIVATE
        CppTDGL::cppTDGL
)

add_test(
    NAME MeshOperatorsSmokeTest
    COMMAND MeshOperatorsSmokeTest
)

add_executable(SparseMatrixSmokeTest
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/core_tests/SparseMatrixSmokeTest.cpp"
)

target_link_libraries(SparseMatrixSmokeTest
    PRIVATE
        CppTDGL::cppTDGL
)

add_test(
    NAME SparseMatrixSmokeTest
    COMMAND SparseMatrixSmokeTest
)

add_executable(ProjectSerializerSmokeTest
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/io_tests/ProjectSerializerSmokeTest.cpp"
)

target_link_libraries(ProjectSerializerSmokeTest
    PRIVATE
        CppTDGL::cppTDGL
)

add_test(
    NAME ProjectSerializerSmokeTest
    COMMAND ProjectSerializerSmokeTest
)
