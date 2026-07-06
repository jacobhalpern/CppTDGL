#include "EdgeMesh.hpp"
#include "Mesh.hpp"
#include "SolverState.hpp"

#include <cmath>
#include <iostream>
#include <limits>
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

        failIfFalse(mesh.vertexCount() == 4, "Square mesh should have 4 vertices.");
        failIfFalse(edgeMesh.edgeCount() == 5, "Square mesh should have 5 unique edges.");

        cppTDGL::VertexScalarField scalarField = cppTDGL::VertexScalarField::zeros(mesh.vertexCount());
        failIfFalse(scalarField.size() == 4, "Vertex scalar field should have one value per vertex.");
        failIfFalse(scalarField.isCompatibleWith(mesh), "Vertex scalar field should be mesh-compatible.");

        cppTDGL::VertexComplexField complexField =
            cppTDGL::VertexComplexField::filled(mesh.vertexCount(), 1.0, 0.0);

        failIfFalse(complexField.size() == 4, "Vertex complex field should have one value per vertex.");
        failIfFalse(complexField.isCompatibleWith(mesh), "Vertex complex field should be mesh-compatible.");
        failIfNotNear(complexField.real[0], 1.0, "Uniform complex field real component should be preserved.");
        failIfNotNear(complexField.imaginary[0], 0.0, "Uniform complex field imaginary component should be preserved.");

        cppTDGL::EdgeScalarField edgeField = cppTDGL::EdgeScalarField::zeros(edgeMesh.edgeCount());
        failIfFalse(edgeField.size() == 5, "Edge scalar field should have one value per edge.");
        failIfFalse(edgeField.isCompatibleWith(edgeMesh), "Edge scalar field should be edge-compatible.");

        cppTDGL::SolverFrame frame =
            cppTDGL::SolverFrame::uniformOrderParameterFor(mesh, edgeMesh, 0.25, 0.75, 0.125);

        failIfFalse(frame.isCompatibleWith(mesh, edgeMesh), "Uniform solver frame should be compatible.");
        failIfNotNear(frame.time, 0.25, "Solver frame time should be preserved.");
        failIfNotNear(frame.orderParameter.real[0], 0.75, "Order parameter real component should be preserved.");
        failIfNotNear(frame.orderParameter.imaginary[0], 0.125, "Order parameter imaginary component should be preserved.");
        failIfFalse(frame.scalarPotential.size() == mesh.vertexCount(), "Scalar potential should be vertex-sized.");
        failIfFalse(frame.vectorPotential.size() == edgeMesh.edgeCount(), "Vector potential should be edge-sized.");
        failIfFalse(frame.currentDensity.size() == edgeMesh.edgeCount(), "Current density should be edge-sized.");

        cppTDGL::SolverState emptyState(mesh);

        failIfFalse(emptyState.isValid(), "SolverState with valid mesh and no frames should be valid.");
        failIfFalse(!emptyState.hasFrames(), "New SolverState should start with no frames.");
        failIfFalse(emptyState.frameCount() == 0, "New SolverState frame count should be zero.");

        bool latestFrameThrew = false;
        try {
            static_cast<void>(emptyState.latestFrame());
        } catch (const std::logic_error&) {
            latestFrameThrew = true;
        }

        failIfFalse(latestFrameThrew, "latestFrame() should throw when SolverState has no frames.");

        cppTDGL::SolverState state =
            cppTDGL::SolverState::createUniformInitialState(mesh, 0.0, 1.0, 0.0);

        failIfFalse(state.isValid(), "SolverState with uniform initial frame should be valid.");
        failIfFalse(state.hasFrames(), "SolverState should report frames after initial frame creation.");
        failIfFalse(state.frameCount() == 1, "Uniform initial SolverState should have one frame.");
        failIfNotNear(state.latestFrame().time, 0.0, "Initial frame time should be zero.");

        state.appendFrame(cppTDGL::SolverFrame::zerosFor(state.mesh(), state.edgeMesh(), 0.1));

        failIfFalse(state.frameCount() == 2, "Appending a valid frame should increase frame count.");
        failIfNotNear(state.latestFrame().time, 0.1, "Latest frame time should match appended frame.");
        failIfFalse(state.isValid(), "SolverState should remain valid after appending a valid frame.");

        cppTDGL::SolverFrame invalidSizeFrame =
            cppTDGL::SolverFrame::zerosFor(state.mesh(), state.edgeMesh(), 0.2);

        invalidSizeFrame.scalarPotential.values.pop_back();

        bool invalidSizeThrew = false;
        try {
            state.appendFrame(invalidSizeFrame);
        } catch (const std::invalid_argument&) {
            invalidSizeThrew = true;
        }

        failIfFalse(invalidSizeThrew, "Appending a vertex-size-incompatible frame should throw.");

        cppTDGL::SolverFrame nonFiniteFrame =
            cppTDGL::SolverFrame::zerosFor(state.mesh(), state.edgeMesh(), 0.3);

        nonFiniteFrame.time = std::numeric_limits<double>::infinity();

        failIfFalse(
            !nonFiniteFrame.isCompatibleWith(state.mesh(), state.edgeMesh()),
            "Non-finite frame time should fail compatibility validation."
        );

        state.clearFrames();

        failIfFalse(!state.hasFrames(), "clearFrames() should remove all frames.");
        failIfFalse(state.frameCount() == 0, "clearFrames() should set frame count to zero.");
        failIfFalse(state.isValid(), "SolverState should remain valid after clearing frames.");

        std::cout << "Solver state smoke test passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Solver state smoke test exception: " << error.what() << '\n';
        return 1;
    }
}
