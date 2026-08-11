#include "CurrentDensityPlot.hpp"

#include "EdgeMesh.hpp"

#include <cmath>
#include <string>
#include <vector>

namespace cppTDGL {

bool CurrentDensityPlotData::hasVectors() const noexcept {
    return !vectors.empty();
}

bool CurrentDensityPlotData::isValid() const noexcept {
    return diagnostics.empty() && hasVectors() && magnitudeRange.hasValues;
}

std::string CurrentDensityPlot::className() const {
    return "CurrentDensityPlot";
}

CurrentDensityPlotData CurrentDensityPlot::fromMesh(
    const Mesh& mesh,
    const EdgeScalarField& currentDensity
) {
    CurrentDensityPlotData data;

    const std::vector<std::string> meshErrors = mesh.validationErrors();
    data.diagnostics.insert(data.diagnostics.end(), meshErrors.begin(), meshErrors.end());

    if (!mesh.isValid()) {
        return data;
    }

    const EdgeMesh edgeMesh = EdgeMesh::fromMesh(mesh);
    const std::vector<std::string> fieldErrors = currentDensity.validationErrors(edgeMesh, "CurrentDensityPlot current density");
    data.diagnostics.insert(data.diagnostics.end(), fieldErrors.begin(), fieldErrors.end());

    if (!fieldErrors.empty()) {
        return data;
    }

    std::vector<double> magnitudes;
    magnitudes.reserve(edgeMesh.edgeCount());
    data.vectors.reserve(edgeMesh.edgeCount());

    for (std::size_t i = 0; i < edgeMesh.edgeCount(); ++i) {
        const MeshEdge& edge = edgeMesh.edges()[i];
        const double value = currentDensity.values[i];
        const double magnitude = std::abs(value);

        magnitudes.push_back(magnitude);
        data.vectors.push_back({
            i,
            edge.a,
            edge.b,
            mesh.vertices()[edge.a],
            mesh.vertices()[edge.b],
            edge.center,
            value,
            magnitude,
            edge.isBoundary,
        });
    }

    data.magnitudeRange = FieldPlot::valueRange(magnitudes);
    return data;
}

} // namespace cppTDGL
