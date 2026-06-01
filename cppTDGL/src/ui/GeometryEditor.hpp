#pragma once

#include <string>

namespace cppTDGL {

class GeometryEditor {
public:
    GeometryEditor() = default;
    virtual ~GeometryEditor() = default;

    [[nodiscard]] std::string className() const;
};

} // namespace cppTDGL
