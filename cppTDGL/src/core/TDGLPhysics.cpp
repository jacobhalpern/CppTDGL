#include "TDGLPhysics.hpp"

#include "MeshOperators.hpp"
#include "SparseMatrix.hpp"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

namespace cppTDGL {
namespace {

constexpr double kPi = 3.141592653589793238462643383279502884;
constexpr double kEdgeLengthTolerance = 1.0e-15;

void appendErrors(
    std::vector<std::string>& destination,
    const std::string& prefix,
    const std::vector<std::string>& source
) {
    for (const std::string& error : source) {
        destination.push_back(prefix + error);
    }
}

[[nodiscard]] bool isPositiveFinite(double value) noexcept {
    return std::isfinite(value) && value > 0.0;
}

[[nodiscard]] double wrapPhaseToPi(double value) noexcept {
    while (value > kPi) {
        value -= 2.0 * kPi;
    }

    while (value <= -kPi) {
        value += 2.0 * kPi;
    }

    return value;
}

[[nodiscard]] double maxAbs(const std::vector<double>& values) noexcept {
    double maximum = 0.0;

    for (const double value : values) {
        maximum = std::max(maximum, std::abs(value));
    }

    return maximum;
}

[[nodiscard]] double meanAbs(const std::vector<double>& values) noexcept {
    if (values.empty()) {
        return 0.0;
    }

    double sum = 0.0;

    for (const double value : values) {
        sum += std::abs(value);
    }

    return sum / static_cast<double>(values.size());
}

} // namespace

bool TDGLMaterialScales::isValid() const {
    return validationErrors().empty();
}

std::vector<std::string> TDGLMaterialScales::validationErrors() const {
    std::vector<std::string> errors;

    if (!isPositiveFinite(coherenceLength)) {
        errors.emplace_back("TDGL material coherence length must be positive and finite.");
    }

    if (!isPositiveFinite(penetrationDepth)) {
        errors.emplace_back("TDGL material penetration depth must be positive and finite.");
    }

    if (!isPositiveFinite(thickness)) {
        errors.emplace_back("TDGL material thickness must be positive and finite.");
    }

    if (!std::isfinite(conductivity) || conductivity < 0.0) {
        errors.emplace_back("TDGL material conductivity must be non-negative and finite.");
    }

    if (!isPositiveFinite(gamma)) {
        errors.emplace_back("TDGL material gamma must be positive and finite.");
    }

    if (!isPositiveFinite(kappa)) {
        errors.emplace_back("TDGL material kappa must be positive and finite.");
    }

    if (!isPositiveFinite(normalizedThickness)) {
        errors.emplace_back("TDGL material normalized thickness must be positive and finite.");
    }

    if (!isPositiveFinite(thinFilmScreeningScale)) {
        errors.emplace_back("TDGL material thin-film screening scale must be positive and finite.");
    }

    if (units.empty()) {
        errors.emplace_back("TDGL material units string must not be empty.");
    }

    return errors;
}

TDGLMaterialScales TDGLPhysics::materialScalesFromLayer(const Layer& layer) {
    const std::vector<std::string> layerErrors = layer.validationErrors();

    if (!layerErrors.empty()) {
        throw std::invalid_argument(layerErrors.front());
    }

    TDGLMaterialScales scales;
    scales.coherenceLength = layer.coherenceLength();
    scales.penetrationDepth = layer.londonLambda();
    scales.thickness = layer.thickness();
    scales.conductivity = layer.conductivity();
    scales.gamma = layer.gamma();
    scales.kappa = layer.londonLambda() / layer.coherenceLength();
    scales.normalizedThickness = layer.thickness() / layer.coherenceLength();
    scales.thinFilmScreeningScale = layer.thickness() / (layer.londonLambda() * layer.londonLambda());
    scales.units = layer.units();

    const std::vector<std::string> scaleErrors = scales.validationErrors();
    if (!scaleErrors.empty()) {
        throw std::invalid_argument(scaleErrors.front());
    }

    return scales;
}

std::vector<double> TDGLPhysics::orderParameterMagnitude(
    const VertexComplexField& orderParameter
) {
    validateOrderParameterOrThrow(orderParameter);

    std::vector<double> magnitudes;
    magnitudes.reserve(orderParameter.real.size());

    for (std::size_t i = 0; i < orderParameter.real.size(); ++i) {
        magnitudes.push_back(std::hypot(orderParameter.real[i], orderParameter.imaginary[i]));
    }

    return magnitudes;
}

std::vector<double> TDGLPhysics::orderParameterMagnitudeSquared(
    const VertexComplexField& orderParameter
) {
    validateOrderParameterOrThrow(orderParameter);

    std::vector<double> magnitudesSquared;
    magnitudesSquared.reserve(orderParameter.real.size());

    for (std::size_t i = 0; i < orderParameter.real.size(); ++i) {
        magnitudesSquared.push_back(
            orderParameter.real[i] * orderParameter.real[i] +
            orderParameter.imaginary[i] * orderParameter.imaginary[i]
        );
    }

    return magnitudesSquared;
}

std::vector<double> TDGLPhysics::orderParameterPhase(
    const VertexComplexField& orderParameter
) {
    validateOrderParameterOrThrow(orderParameter);

    std::vector<double> phases;
    phases.reserve(orderParameter.real.size());

    for (std::size_t i = 0; i < orderParameter.real.size(); ++i) {
        phases.push_back(std::atan2(orderParameter.imaginary[i], orderParameter.real[i]));
    }

    return phases;
}

EdgeScalarField TDGLPhysics::gaugeInvariantPhaseDifference(
    const Mesh& mesh,
    const EdgeMesh& edgeMesh,
    const SolverFrame& frame
) {
    validateFrameOrThrow(mesh, edgeMesh, frame);

    const std::vector<double> phases = orderParameterPhase(frame.orderParameter);
    EdgeScalarField differences = EdgeScalarField::zeros(edgeMesh.edgeCount());

    for (std::size_t edgeIndex = 0; edgeIndex < edgeMesh.edgeCount(); ++edgeIndex) {
        const MeshEdge& edge = edgeMesh.edges()[edgeIndex];
        const double phaseDifference = phases[edge.b] - phases[edge.a];
        const double gaugeLineIntegral = frame.vectorPotential.values[edgeIndex];

        differences.values[edgeIndex] = wrapPhaseToPi(phaseDifference - gaugeLineIntegral);
    }

    return differences;
}

EdgeScalarField TDGLPhysics::superconductingCurrentProxy(
    const Mesh& mesh,
    const EdgeMesh& edgeMesh,
    const SolverFrame& frame
) {
    validateFrameOrThrow(mesh, edgeMesh, frame);

    const std::vector<double> magnitudes = orderParameterMagnitude(frame.orderParameter);
    const EdgeScalarField phaseDifferences = gaugeInvariantPhaseDifference(mesh, edgeMesh, frame);
    EdgeScalarField current = EdgeScalarField::zeros(edgeMesh.edgeCount());

    for (std::size_t edgeIndex = 0; edgeIndex < edgeMesh.edgeCount(); ++edgeIndex) {
        const MeshEdge& edge = edgeMesh.edges()[edgeIndex];

        if (edge.length <= kEdgeLengthTolerance) {
            throw std::invalid_argument("Cannot compute current proxy for an edge with zero length.");
        }

        const double edgeAmplitude = magnitudes[edge.a] * magnitudes[edge.b];
        current.values[edgeIndex] =
            edgeAmplitude * std::sin(phaseDifferences.values[edgeIndex]) / edge.length;
    }

    return current;
}

VertexScalarField TDGLPhysics::chargeContinuityResidualProxy(
    const Mesh& mesh,
    const EdgeMesh& edgeMesh,
    const SolverFrame& frame
) {
    validateFrameOrThrow(mesh, edgeMesh, frame);

    const std::vector<double> controlVolumes = MeshOperators::vertexControlVolumes(mesh);
    const SparseMatrix divergence = MeshOperators::finiteVolumeDivergenceOperator(
        edgeMesh,
        controlVolumes,
        mesh.vertexCount()
    );

    return VertexScalarField{divergence.multiply(frame.currentDensity.values)};
}

TDGLPhysicsDiagnostics TDGLPhysics::diagnosticsForFrame(
    const Mesh& mesh,
    const EdgeMesh& edgeMesh,
    const SolverFrame& frame
) {
    validateFrameOrThrow(mesh, edgeMesh, frame);

    const std::vector<double> magnitudes = orderParameterMagnitude(frame.orderParameter);
    const VertexScalarField continuityResidual = chargeContinuityResidualProxy(mesh, edgeMesh, frame);

    TDGLPhysicsDiagnostics diagnostics;
    diagnostics.meanOrderParameterMagnitude = meanAbs(magnitudes);
    diagnostics.maxOrderParameterMagnitude = maxAbs(magnitudes);
    diagnostics.meanCurrentDensityMagnitude = meanAbs(frame.currentDensity.values);
    diagnostics.maxCurrentDensityMagnitude = maxAbs(frame.currentDensity.values);
    diagnostics.maxChargeContinuityResidual = maxAbs(continuityResidual.values);

    return diagnostics;
}

void TDGLPhysics::validateOrderParameterOrThrow(const VertexComplexField& orderParameter) {
    if (orderParameter.real.size() != orderParameter.imaginary.size()) {
        throw std::invalid_argument("Order parameter real and imaginary arrays must have the same size.");
    }

    for (std::size_t i = 0; i < orderParameter.real.size(); ++i) {
        if (!std::isfinite(orderParameter.real[i])) {
            throw std::invalid_argument("Order parameter real component " + std::to_string(i) + " must be finite.");
        }

        if (!std::isfinite(orderParameter.imaginary[i])) {
            throw std::invalid_argument("Order parameter imaginary component " + std::to_string(i) + " must be finite.");
        }
    }
}

void TDGLPhysics::validateFrameOrThrow(
    const Mesh& mesh,
    const EdgeMesh& edgeMesh,
    const SolverFrame& frame
) {
    std::vector<std::string> errors;

    appendErrors(errors, "Mesh: ", mesh.validationErrors());
    appendErrors(errors, "EdgeMesh: ", edgeMesh.validationErrors());

    if (mesh.isValid() && edgeMesh.isValid()) {
        appendErrors(errors, "Frame: ", frame.validationErrors(mesh, edgeMesh));
    }

    if (!errors.empty()) {
        throw std::invalid_argument(errors.front());
    }
}

} // namespace cppTDGL
