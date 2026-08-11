#pragma once

#include "FieldPlot.hpp"
#include "Solution.hpp"

#include <string>
#include <vector>

namespace cppTDGL {

struct TimeSeriesPoint {
    double x = 0.0;
    double y = 0.0;
};

struct TimeSeriesData {
    std::string seriesName = "series";
    std::string xLabel = "time";
    std::string yLabel = "value";
    std::vector<TimeSeriesPoint> points;
    PlotValueRange yRange{};
    std::vector<std::string> diagnostics;

    [[nodiscard]] bool hasPoints() const noexcept;
    [[nodiscard]] bool isValid() const noexcept;
};

class TimeSeriesPlot {
public:
    TimeSeriesPlot() = default;
    virtual ~TimeSeriesPlot() = default;

    [[nodiscard]] std::string className() const;

    [[nodiscard]] static TimeSeriesData fromValues(
        std::string seriesName,
        std::string xLabel,
        std::string yLabel,
        const std::vector<double>& xValues,
        const std::vector<double>& yValues
    );

    [[nodiscard]] static TimeSeriesData frameTimes(const Solution& solution);
    [[nodiscard]] static TimeSeriesData orderParameterMeanMagnitude(const Solution& solution);
    [[nodiscard]] static TimeSeriesData currentDensityMeanMagnitude(const Solution& solution);
};

} // namespace cppTDGL
