#pragma once

#include "Mesh.hpp"
#include "SolverState.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace cppTDGL {

struct PlotValueRange {
    double minimum = 0.0;
    double maximum = 0.0;
    bool hasValues = false;

    [[nodiscard]] double span() const noexcept;
};

struct FieldPlotSample {
    std::size_t vertexIndex = 0;
    Point2D position{};
    double value = 0.0;
};

struct FieldPlotData {
    std::string fieldName = "field";
    std::vector<FieldPlotSample> samples;
    PlotValueRange range{};
    BoundingBox boundingBox{};
    std::vector<std::string> diagnostics;

    [[nodiscard]] bool hasSamples() const noexcept;
    [[nodiscard]] bool isValid() const noexcept;
};

class FieldPlot {
public:
    FieldPlot() = default;
    virtual ~FieldPlot() = default;

    [[nodiscard]] std::string className() const;

    [[nodiscard]] static FieldPlotData fromVertexScalarField(
        const Mesh& mesh,
        const VertexScalarField& field,
        std::string fieldName = "scalar field"
    );

    [[nodiscard]] static FieldPlotData orderParameterMagnitude(
        const Mesh& mesh,
        const VertexComplexField& field,
        std::string fieldName = "order parameter magnitude"
    );

    [[nodiscard]] static PlotValueRange valueRange(const std::vector<double>& values);
};

} // namespace cppTDGL
