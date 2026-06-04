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

        failIfFalse(edgeMesh.edgeCount() == 5, "Square mesh should have 5 unique edges.");

        const cppTDGL::SparseMatrix incidence =
            cppTDGL::MeshOperators::edgeVertexIncidenceMatrix(edgeMesh, mesh.vertexCount());

        failIfFalse(incidence.isValid(), "Incidence matrix should be valid.");
        failIfFalse(incidence.rowCount() == edgeMesh.edgeCount(), "Incidence matrix row count should equal edge count.");
        failIfFalse(incidence.columnCount() == mesh.vertexCount(), "Incidence matrix column count should equal vertex count.");
        failIfFalse(incidence.entries().size() == 2 * edgeMesh.edgeCount(), "Each edge should contribute two incidence entries.");

        const cppTDGL::SparseMatrix gradient =
            cppTDGL::MeshOperators::graphGradientOperator(edgeMesh, mesh.vertexCount());

        failIfFalse(gradient.isValid(), "Graph gradient matrix should be valid.");
        failIfFalse(gradient.rowCount() == edgeMesh.edgeCount(), "Gradient row count should equal edge count.");
        failIfFalse(gradient.columnCount() == mesh.vertexCount(), "Gradient column count should equal vertex count.");

        const std::vector<double> scalarField{0.0, 1.0, 2.0, 3.0};
        const std::vector<double> edgeDifferences = gradient.multiply(scalarField);

        failIfFalse(edgeDifferences.size() == edgeMesh.edgeCount(), "Gradient result should have one value per edge.");

        for (std::size_t edgeIndex = 0; edgeIndex < edgeMesh.edgeCount(); ++edgeIndex) {
            const cppTDGL::MeshEdge& edge = edgeMesh.edges()[edgeIndex];
            const double expected = scalarField[edge.b] - scalarField[edge.a];

            failIfNotNear(
                edgeDifferences[edgeIndex],
                expected,
                "Gradient result should equal scalar difference across each oriented edge."
            );
        }

        const std::vector<double> constantField(mesh.vertexCount(), 7.0);
        const std::vector<double> constantGradient = gradient.multiply(constantField);

        for (const double value : constantGradient) {
            failIfNotNear(value, 0.0, "Constant scalar field should have zero graph gradient.");
        }

        const cppTDGL::SparseMatrix divergence =
            cppTDGL::MeshOperators::graphDivergenceOperator(edgeMesh, mesh.vertexCount());

        failIfFalse(divergence.isValid(), "Graph divergence matrix should be valid.");
        failIfFalse(divergence.rowCount() == mesh.vertexCount(), "Divergence row count should equal vertex count.");
        failIfFalse(divergence.columnCount() == edgeMesh.edgeCount(), "Divergence column count should equal edge count.");
        failIfFalse(divergence.entries().size() == 2 * edgeMesh.edgeCount(), "Each edge should contribute two divergence entries.");

        const cppTDGL::SparseMatrix laplacian =
            cppTDGL::MeshOperators::graphLaplacianOperator(edgeMesh, mesh.vertexCount());

        failIfFalse(laplacian.isValid(), "Graph Laplacian matrix should be valid.");
        failIfFalse(laplacian.rowCount() == mesh.vertexCount(), "Laplacian row count should equal vertex count.");
        failIfFalse(laplacian.columnCount() == mesh.vertexCount(), "Laplian column count should equal vertex count.");

        const std::vector<double> laplacianOfConstant = laplacian.multiply(constantField);

        for (const double value : laplacianOfConstant) {
            failIfNotNear(value, 0.0, "Graph Laplacian of a constant field should be zero.");
        }

        failIfNotNear(laplacian.valueAt(0, 0), 3.0, "Vertex 0 graph degree should be 3.");
        failIfNotNear(laplacian.valueAt(1, 1), 2.0, "Vertex 1 graph degree should be 2.");
        failIfNotNear(laplacian.valueAt(2, 2), 3.0, "Vertex 2 graph degree should be 3.");
        failIfNotNear(laplacian.valueAt(3, 3), 2.0, "Vertex 3 graph degree should be 2.");

        std::cout << "Sparse mesh operators smoke test passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Sparse mesh operators smoke test exception: " << error.what() << '\n';
        return 1;
    }
}
