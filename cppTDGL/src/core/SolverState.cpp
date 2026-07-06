#include "SolverState.hpp"

#include <cmath>
#include <stdexcept>
#include <string>
#include <utility>

namespace cppTDGL {
namespace {

void appendErrors(
    std::vector<std::string>& destination,
    const std::vector<std::string>& source
) {
    destination.insert(destination.end(), source.begin(), source.end());
}

void appendFiniteErrors(
    std::vector<std::string>& errors,
    const std::vector<double>& values,
    const std::string& label
) {
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (!std::isfinite(values[i])) {
            errors.push_back(label + " value " + std::to_string(i) + " must be finite.");
        }
    }
}

} // namespace

VertexScalarField VertexScalarField::filled(std::size_t vertexCount, double value) {
    return VertexScalarField{std::vector<double>(vertexCount, value)};
}

VertexScalarField VertexScalarField::zeros(std::size_t vertexCount) {
    return filled(vertexCount, 0.0);
}

std::size_t VertexScalarField::size() const noexcept {
    return values.size();
}

bool VertexScalarField::isCompatibleWith(const Mesh& mesh) const {
    return validationErrors(mesh).empty();
}

std::vector<std::string> VertexScalarField::validationErrors(
    const Mesh& mesh,
    const std::string& label
) const {
    std::vector<std::string> errors;

    if (values.size() != mesh.vertexCount()) {
        errors.push_back(
            label + " size must equal mesh vertex count. Expected " +
            std::to_string(mesh.vertexCount()) + ", got " +
            std::to_string(values.size()) + "."
        );
    }

    appendFiniteErrors(errors, values, label);

    return errors;
}

VertexComplexField VertexComplexField::filled(
    std::size_t vertexCount,
    double realValue,
    double imaginaryValue
) {
    return VertexComplexField{
        std::vector<double>(vertexCount, realValue),
        std::vector<double>(vertexCount, imaginaryValue),
    };
}

VertexComplexField VertexComplexField::zeros(std::size_t vertexCount) {
    return filled(vertexCount, 0.0, 0.0);
}

std::size_t VertexComplexField::size() const noexcept {
    return real.size();
}

bool VertexComplexField::isCompatibleWith(const Mesh& mesh) const {
    return validationErrors(mesh).empty();
}

std::vector<std::string> VertexComplexField::validationErrors(
    const Mesh& mesh,
    const std::string& label
) const {
    std::vector<std::string> errors;

    if (real.size() != mesh.vertexCount()) {
        errors.push_back(
            label + " real size must equal mesh vertex count. Expected " +
            std::to_string(mesh.vertexCount()) + ", got " +
            std::to_string(real.size()) + "."
        );
    }

    if (imaginary.size() != mesh.vertexCount()) {
        errors.push_back(
            label + " imaginary size must equal mesh vertex count. Expected " +
            std::to_string(mesh.vertexCount()) + ", got " +
            std::to_string(imaginary.size()) + "."
        );
    }

    appendFiniteErrors(errors, real, label + " real");
    appendFiniteErrors(errors, imaginary, label + " imaginary");

    return errors;
}

EdgeScalarField EdgeScalarField::filled(std::size_t edgeCount, double value) {
    return EdgeScalarField{std::vector<double>(edgeCount, value)};
}

EdgeScalarField EdgeScalarField::zeros(std::size_t edgeCount) {
    return filled(edgeCount, 0.0);
}

std::size_t EdgeScalarField::size() const noexcept {
    return values.size();
}

bool EdgeScalarField::isCompatibleWith(const EdgeMesh& edgeMesh) const {
    return validationErrors(edgeMesh).empty();
}

std::vector<std::string> EdgeScalarField::validationErrors(
    const EdgeMesh& edgeMesh,
    const std::string& label
) const {
    std::vector<std::string> errors;

    if (values.size() != edgeMesh.edgeCount()) {
        errors.push_back(
            label + " size must equal edge count. Expected " +
            std::to_string(edgeMesh.edgeCount()) + ", got " +
            std::to_string(values.size()) + "."
        );
    }

    appendFiniteErrors(errors, values, label);

    return errors;
}

SolverFrame SolverFrame::uniformOrderParameterFor(
    const Mesh& mesh,
    const EdgeMesh& edgeMesh,
    double time,
    double orderParameterReal,
    double orderParameterImaginary
) {
    return SolverFrame{
        time,
        VertexComplexField::filled(mesh.vertexCount(), orderParameterReal, orderParameterImaginary),
        VertexScalarField::zeros(mesh.vertexCount()),
        EdgeScalarField::zeros(edgeMesh.edgeCount()),
        EdgeScalarField::zeros(edgeMesh.edgeCount()),
    };
}

SolverFrame SolverFrame::zerosFor(
    const Mesh& mesh,
    const EdgeMesh& edgeMesh,
    double time
) {
    return SolverFrame{
        time,
        VertexComplexField::zeros(mesh.vertexCount()),
        VertexScalarField::zeros(mesh.vertexCount()),
        EdgeScalarField::zeros(edgeMesh.edgeCount()),
        EdgeScalarField::zeros(edgeMesh.edgeCount()),
    };
}

bool SolverFrame::isCompatibleWith(const Mesh& mesh, const EdgeMesh& edgeMesh) const {
    return validationErrors(mesh, edgeMesh).empty();
}

std::vector<std::string> SolverFrame::validationErrors(
    const Mesh& mesh,
    const EdgeMesh& edgeMesh
) const {
    std::vector<std::string> errors;

    if (!std::isfinite(time)) {
        errors.emplace_back("SolverFrame time must be finite.");
    }

    appendErrors(errors, orderParameter.validationErrors(mesh, "SolverFrame orderParameter"));
    appendErrors(errors, scalarPotential.validationErrors(mesh, "SolverFrame scalarPotential"));
    appendErrors(errors, vectorPotential.validationErrors(edgeMesh, "SolverFrame vectorPotential"));
    appendErrors(errors, currentDensity.validationErrors(edgeMesh, "SolverFrame currentDensity"));

    return errors;
}

SolverState::SolverState(Mesh mesh)
    : mesh_(std::move(mesh)) {
    if (!mesh_.isValid()) {
        throw std::invalid_argument("SolverState requires a valid Mesh.");
    }
}

SolverState SolverState::createUniformInitialState(
    Mesh mesh,
    double initialTime,
    double orderParameterReal,
    double orderParameterImaginary
) {
    SolverState state(std::move(mesh));
    const EdgeMesh edges = state.edgeMesh();

    state.appendFrame(
        SolverFrame::uniformOrderParameterFor(
            state.mesh(),
            edges,
            initialTime,
            orderParameterReal,
            orderParameterImaginary
        )
    );

    return state;
}

const Mesh& SolverState::mesh() const noexcept {
    return mesh_;
}

EdgeMesh SolverState::edgeMesh() const {
    return EdgeMesh::fromMesh(mesh_);
}

const std::vector<SolverFrame>& SolverState::frames() const noexcept {
    return frames_;
}

std::size_t SolverState::frameCount() const noexcept {
    return frames_.size();
}

bool SolverState::hasFrames() const noexcept {
    return !frames_.empty();
}

const SolverFrame& SolverState::latestFrame() const {
    if (frames_.empty()) {
        throw std::logic_error("SolverState has no frames.");
    }

    return frames_.back();
}

void SolverState::appendFrame(SolverFrame frame) {
    const EdgeMesh edges = edgeMesh();
    const std::vector<std::string> errors = frame.validationErrors(mesh_, edges);

    if (!errors.empty()) {
        throw std::invalid_argument(errors.front());
    }

    frames_.push_back(std::move(frame));
}

void SolverState::clearFrames() {
    frames_.clear();
}

bool SolverState::isValid() const {
    return validationErrors().empty();
}

std::vector<std::string> SolverState::validationErrors() const {
    std::vector<std::string> errors;

    appendErrors(errors, mesh_.validationErrors());

    if (!mesh_.isValid()) {
        return errors;
    }

    const EdgeMesh edges = edgeMesh();

    for (std::size_t i = 0; i < frames_.size(); ++i) {
        const std::vector<std::string> frameErrors = frames_[i].validationErrors(mesh_, edges);
        for (const std::string& error : frameErrors) {
            errors.push_back("Frame " + std::to_string(i) + ": " + error);
        }
    }

    return errors;
}

} // namespace cppTDGL
