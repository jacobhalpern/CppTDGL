#include "EdgeMesh.hpp"
#include "Mesh.hpp"
#include "MeshOperators.hpp"
#include "SparseMatrix.hpp"

#include <cmath>
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
        const cppTDGL::Mesh mesh = makeSquareMesh();
        const cppTDGL::EdgeMesh edgeMesh = cppTDGL::EdgeMesh::fromMesh(mesh);
        const std::vector<double> controlVolumes =
            cppTDGL::MeshOperators::vertexControlVolumes(mesh);

        const cppTDGL::SparseMatrix gradient =
            cppTDGL::MeshOperators::finiteVolumeGradientOperator(edgeMesh, mesh.vertexCount());

        failIfFalse(gradient.isValid(), "Finite-volume gradient matrix should be valid.");
        failIfFalse(gradient.rowCount() == edgeMesh.edgeCount(), "Gradient row count should equal edge count.");
        failIfFalse(gradient.columnCount() == mesh.vertexCount(), "Gradient column count should equal vertex count.");
        failIfFalse(gradient.entries().size() == 2 * edgeMesh.edgeCount(), "Each edge should produce two gradient entries.");

        const std::vector<double> constantField(mesh.vertexCount(), 5.0);
        const std::vector<double> constantGradient = gradient.multiply(constantField);

        for (const double value : constantGradient) {
            failIfNotNear(value, 0.0, "Finite-volume gradient of a constant field should be zero.");
        }

        const std::vector<double> xField{0.0, 1.0, 1.0, 0.0};
        const std::vector<double> xGradient = gradient.multiply(xField);

        for (std::size_t edgeIndex = 0; edgeIndex < edgeMesh.edgeCount(); ++edgeIndex) {
            const cppTDGL::MeshEdge& edge = edgeMesh.edges()[edgeIndex];
            const double expected = (xField[edge.b] - xField[edge.a]) / edge.length;

            failIfNotNear(
                xGradient[edgeIndex],
                expected,
                "Finite-volume gradient should equal scalar difference divided by edge length."
            );
        }

        const cppTDGL::SparseMatrix divergence =
            cppTDGL::MeshOperators::finiteVolumeDivergenceOperator(
                edgeMesh,
                controlVolumes,
                mesh.vertexCount()
            );

        failIfFalse(divergence.isValid(), "Finite-volume divergence matrix should be valid.");
        failIfFalse(divergence.rowCount() == mesh.vertexCount(), "Divergence row count should equal vertex count.");
        failIfFalse(divergence.columnCount() == edgeMesh.edgeCount(), "Divergence column count should equal edge count.");
        failIfFalse(divergence.entries().size() == 2 * edgeMesh.edgeCount(), "Each edge should produce two divergence entries.");

        const std::vector<double> zeroFlux(edgeMesh.edgeCount(), 0.0);
        const std::vector<double> zeroDivergence = divergence.multiply(zeroFlux);

        for (const double value : zeroDivergence) {
            failIfNotNear(value, 0.0, "Zero edge flux should have zero divergence.");
        }

        for (std::size_t edgeIndex = 0; edgeIndex < edgeMesh.edgeCount(); ++edgeIndex) {
            const cppTDGL::MeshEdge& edge = edgeMesh.edges()[edgeIndex];

            failIfNotNear(
                divergence.valueAt(edge.a, edgeIndex),
                -1.0 / controlVolumes[edge.a],
                "Divergence source vertex coefficient should be -1/controlVolume."
            );

            failIfNotNear(
                divergence.valueAt(edge.b, edgeIndex),
                1.0 / controlVolumes[edge.b],
                "Divergence destination vertex coefficient should be +1/controlVolume."
            );
        }

        std::cout << "Finite-volume sparse operators smoke test passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Finite-volume sparse operators smoke test exception: " << error.what() << '\n';
        return 1;
    }
}
