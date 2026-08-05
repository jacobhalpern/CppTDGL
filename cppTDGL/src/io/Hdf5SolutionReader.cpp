#include "Hdf5SolutionReader.hpp"

#include "SerializationFormat.hpp"

#include <fstream>
#include <iomanip>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace cppTDGL {
namespace {

[[nodiscard]] std::runtime_error parseError(const std::string& message) {
    return std::runtime_error("CppTDGL solution scaffold parse error: " + message);
}

void expectToken(std::istream& input, const std::string& expected) {
    std::string actual;
    if (!(input >> actual)) {
        throw parseError("Expected token '" + expected + "', but reached end of file.");
    }

    if (actual != expected) {
        throw parseError("Expected token '" + expected + "', but found '" + actual + "'.");
    }
}

[[nodiscard]] Mesh readMesh(std::istream& input) {
    expectToken(input, "MESH");

    expectToken(input, "VERTICES");

    std::size_t vertexCount = 0;
    if (!(input >> vertexCount)) {
        throw parseError("Failed to read mesh vertex count.");
    }

    std::vector<Point2D> vertices;
    vertices.reserve(vertexCount);

    for (std::size_t i = 0; i < vertexCount; ++i) {
        expectToken(input, "VERTEX");

        Point2D point{};
        if (!(input >> point.x >> point.y)) {
            throw parseError("Failed to read mesh vertex " + std::to_string(i) + ".");
        }

        vertices.push_back(point);
    }

    expectToken(input, "TRIANGLES");

    std::size_t triangleCount = 0;
    if (!(input >> triangleCount)) {
        throw parseError("Failed to read mesh triangle count.");
    }

    std::vector<Triangle> triangles;
    triangles.reserve(triangleCount);

    for (std::size_t i = 0; i < triangleCount; ++i) {
        expectToken(input, "TRIANGLE");

        Triangle triangle{};
        if (!(input >> triangle.a >> triangle.b >> triangle.c)) {
            throw parseError("Failed to read mesh triangle " + std::to_string(i) + ".");
        }

        triangles.push_back(triangle);
    }

    expectToken(input, "BOUNDARY_VERTICES");

    std::size_t boundaryVertexCount = 0;
    if (!(input >> boundaryVertexCount)) {
        throw parseError("Failed to read mesh boundary vertex count.");
    }

    std::vector<std::size_t> boundaryVertices;
    boundaryVertices.reserve(boundaryVertexCount);

    for (std::size_t i = 0; i < boundaryVertexCount; ++i) {
        expectToken(input, "BOUNDARY_VERTEX");

        std::size_t index = 0;
        if (!(input >> index)) {
            throw parseError("Failed to read mesh boundary vertex " + std::to_string(i) + ".");
        }

        boundaryVertices.push_back(index);
    }

    expectToken(input, "END_MESH");

    return Mesh(std::move(vertices), std::move(triangles), std::move(boundaryVertices));
}

[[nodiscard]] std::vector<double> readDoubleVector(
    std::istream& input,
    const std::string& expectedToken
) {
    expectToken(input, expectedToken);

    std::size_t valueCount = 0;
    if (!(input >> valueCount)) {
        throw parseError("Failed to read " + expectedToken + " value count.");
    }

    std::vector<double> values;
    values.reserve(valueCount);

    for (std::size_t i = 0; i < valueCount; ++i) {
        expectToken(input, "VALUE");

        double value = 0.0;
        if (!(input >> value)) {
            throw parseError("Failed to read " + expectedToken + " value " + std::to_string(i) + ".");
        }

        values.push_back(value);
    }

    return values;
}

[[nodiscard]] SolverFrame readFrame(std::istream& input) {
    expectToken(input, "FRAME");

    std::size_t frameIndex = 0;
    SolverFrame frame{};

    if (!(input >> frameIndex >> frame.time)) {
        throw parseError("Failed to read frame header.");
    }

    expectToken(input, "ORDER_PARAMETER");

    std::size_t orderParameterCount = 0;
    if (!(input >> orderParameterCount)) {
        throw parseError("Failed to read order parameter count.");
    }

    frame.orderParameter.real.reserve(orderParameterCount);
    frame.orderParameter.imaginary.reserve(orderParameterCount);

    for (std::size_t i = 0; i < orderParameterCount; ++i) {
        expectToken(input, "COMPLEX");

        double real = 0.0;
        double imaginary = 0.0;

        if (!(input >> real >> imaginary)) {
            throw parseError("Failed to read order parameter value " + std::to_string(i) + ".");
        }

        frame.orderParameter.real.push_back(real);
        frame.orderParameter.imaginary.push_back(imaginary);
    }

    frame.scalarPotential.values = readDoubleVector(input, "SCALAR_POTENTIAL");
    frame.vectorPotential.values = readDoubleVector(input, "VECTOR_POTENTIAL");
    frame.currentDensity.values = readDoubleVector(input, "CURRENT_DENSITY");

    expectToken(input, "END_FRAME");

    static_cast<void>(frameIndex);
    return frame;
}

} // namespace

std::string Hdf5SolutionReader::className() const {
    return "Hdf5SolutionReader";
}

Solution Hdf5SolutionReader::read(const std::filesystem::path& path) {
    std::ifstream input(path);

    if (!input) {
        throw std::runtime_error("Failed to open solution file for reading: " + path.string());
    }

    expectToken(input, std::string(SerializationFormat::solutionScaffoldFileSignature));

    int version = 0;
    if (!(input >> version)) {
        throw parseError("Failed to read solution scaffold file version.");
    }

    if (!SerializationFormat::isSupportedSolutionScaffoldFileVersion(version)) {
        throw parseError("Unsupported solution scaffold file version: " + std::to_string(version) + ".");
    }

    expectToken(input, "PROJECT_NAME");

    std::string projectName;
    if (!(input >> std::quoted(projectName))) {
        throw parseError("Failed to read solution project name.");
    }

    expectToken(input, "RESULT_PATH");

    std::string resultPathString;
    if (!(input >> std::quoted(resultPathString))) {
        throw parseError("Failed to read solution result path.");
    }

    expectToken(input, "COMPLETED");

    int completed = 0;
    if (!(input >> completed)) {
        throw parseError("Failed to read solution completion flag.");
    }

    expectToken(input, "DIAGNOSTICS");

    std::size_t diagnosticCount = 0;
    if (!(input >> diagnosticCount)) {
        throw parseError("Failed to read solution diagnostic count.");
    }

    std::vector<std::string> diagnostics;
    diagnostics.reserve(diagnosticCount);

    for (std::size_t i = 0; i < diagnosticCount; ++i) {
        expectToken(input, "DIAGNOSTIC");

        std::string diagnostic;
        if (!(input >> std::quoted(diagnostic))) {
            throw parseError("Failed to read solution diagnostic " + std::to_string(i) + ".");
        }

        diagnostics.push_back(std::move(diagnostic));
    }

    expectToken(input, "MESH_PRESENT");

    int meshPresent = 0;
    if (!(input >> meshPresent)) {
        throw parseError("Failed to read solution mesh presence flag.");
    }

    std::optional<Mesh> mesh;
    if (meshPresent != 0) {
        mesh = readMesh(input);
    }

    expectToken(input, "FRAMES");

    std::size_t frameCount = 0;
    if (!(input >> frameCount)) {
        throw parseError("Failed to read solution frame count.");
    }

    std::vector<SolverFrame> frames;
    frames.reserve(frameCount);

    for (std::size_t i = 0; i < frameCount; ++i) {
        frames.push_back(readFrame(input));
    }

    expectToken(input, "END_SOLUTION");

    Solution solution;

    if (mesh.has_value()) {
        solution = Solution(
            std::move(projectName),
            std::move(*mesh),
            std::move(frames),
            std::filesystem::path(resultPathString)
        );
    } else {
        if (!frames.empty()) {
            throw parseError("Solution frames cannot be loaded without an attached mesh.");
        }

        solution = Solution(
            std::move(projectName),
            std::filesystem::path(resultPathString),
            0
        );
    }

    solution.setCompleted(completed != 0);

    for (std::string& diagnostic : diagnostics) {
        solution.addDiagnostic(std::move(diagnostic));
    }

    const std::vector<std::string> validationErrors = solution.validationErrors();
    if (!validationErrors.empty()) {
        throw parseError("Loaded solution is invalid: " + validationErrors.front());
    }

    return solution;
}

} // namespace cppTDGL
