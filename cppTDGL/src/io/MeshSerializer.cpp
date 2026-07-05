#include "MeshSerializer.hpp"

#include "SerializationFormat.hpp"

#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace cppTDGL {
namespace {

[[nodiscard]] std::runtime_error parseError(const std::string& message) {
    return std::runtime_error("CppTDGL mesh parse error: " + message);
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

} // namespace

void MeshSerializer::save(const Mesh& mesh, const std::filesystem::path& path) {
    const std::vector<std::string> errors = mesh.validationErrors();
    if (!errors.empty()) {
        throw std::invalid_argument("Cannot save an invalid Mesh.");
    }

    std::ofstream output(path);

    if (!output) {
        throw std::runtime_error("Failed to open mesh file for writing: " + path.string());
    }

    output << std::setprecision(17);

    output << SerializationFormat::meshFileSignature << ' ' << SerializationFormat::latestMeshFileVersion << '\n';

    output << "VERTICES " << mesh.vertices().size() << '\n';
    for (const Point2D& point : mesh.vertices()) {
        output << "VERTEX " << point.x << ' ' << point.y << '\n';
    }

    output << "TRIANGLES " << mesh.triangles().size() << '\n';
    for (const Triangle& triangle : mesh.triangles()) {
        output << "TRIANGLE "
               << triangle.a << ' '
               << triangle.b << ' '
               << triangle.c << '\n';
    }

    output << "BOUNDARY_VERTICES " << mesh.boundaryVertexIndices().size() << '\n';
    for (const std::size_t index : mesh.boundaryVertexIndices()) {
        output << "BOUNDARY_VERTEX " << index << '\n';
    }

    output << "END_MESH\n";

    if (!output) {
        throw std::runtime_error("Failed while writing mesh file: " + path.string());
    }
}

Mesh MeshSerializer::load(const std::filesystem::path& path) {
    std::ifstream input(path);

    if (!input) {
        throw std::runtime_error("Failed to open mesh file for reading: " + path.string());
    }

    expectToken(input, std::string(SerializationFormat::meshFileSignature));

    int version = 0;
    if (!(input >> version)) {
        throw parseError("Failed to read mesh file version.");
    }

    if (!SerializationFormat::isSupportedMeshFileVersion(version)) {
        throw parseError("Unsupported mesh file version: " + std::to_string(version) + ".");
    }

    expectToken(input, "VERTICES");

    std::size_t vertexCount = 0;
    if (!(input >> vertexCount)) {
        throw parseError("Failed to read vertex count.");
    }

    std::vector<Point2D> vertices;
    vertices.reserve(vertexCount);

    for (std::size_t i = 0; i < vertexCount; ++i) {
        expectToken(input, "VERTEX");

        Point2D point{};
        if (!(input >> point.x >> point.y)) {
            throw parseError("Failed to read vertex " + std::to_string(i) + ".");
        }

        vertices.push_back(point);
    }

    expectToken(input, "TRIANGLES");

    std::size_t triangleCount = 0;
    if (!(input >> triangleCount)) {
        throw parseError("Failed to read triangle count.");
    }

    std::vector<Triangle> triangles;
    triangles.reserve(triangleCount);

    for (std::size_t i = 0; i < triangleCount; ++i) {
        expectToken(input, "TRIANGLE");

        Triangle triangle{};
        if (!(input >> triangle.a >> triangle.b >> triangle.c)) {
            throw parseError("Failed to read triangle " + std::to_string(i) + ".");
        }

        triangles.push_back(triangle);
    }

    expectToken(input, "BOUNDARY_VERTICES");

    std::size_t boundaryVertexCount = 0;
    if (!(input >> boundaryVertexCount)) {
        throw parseError("Failed to read boundary vertex count.");
    }

    std::vector<std::size_t> boundaryVertices;
    boundaryVertices.reserve(boundaryVertexCount);

    for (std::size_t i = 0; i < boundaryVertexCount; ++i) {
        expectToken(input, "BOUNDARY_VERTEX");

        std::size_t index = 0;
        if (!(input >> index)) {
            throw parseError("Failed to read boundary vertex " + std::to_string(i) + ".");
        }

        boundaryVertices.push_back(index);
    }

    expectToken(input, "END_MESH");

    Mesh mesh(std::move(vertices), std::move(triangles), std::move(boundaryVertices));

    const std::vector<std::string> errors = mesh.validationErrors();
    if (!errors.empty()) {
        throw parseError("Loaded mesh is invalid.");
    }

    return mesh;
}

} // namespace cppTDGL
