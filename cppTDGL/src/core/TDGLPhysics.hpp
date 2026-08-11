#pragma once

#include "EdgeMesh.hpp"
#include "Layer.hpp"
#include "Mesh.hpp"
#include "SolverState.hpp"

#include <string>
#include <vector>

namespace cppTDGL {

struct TDGLMaterialScales {
    double coherenceLength = 1.0;
    double penetrationDepth = 1.0;
    double thickness = 0.1;
    double conductivity = 1.0;
    double gamma = 1.0;
    double kappa = 1.0;
    double normalizedThickness = 0.1;
    double thinFilmScreeningScale = 0.1;
    std::string units = "um";

    [[nodiscard]] bool isValid() const;
    [[nodiscard]] std::vector<std::string> validationErrors() const;
};

struct TDGLPhysicsDiagnostics {
    double meanOrderParameterMagnitude = 0.0;
    double maxOrderParameterMagnitude = 0.0;
    double meanCurrentDensityMagnitude = 0.0;
    double maxCurrentDensityMagnitude = 0.0;
    double maxChargeContinuityResidual = 0.0;
};

class TDGLPhysics {
public:
    [[nodiscard]] static TDGLMaterialScales materialScalesFromLayer(const Layer& layer);

    [[nodiscard]] static std::vector<double> orderParameterMagnitude(
        const VertexComplexField& orderParameter
    );

    [[nodiscard]] static std::vector<double> orderParameterMagnitudeSquared(
        const VertexComplexField& orderParameter
    );

    [[nodiscard]] static std::vector<double> orderParameterPhase(
        const VertexComplexField& orderParameter
    );

    [[nodiscard]] static EdgeScalarField gaugeInvariantPhaseDifference(
        const Mesh& mesh,
        const EdgeMesh& edgeMesh,
        const SolverFrame& frame
    );

    [[nodiscard]] static EdgeScalarField superconductingCurrentProxy(
        const Mesh& mesh,
        const EdgeMesh& edgeMesh,
        const SolverFrame& frame
    );

    [[nodiscard]] static VertexScalarField chargeContinuityResidualProxy(
        const Mesh& mesh,
        const EdgeMesh& edgeMesh,
        const SolverFrame& frame
    );

    [[nodiscard]] static TDGLPhysicsDiagnostics diagnosticsForFrame(
        const Mesh& mesh,
        const EdgeMesh& edgeMesh,
        const SolverFrame& frame
    );

private:
    static void validateOrderParameterOrThrow(const VertexComplexField& orderParameter);
    static void validateFrameOrThrow(const Mesh& mesh, const EdgeMesh& edgeMesh, const SolverFrame& frame);
};

} // namespace cppTDGL
