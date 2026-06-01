#pragma once

#include <string>

namespace cppTDGL {

class FieldPlot {
public:
    FieldPlot() = default;
    virtual ~FieldPlot() = default;

    [[nodiscard]] std::string className() const;
};

} // namespace cppTDGL
