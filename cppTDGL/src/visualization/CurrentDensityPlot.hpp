#pragma once

#include "FieldPlot.hpp"
#include "Mesh.hpp"
#include "SolverState.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace cppTDGL {

struct CurrentDensityVector {
    std::size_t edgeIndex = 0;
    std::size_t a = 0;
    std::size_t b = 0;
    Point2D start{};
    Point2D end{};
    Point2D center{};
    double value = 0.0;
    double magnitude = 0.0;
    bool isBoundary = false;
};

struct CurrentDensityPlotData {
    std::vector<CurrentDensityVector> vectors;
    PlotValueRange magnitudeRange{};
    std::vector<std::string> diagnostics;

    [[nodiscard]] bool hasVectors() const noexcept;
    [[nodiscard]] bool isValid() const noexcept;
};

class CurrentDensityPlot {
public:
    CurrentDensityPlot() = default;
    virtual ~CurrentDensityPlot() = default;

    [[nodiscard]] std::string className() const;

    [[nodiscard]] static CurrentDensityPlotData fromMesh(
        const Mesh& mesh,
        const EdgeScalarField& currentDensity
    );
};

} // namespace cppTDGL
