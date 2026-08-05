#include "EdgeMesh.hpp"
#include "Hdf5SolutionReader.hpp"
#include "Hdf5SolutionWriter.hpp"
#include "Mesh.hpp"
#include "Solution.hpp"
#include "SolverState.hpp"

#include <cmath>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

constexpr double kTolerance = 1.0e-12;

void failIfFalse(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void failIfNotNear(double actual, double expected, const std::string& message) {
    if (std::abs(actual - expected) > kTolerance) {
        throw std::runtime_error(
            message + " Expected " + std::to_string(expected) +
            ", got " + std::to_string(actual) + "."
        );
    }
}

cppTDGL::Mesh makeSquareMesh() {
    const std::vector<cppTDGL::Point2D> vertices{
        {0.0, 0.0},
        {1.0, 0.0},
        {1.0, 1.0},
        {0.0, 1.0},
    };

    const std::vector<cppTDGL::Triangle> triangles{
        {0, 1, 2},
        {0, 2, 3},
    };

    const std::vector<std::size_t> boundaryVertices{
        0, 1, 2, 3,
    };

    return cppTDGL::Mesh(vertices, triangles, boundaryVertices);
}

} // namespace

int main() {
    try {
        cppTDGL::Hdf5SolutionWriter writer;
        cppTDGL::Hdf5SolutionReader reader;

        failIfFalse(writer.className() == "Hdf5SolutionWriter", "Writer className should be preserved.");
        failIfFalse(reader.className() == "Hdf5SolutionReader", "Reader className should be preserved.");

        const cppTDGL::Mesh mesh = makeSquareMesh();
        const cppTDGL::EdgeMesh edgeMesh = cppTDGL::EdgeMesh::fromMesh(mesh);

        cppTDGL::SolverState state(mesh);
        state.appendFrame(cppTDGL::SolverFrame::uniformOrderParameterFor(mesh, edgeMesh, 0.0, 1.0, 0.0));
        state.appendFrame(cppTDGL::SolverFrame::uniformOrderParameterFor(mesh, edgeMesh, 0.5, 0.9, 0.1));

        const std::filesystem::path solutionPath =
            std::filesystem::temp_directory_path() / "CppTDGL_Hdf5SolutionPersistenceSmokeTest.h5";

        cppTDGL::Solution original =
            cppTDGL::Solution::fromSolverState(
                "hdf5_solution_persistence_smoke_test",
                state,
                solutionPath
            );

        original.setCompleted(true);
        original.addDiagnostic("diagnostic one");
        original.addDiagnostic("diagnostic two");

        failIfFalse(original.isValid(), "Original solution should be valid.");
        failIfFalse(original.hasMesh(), "Original solution should have a mesh.");
        failIfFalse(original.frameCount() == 2, "Original solution should have two frames.");

        cppTDGL::Hdf5SolutionWriter::write(original);
        const cppTDGL::Solution loaded = cppTDGL::Hdf5SolutionReader::read(solutionPath);

        failIfFalse(loaded.isValid(), "Loaded solution should be valid.");
        failIfFalse(loaded.projectName() == original.projectName(), "Project name should round trip.");
        failIfFalse(loaded.resultPath() == original.resultPath(), "Result path should round trip.");
        failIfFalse(loaded.completed(), "Completion flag should round trip.");
        failIfFalse(loaded.diagnostics().size() == 2, "Diagnostics should round trip.");
        failIfFalse(loaded.diagnostics()[0] == "diagnostic one", "First diagnostic should round trip.");
        failIfFalse(loaded.diagnostics()[1] == "diagnostic two", "Second diagnostic should round trip.");

        failIfFalse(loaded.hasMesh(), "Loaded solution should preserve mesh presence.");
        failIfFalse(loaded.mesh().vertexCount() == 4, "Loaded mesh vertex count should round trip.");
        failIfFalse(loaded.mesh().triangleCount() == 2, "Loaded mesh triangle count should round trip.");
        failIfFalse(loaded.mesh().boundaryVertexIndices().size() == 4, "Loaded mesh boundary vertices should round trip.");
        failIfNotNear(loaded.mesh().totalArea(), 1.0, "Loaded mesh area should round trip.");

        failIfFalse(loaded.frameCount() == 2, "Loaded solution should preserve frame count.");
        failIfNotNear(loaded.frame(0).time, 0.0, "First frame time should round trip.");
        failIfNotNear(loaded.frame(1).time, 0.5, "Second frame time should round trip.");
        failIfNotNear(loaded.frame(1).orderParameter.real[0], 0.9, "Order parameter real component should round trip.");
        failIfNotNear(loaded.frame(1).orderParameter.imaginary[0], 0.1, "Order parameter imaginary component should round trip.");
        failIfFalse(
            loaded.frame(1).vectorPotential.size() == edgeMesh.edgeCount(),
            "Loaded vector potential should be edge-sized."
        );
        failIfFalse(
            loaded.frame(1).currentDensity.size() == edgeMesh.edgeCount(),
            "Loaded current density should be edge-sized."
        );

        const std::filesystem::path metadataPath =
            std::filesystem::temp_directory_path() / "CppTDGL_Hdf5SolutionPersistenceSmokeTest_MetadataOnly.h5";

        cppTDGL::Solution metadataOnly("metadata_only_solution", metadataPath, 0);
        metadataOnly.addDiagnostic("metadata diagnostic");

        cppTDGL::Hdf5SolutionWriter::write(metadataOnly);
        const cppTDGL::Solution loadedMetadataOnly =
            cppTDGL::Hdf5SolutionReader::read(metadataPath);

        failIfFalse(loadedMetadataOnly.isValid(), "Loaded metadata-only solution should be valid.");
        failIfFalse(!loadedMetadataOnly.hasMesh(), "Metadata-only solution should not have a mesh.");
        failIfFalse(!loadedMetadataOnly.hasFrames(), "Metadata-only solution should not have frames.");
        failIfFalse(loadedMetadataOnly.diagnostics().size() == 1, "Metadata-only diagnostics should round trip.");

        cppTDGL::Solution noPathSolution("no_path_solution", std::filesystem::path{}, 0);

        bool missingPathThrew = false;
        try {
            cppTDGL::Hdf5SolutionWriter::write(noPathSolution);
        } catch (const std::logic_error&) {
            missingPathThrew = true;
        }

        failIfFalse(missingPathThrew, "Writing via solution.resultPath() should throw when the path is missing.");

        std::filesystem::remove(solutionPath);
        std::filesystem::remove(metadataPath);

        std::cout << "HDF5 solution persistence scaffold smoke test passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "HDF5 solution persistence scaffold smoke test exception: " << error.what() << '\n';
        return 1;
    }
}
