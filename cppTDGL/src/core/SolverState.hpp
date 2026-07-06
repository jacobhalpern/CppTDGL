#pragma once

#include "EdgeMesh.hpp"
#include "Mesh.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace cppTDGL {

struct VertexScalarField {
    std::vector<double> values;

    [[nodiscard]] static VertexScalarField filled(std::size_t vertexCount, double value);
    [[nodiscard]] static VertexScalarField zeros(std::size_t vertexCount);

    [[nodiscard]] std::size_t size() const noexcept;
    [[nodiscard]] bool isCompatibleWith(const Mesh& mesh) const;
    [[nodiscard]] std::vector<std::string> validationErrors(
        const Mesh& mesh,
        const std::string& label = "VertexScalarField"
    ) const;
};

struct VertexComplexField {
    std::vector<double> real;
    std::vector<double> imaginary;

    [[nodiscard]] static VertexComplexField filled(
        std::size_t vertexCount,
        double realValue,
        double imaginaryValue
    );

    [[nodiscard]] static VertexComplexField zeros(std::size_t vertexCount);

    [[nodiscard]] std::size_t size() const noexcept;
    [[nodiscard]] bool isCompatibleWith(const Mesh& mesh) const;
    [[nodiscard]] std::vector<std::string> validationErrors(
        const Mesh& mesh,
        const std::string& label = "VertexComplexField"
    ) const;
};

struct EdgeScalarField {
    std::vector<double> values;

    [[nodiscard]] static EdgeScalarField filled(std::size_t edgeCount, double value);
    [[nodiscard]] static EdgeScalarField zeros(std::size_t edgeCount);

    [[nodiscard]] std::size_t size() const noexcept;
    [[nodiscard]] bool isCompatibleWith(const EdgeMesh& edgeMesh) const;
    [[nodiscard]] std::vector<std::string> validationErrors(
        const EdgeMesh& edgeMesh,
        const std::string& label = "EdgeScalarField"
    ) const;
};

struct SolverFrame {
    double time = 0.0;
    VertexComplexField orderParameter;
    VertexScalarField scalarPotential;
    EdgeScalarField vectorPotential;
    EdgeScalarField currentDensity;

    [[nodiscard]] static SolverFrame uniformOrderParameterFor(
        const Mesh& mesh,
        const EdgeMesh& edgeMesh,
        double time,
        double orderParameterReal = 1.0,
        double orderParameterImaginary = 0.0
    );

    [[nodiscard]] static SolverFrame zerosFor(
        const Mesh& mesh,
        const EdgeMesh& edgeMesh,
        double time
    );

    [[nodiscard]] bool isCompatibleWith(const Mesh& mesh, const EdgeMesh& edgeMesh) const;
    [[nodiscard]] std::vector<std::string> validationErrors(
        const Mesh& mesh,
        const EdgeMesh& edgeMesh
    ) const;
};

class SolverState {
public:
    explicit SolverState(Mesh mesh);

    [[nodiscard]] static SolverState createUniformInitialState(
        Mesh mesh,
        double initialTime = 0.0,
        double orderParameterReal = 1.0,
        double orderParameterImaginary = 0.0
    );

    [[nodiscard]] const Mesh& mesh() const noexcept;
    [[nodiscard]] EdgeMesh edgeMesh() const;

    [[nodiscard]] const std::vector<SolverFrame>& frames() const noexcept;
    [[nodiscard]] std::size_t frameCount() const noexcept;
    [[nodiscard]] bool hasFrames() const noexcept;

    [[nodiscard]] const SolverFrame& latestFrame() const;

    void appendFrame(SolverFrame frame);
    void clearFrames();

    [[nodiscard]] bool isValid() const;
    [[nodiscard]] std::vector<std::string> validationErrors() const;

private:
    Mesh mesh_;
    std::vector<SolverFrame> frames_;
};

} // namespace cppTDGL
