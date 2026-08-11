#include "EdgeMesh.hpp"
#include "Layer.hpp"
#include "Mesh.hpp"
#include "SolverState.hpp"
#include "TDGLPhysics.hpp"

#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

constexpr double kPi = 3.141592653589793238462643383279502884;
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

std::size_t edgeIndexFor(const cppTDGL::EdgeMesh& edgeMesh, std::size_t a, std::size_t b) {
    for (std::size_t i = 0; i < edgeMesh.edgeCount(); ++i) {
        const cppTDGL::MeshEdge& edge = edgeMesh.edges()[i];
        if (edge.a == a && edge.b == b) {
            return i;
        }
    }

    throw std::runtime_error("Expected edge was not found.");
}

cppTDGL::SolverFrame makePhaseFrame(
    const cppTDGL::Mesh& mesh,
    const cppTDGL::EdgeMesh& edgeMesh
) {
    cppTDGL::SolverFrame frame = cppTDGL::SolverFrame::zerosFor(mesh, edgeMesh, 0.0);

    frame.orderParameter.real = {1.0, 0.0, -1.0, 0.0};
    frame.orderParameter.imaginary = {0.0, 1.0, 0.0, -1.0};

    return frame;
}

} // namespace

int main() {
    try {
        const cppTDGL::Layer layer(
            0.5,
            2.0,
            0.1,
            3.0,
            4.0,
            "um"
        );

        const cppTDGL::TDGLMaterialScales scales = cppTDGL::TDGLPhysics::materialScalesFromLayer(layer);

        failIfFalse(scales.isValid(), "Material scales derived from a valid Layer should be valid.");
        failIfNotNear(scales.coherenceLength, 0.5, "Coherence length should round trip from Layer.");
        failIfNotNear(scales.penetrationDepth, 2.0, "Penetration depth should round trip from Layer.");
        failIfNotNear(scales.kappa, 4.0, "Kappa should equal lambda / xi.");
        failIfNotNear(scales.normalizedThickness, 0.2, "Normalized thickness should equal d / xi.");
        failIfNotNear(scales.thinFilmScreeningScale, 0.025, "Thin-film screening scale should equal d / lambda^2.");
        failIfFalse(scales.units == "um", "Material units should round trip from Layer.");

        cppTDGL::Layer invalidLayer = layer;
        invalidLayer.setCoherenceLength(0.0);

        bool invalidLayerThrew = false;
        try {
            static_cast<void>(cppTDGL::TDGLPhysics::materialScalesFromLayer(invalidLayer));
        } catch (const std::invalid_argument&) {
            invalidLayerThrew = true;
        }

        failIfFalse(invalidLayerThrew, "Invalid Layer should not produce TDGL material scales.");

        const cppTDGL::Mesh mesh = makeSquareMesh();
        const cppTDGL::EdgeMesh edgeMesh = cppTDGL::EdgeMesh::fromMesh(mesh);
        cppTDGL::SolverFrame frame = makePhaseFrame(mesh, edgeMesh);

        const std::vector<double> magnitudes = cppTDGL::TDGLPhysics::orderParameterMagnitude(frame.orderParameter);
        const std::vector<double> magnitudesSquared = cppTDGL::TDGLPhysics::orderParameterMagnitudeSquared(frame.orderParameter);
        const std::vector<double> phases = cppTDGL::TDGLPhysics::orderParameterPhase(frame.orderParameter);

        failIfFalse(magnitudes.size() == mesh.vertexCount(), "Order parameter magnitude should be vertex-sized.");
        failIfFalse(magnitudesSquared.size() == mesh.vertexCount(), "Order parameter magnitude squared should be vertex-sized.");
        failIfFalse(phases.size() == mesh.vertexCount(), "Order parameter phase should be vertex-sized.");

        for (std::size_t i = 0; i < mesh.vertexCount(); ++i) {
            failIfNotNear(magnitudes[i], 1.0, "Unit-circle order parameter magnitude should be one.");
            failIfNotNear(magnitudesSquared[i], 1.0, "Unit-circle order parameter magnitude squared should be one.");
        }

        failIfNotNear(phases[0], 0.0, "Vertex 0 phase should be zero.");
        failIfNotNear(phases[1], kPi / 2.0, "Vertex 1 phase should be pi/2.");
        failIfNotNear(phases[2], kPi, "Vertex 2 phase should be pi.");
        failIfNotNear(phases[3], -kPi / 2.0, "Vertex 3 phase should be -pi/2.");

        const cppTDGL::EdgeScalarField phaseDifference =
            cppTDGL::TDGLPhysics::gaugeInvariantPhaseDifference(mesh, edgeMesh, frame);

        const std::size_t edge01 = edgeIndexFor(edgeMesh, 0, 1);
        const std::size_t edge02 = edgeIndexFor(edgeMesh, 0, 2);
        const std::size_t edge03 = edgeIndexFor(edgeMesh, 0, 3);

        failIfNotNear(phaseDifference.values[edge01], kPi / 2.0, "Edge 0-1 phase difference should be pi/2.");
        failIfNotNear(phaseDifference.values[edge02], kPi, "Edge 0-2 phase difference should wrap to pi.");
        failIfNotNear(phaseDifference.values[edge03], -kPi / 2.0, "Edge 0-3 phase difference should be -pi/2.");

        cppTDGL::SolverFrame gaugedFrame = frame;
        gaugedFrame.vectorPotential.values[edge01] = kPi / 2.0;

        const cppTDGL::EdgeScalarField gaugedPhaseDifference =
            cppTDGL::TDGLPhysics::gaugeInvariantPhaseDifference(mesh, edgeMesh, gaugedFrame);

        failIfNotNear(gaugedPhaseDifference.values[edge01], 0.0, "Vector potential should shift edge phase difference.");

        const cppTDGL::EdgeScalarField current =
            cppTDGL::TDGLPhysics::superconductingCurrentProxy(mesh, edgeMesh, frame);

        failIfFalse(current.size() == edgeMesh.edgeCount(), "Superconducting current proxy should be edge-sized.");
        failIfNotNear(current.values[edge01], 1.0, "Edge 0-1 current proxy should be one.");
        failIfNotNear(current.values[edge02], 0.0, "Diagonal edge current proxy should be near zero for sin(pi).");
        failIfNotNear(current.values[edge03], -1.0, "Edge 0-3 current proxy should be negative one.");

        const cppTDGL::VertexScalarField zeroResidual =
            cppTDGL::TDGLPhysics::chargeContinuityResidualProxy(mesh, edgeMesh, frame);

        failIfFalse(zeroResidual.size() == mesh.vertexCount(), "Charge residual proxy should be vertex-sized.");
        for (const double value : zeroResidual.values) {
            failIfNotNear(value, 0.0, "Zero current density should have zero residual proxy.");
        }

        frame.currentDensity = current;

        const cppTDGL::TDGLPhysicsDiagnostics diagnostics =
            cppTDGL::TDGLPhysics::diagnosticsForFrame(mesh, edgeMesh, frame);

        failIfNotNear(diagnostics.meanOrderParameterMagnitude, 1.0, "Mean order parameter magnitude should be one.");
        failIfNotNear(diagnostics.maxOrderParameterMagnitude, 1.0, "Max order parameter magnitude should be one.");
        failIfFalse(diagnostics.maxCurrentDensityMagnitude > 0.99, "Max current density proxy should be near one.");
        failIfFalse(diagnostics.maxChargeContinuityResidual > 0.0, "Nonzero edge currents should produce a residual proxy.");

        cppTDGL::VertexComplexField invalidOrderParameter = frame.orderParameter;
        invalidOrderParameter.imaginary.pop_back();

        bool invalidOrderParameterThrew = false;
        try {
            static_cast<void>(cppTDGL::TDGLPhysics::orderParameterMagnitude(invalidOrderParameter));
        } catch (const std::invalid_argument&) {
            invalidOrderParameterThrew = true;
        }

        failIfFalse(invalidOrderParameterThrew, "Mismatched order-parameter arrays should throw.");

        cppTDGL::SolverFrame invalidFrame = frame;
        invalidFrame.scalarPotential.values.pop_back();

        bool invalidFrameThrew = false;
        try {
            static_cast<void>(cppTDGL::TDGLPhysics::diagnosticsForFrame(mesh, edgeMesh, invalidFrame));
        } catch (const std::invalid_argument&) {
            invalidFrameThrew = true;
        }

        failIfFalse(invalidFrameThrew, "Frame-size incompatibility should throw during physics diagnostics.");

        std::cout << "TDGL physics smoke test passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "TDGL physics smoke test exception: " << error.what() << '\n';
        return 1;
    }
}
