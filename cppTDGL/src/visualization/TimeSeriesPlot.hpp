#pragma once

#include <string>

namespace cppTDGL {

class TimeSeriesPlot {
public:
    TimeSeriesPlot() = default;
    virtual ~TimeSeriesPlot() = default;

    [[nodiscard]] std::string className() const;
};

} // namespace cppTDGL
