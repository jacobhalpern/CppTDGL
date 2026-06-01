#pragma once

#include <string>

namespace cppTDGL {

class CurrentDensityPlot {
public:
    CurrentDensityPlot() = default;
    virtual ~CurrentDensityPlot() = default;

    [[nodiscard]] std::string className() const;
};

} // namespace cppTDGL
