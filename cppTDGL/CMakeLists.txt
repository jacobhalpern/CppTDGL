cmake_minimum_required(VERSION 3.24)

project(CppTDGL
    VERSION 0.1.0
    DESCRIPTION "Native C++ TDGL simulation application"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

file(GLOB_RECURSE CPPTDGL_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

file(GLOB_RECURSE CPPTDGL_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
)

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

option(CPPTDGL_ENABLE_QT "Build optional Qt Widgets application shell" OFF)

if (CPPTDGL_ENABLE_QT)
    find_package(Qt6 REQUIRED COMPONENTS Widgets)

    add_executable(CppTDGLQtApp
        "${CMAKE_CURRENT_SOURCE_DIR}/apps/CppTDGLQtApp.cpp"
    )

    target_link_libraries(CppTDGLQtApp
        PRIVATE
            CppTDGL::cppTDGL
            Qt6::Widgets
    )

    target_compile_features(CppTDGLQtApp PRIVATE cxx_std_20)

    set_target_properties(CppTDGLQtApp
        PROPERTIES
            WIN32_EXECUTABLE TRUE
    )
endif()

enable_testing()

add_executable(CppTDGLSmokeCli "${CMAKE_CURRENT_SOURCE_DIR}/apps/CppTDGLSmokeCli.cpp")
target_link_libraries(CppTDGLSmokeCli PRIVATE CppTDGL::cppTDGL)

add_executable(CoreModelSmokeTest "${CMAKE_CURRENT_SOURCE_DIR}/tests/core_tests/CoreModelSmokeTest.cpp")
target_link_libraries(CoreModelSmokeTest PRIVATE CppTDGL::cppTDGL)
add_test(NAME CoreModelSmokeTest COMMAND CoreModelSmokeTest)

add_executable(DeviceMeshIntegrationSmokeTest "${CMAKE_CURRENT_SOURCE_DIR}/tests/core_tests/DeviceMeshIntegrationSmokeTest.cpp")
target_link_libraries(DeviceMeshIntegrationSmokeTest PRIVATE CppTDGL::cppTDGL)
add_test(NAME DeviceMeshIntegrationSmokeTest COMMAND DeviceMeshIntegrationSmokeTest)

add_executable(MeshSmokeTest "${CMAKE_CURRENT_SOURCE_DIR}/tests/core_tests/MeshSmokeTest.cpp")
target_link_libraries(MeshSmokeTest PRIVATE CppTDGL::cppTDGL)
add_test(NAME MeshSmokeTest COMMAND MeshSmokeTest)

add_executable(MeshGeneratorSmokeTest "${CMAKE_CURRENT_SOURCE_DIR}/tests/core_tests/MeshGeneratorSmokeTest.cpp")
target_link_libraries(MeshGeneratorSmokeTest PRIVATE CppTDGL::cppTDGL)
add_test(NAME MeshGeneratorSmokeTest COMMAND MeshGeneratorSmokeTest)

add_executable(MeshOperatorsSmokeTest "${CMAKE_CURRENT_SOURCE_DIR}/tests/core_tests/MeshOperatorsSmokeTest.cpp")
target_link_libraries(MeshOperatorsSmokeTest PRIVATE CppTDGL::cppTDGL)
add_test(NAME MeshOperatorsSmokeTest COMMAND MeshOperatorsSmokeTest)

add_executable(SparseMatrixSmokeTest "${CMAKE_CURRENT_SOURCE_DIR}/tests/core_tests/SparseMatrixSmokeTest.cpp")
target_link_libraries(SparseMatrixSmokeTest PRIVATE CppTDGL::cppTDGL)
add_test(NAME SparseMatrixSmokeTest COMMAND SparseMatrixSmokeTest)

add_executable(SolverStateSmokeTest "${CMAKE_CURRENT_SOURCE_DIR}/tests/core_tests/SolverStateSmokeTest.cpp")
target_link_libraries(SolverStateSmokeTest PRIVATE CppTDGL::cppTDGL)
add_test(NAME SolverStateSmokeTest COMMAND SolverStateSmokeTest)

add_executable(TDGLSolverSmokeTest "${CMAKE_CURRENT_SOURCE_DIR}/tests/core_tests/TDGLSolverSmokeTest.cpp")
target_link_libraries(TDGLSolverSmokeTest PRIVATE CppTDGL::cppTDGL)
add_test(NAME TDGLSolverSmokeTest COMMAND TDGLSolverSmokeTest)

add_executable(SolutionSmokeTest "${CMAKE_CURRENT_SOURCE_DIR}/tests/core_tests/SolutionSmokeTest.cpp")
target_link_libraries(SolutionSmokeTest PRIVATE CppTDGL::cppTDGL)
add_test(NAME SolutionSmokeTest COMMAND SolutionSmokeTest)

add_executable(SparseMeshOperatorsSmokeTest "${CMAKE_CURRENT_SOURCE_DIR}/tests/core_tests/SparseMeshOperatorsSmokeTest.cpp")
target_link_libraries(SparseMeshOperatorsSmokeTest PRIVATE CppTDGL::cppTDGL)
add_test(NAME SparseMeshOperatorsSmokeTest COMMAND SparseMeshOperatorsSmokeTest)

add_executable(FiniteVolumeSparseOperatorsSmokeTest "${CMAKE_CURRENT_SOURCE_DIR}/tests/core_tests/FiniteVolumeSparseOperatorsSmokeTest.cpp")
target_link_libraries(FiniteVolumeSparseOperatorsSmokeTest PRIVATE CppTDGL::cppTDGL)
add_test(NAME FiniteVolumeSparseOperatorsSmokeTest COMMAND FiniteVolumeSparseOperatorsSmokeTest)

add_executable(MeshSerializerSmokeTest "${CMAKE_CURRENT_SOURCE_DIR}/tests/io_tests/MeshSerializerSmokeTest.cpp")
target_link_libraries(MeshSerializerSmokeTest PRIVATE CppTDGL::cppTDGL)
add_test(NAME MeshSerializerSmokeTest COMMAND MeshSerializerSmokeTest)

add_executable(Hdf5SolutionPersistenceSmokeTest "${CMAKE_CURRENT_SOURCE_DIR}/tests/io_tests/Hdf5SolutionPersistenceSmokeTest.cpp")
target_link_libraries(Hdf5SolutionPersistenceSmokeTest PRIVATE CppTDGL::cppTDGL)
add_test(NAME Hdf5SolutionPersistenceSmokeTest COMMAND Hdf5SolutionPersistenceSmokeTest)

add_executable(SimulationRunnerSmokeTest "${CMAKE_CURRENT_SOURCE_DIR}/tests/simulation_tests/SimulationRunnerSmokeTest.cpp")
target_link_libraries(SimulationRunnerSmokeTest PRIVATE CppTDGL::cppTDGL)
add_test(NAME SimulationRunnerSmokeTest COMMAND SimulationRunnerSmokeTest)

add_test(NAME CppTDGLSmokeCliTest COMMAND CppTDGLSmokeCli)

add_executable(ApplicationInfoSmokeTest "${CMAKE_CURRENT_SOURCE_DIR}/tests/ui_tests/ApplicationInfoSmokeTest.cpp")
target_link_libraries(ApplicationInfoSmokeTest PRIVATE CppTDGL::cppTDGL)
add_test(NAME ApplicationInfoSmokeTest COMMAND ApplicationInfoSmokeTest)

add_executable(ProjectWizardModelSmokeTest "${CMAKE_CURRENT_SOURCE_DIR}/tests/ui_tests/ProjectWizardModelSmokeTest.cpp")
target_link_libraries(ProjectWizardModelSmokeTest PRIVATE CppTDGL::cppTDGL)
add_test(NAME ProjectWizardModelSmokeTest COMMAND ProjectWizardModelSmokeTest)

add_executable(GeometryEditorModelSmokeTest "${CMAKE_CURRENT_SOURCE_DIR}/tests/ui_tests/GeometryEditorModelSmokeTest.cpp")
target_link_libraries(GeometryEditorModelSmokeTest PRIVATE CppTDGL::cppTDGL)
add_test(NAME GeometryEditorModelSmokeTest COMMAND GeometryEditorModelSmokeTest)

add_executable(MeshPreviewModelSmokeTest "${CMAKE_CURRENT_SOURCE_DIR}/tests/ui_tests/MeshPreviewModelSmokeTest.cpp")
target_link_libraries(MeshPreviewModelSmokeTest PRIVATE CppTDGL::cppTDGL)
add_test(NAME MeshPreviewModelSmokeTest COMMAND MeshPreviewModelSmokeTest)

add_executable(SolverSettingsModelSmokeTest "${CMAKE_CURRENT_SOURCE_DIR}/tests/ui_tests/SolverSettingsModelSmokeTest.cpp")
target_link_libraries(SolverSettingsModelSmokeTest PRIVATE CppTDGL::cppTDGL)
add_test(NAME SolverSettingsModelSmokeTest COMMAND SolverSettingsModelSmokeTest)

add_executable(SerializationFormatSmokeTest "${CMAKE_CURRENT_SOURCE_DIR}/tests/io_tests/SerializationFormatSmokeTest.cpp")
target_link_libraries(SerializationFormatSmokeTest PRIVATE CppTDGL::cppTDGL)
add_test(NAME SerializationFormatSmokeTest COMMAND SerializationFormatSmokeTest)

add_executable(ProjectSerializerMeshIntegrationSmokeTest "${CMAKE_CURRENT_SOURCE_DIR}/tests/io_tests/ProjectSerializerMeshIntegrationSmokeTest.cpp")
target_link_libraries(ProjectSerializerMeshIntegrationSmokeTest PRIVATE CppTDGL::cppTDGL)
add_test(NAME ProjectSerializerMeshIntegrationSmokeTest COMMAND ProjectSerializerMeshIntegrationSmokeTest)

add_executable(ProjectSerializerSmokeTest "${CMAKE_CURRENT_SOURCE_DIR}/tests/io_tests/ProjectSerializerSmokeTest.cpp")
target_link_libraries(ProjectSerializerSmokeTest PRIVATE CppTDGL::cppTDGL)
add_test(NAME ProjectSerializerSmokeTest COMMAND ProjectSerializerSmokeTest)
