#pragma once

#include <string>

namespace cppTDGL {

class Polygon2D {
public:
    Polygon2D() = default;
    virtual ~Polygon2D() = default;

    [[nodiscard]] std::string className() const;
};

} // namespace cppTDGL
