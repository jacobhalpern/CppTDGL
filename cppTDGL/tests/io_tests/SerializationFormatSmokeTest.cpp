#include "SerializationFormat.hpp"

#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {

void failIfFalse(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

} // namespace

int main() {
    try {
        namespace format = cppTDGL::SerializationFormat;

        failIfFalse(format::earliestSupportedProjectFileVersion == 1, "Earliest supported project version should be 1.");
        failIfFalse(format::latestProjectFileVersion == 2, "Latest project version should be 2.");
        failIfFalse(format::latestMeshFileVersion == 1, "Latest mesh version should be 1.");
        failIfFalse(format::latestSolutionScaffoldFileVersion == 1, "Latest solution scaffold version should be 1.");

        failIfFalse(format::projectFileExtension == std::string_view(".cpptdgl"), "Project extension should be .cpptdgl.");
        failIfFalse(format::meshFileExtension == std::string_view(".mesh"), "Mesh extension should be .mesh.");
        failIfFalse(format::solutionFileExtension == std::string_view(".h5"), "Solution extension should be .h5.");

        failIfFalse(format::projectFileSignature == std::string_view("CPPTDGL_PROJECT_VERSION"), "Project signature should match.");
        failIfFalse(format::meshFileSignature == std::string_view("CPPTDGL_MESH_VERSION"), "Mesh signature should match.");
        failIfFalse(format::solutionScaffoldFileSignature == std::string_view("CPPTDGL_SOLUTION_SCAFFOLD_VERSION"), "Solution scaffold signature should match.");

        failIfFalse(!format::isSupportedProjectFileVersion(0), "Project version 0 should not be supported.");
        failIfFalse(format::isSupportedProjectFileVersion(1), "Project version 1 should be supported.");
        failIfFalse(format::isSupportedProjectFileVersion(2), "Project version 2 should be supported.");
        failIfFalse(!format::isSupportedProjectFileVersion(3), "Project version 3 should not be supported yet.");

        failIfFalse(!format::isSupportedMeshFileVersion(0), "Mesh version 0 should not be supported.");
        failIfFalse(format::isSupportedMeshFileVersion(1), "Mesh version 1 should be supported.");
        failIfFalse(!format::isSupportedMeshFileVersion(2), "Mesh version 2 should not be supported yet.");

        failIfFalse(!format::isSupportedSolutionScaffoldFileVersion(0), "Solution scaffold version 0 should not be supported.");
        failIfFalse(format::isSupportedSolutionScaffoldFileVersion(1), "Solution scaffold version 1 should be supported.");
        failIfFalse(!format::isSupportedSolutionScaffoldFileVersion(2), "Solution scaffold version 2 should not be supported yet.");

        std::cout << "Serialization format smoke test passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Serialization format smoke test exception: " << error.what() << '\n';
        return 1;
    }
}
