#include "TimeSeriesPlot.hpp"

#include <cmath>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

namespace cppTDGL {
namespace {

[[nodiscard]] double meanOrderParameterMagnitude(const SolverFrame& frame) {
    if (frame.orderParameter.real.empty()) {
        return 0.0;
    }

    double total = 0.0;
    for (std::size_t i = 0; i < frame.orderParameter.real.size(); ++i) {
        total += std::hypot(frame.orderParameter.real[i], frame.orderParameter.imaginary[i]);
    }

    return total / static_cast<double>(frame.orderParameter.real.size());
}

[[nodiscard]] double meanCurrentDensityMagnitude(const SolverFrame& frame) {
    if (frame.currentDensity.values.empty()) {
        return 0.0;
    }

    double total = 0.0;
    for (const double value : frame.currentDensity.values) {
        total += std::abs(value);
    }

    return total / static_cast<double>(frame.currentDensity.values.size());
}

[[nodiscard]] std::vector<double> frameTimeValues(const Solution& solution) {
    std::vector<double> values;
    values.reserve(solution.frameCount());

    for (const SolverFrame& frame : solution.frames()) {
        values.push_back(frame.time);
    }

    return values;
}

} // namespace

bool TimeSeriesData::hasPoints() const noexcept {
    return !points.empty();
}

bool TimeSeriesData::isValid() const noexcept {
    return diagnostics.empty() && hasPoints() && yRange.hasValues;
}

std::string TimeSeriesPlot::className() const {
    return "TimeSeriesPlot";
}

TimeSeriesData TimeSeriesPlot::fromValues(
    std::string seriesName,
    std::string xLabel,
    std::string yLabel,
    const std::vector<double>& xValues,
    const std::vector<double>& yValues
) {
    TimeSeriesData data;
    data.seriesName = std::move(seriesName);
    data.xLabel = std::move(xLabel);
    data.yLabel = std::move(yLabel);

    if (xValues.size() != yValues.size()) {
        data.diagnostics.push_back(
            "Time-series x/y arrays must have the same size. Expected " +
            std::to_string(xValues.size()) + ", got " +
            std::to_string(yValues.size()) + "."
        );
        return data;
    }

    data.points.reserve(xValues.size());
    for (std::size_t i = 0; i < xValues.size(); ++i) {
        data.points.push_back({xValues[i], yValues[i]});
    }

    data.yRange = FieldPlot::valueRange(yValues);
    return data;
}

TimeSeriesData TimeSeriesPlot::frameTimes(const Solution& solution) {
    std::vector<double> xValues;
    std::vector<double> yValues;
    xValues.reserve(solution.frameCount());
    yValues.reserve(solution.frameCount());

    for (std::size_t i = 0; i < solution.frameCount(); ++i) {
        xValues.push_back(static_cast<double>(i));
        yValues.push_back(solution.frame(i).time);
    }

    TimeSeriesData data = fromValues("frame times", "frame", "time", xValues, yValues);

    if (!solution.hasFrames()) {
        data.diagnostics.emplace_back("Solution has no frames.");
    }

    return data;
}

TimeSeriesData TimeSeriesPlot::orderParameterMeanMagnitude(const Solution& solution) {
    const std::vector<double> xValues = frameTimeValues(solution);
    std::vector<double> yValues;
    yValues.reserve(solution.frameCount());

    for (const SolverFrame& frame : solution.frames()) {
        yValues.push_back(meanOrderParameterMagnitude(frame));
    }

    TimeSeriesData data = fromValues("mean order parameter magnitude", "time", "mean |psi|", xValues, yValues);

    if (!solution.hasFrames()) {
        data.diagnostics.emplace_back("Solution has no frames.");
    }

    return data;
}

TimeSeriesData TimeSeriesPlot::currentDensityMeanMagnitude(const Solution& solution) {
    const std::vector<double> xValues = frameTimeValues(solution);
    std::vector<double> yValues;
    yValues.reserve(solution.frameCount());

    for (const SolverFrame& frame : solution.frames()) {
        yValues.push_back(meanCurrentDensityMagnitude(frame));
    }

    TimeSeriesData data = fromValues("mean current-density magnitude", "time", "mean |J|", xValues, yValues);

    if (!solution.hasFrames()) {
        data.diagnostics.emplace_back("Solution has no frames.");
    }

    return data;
}

} // namespace cppTDGL
