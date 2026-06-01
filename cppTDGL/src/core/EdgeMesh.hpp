#pragma once

#include <string>

namespace cppTDGL {

class EdgeMesh {
public:
    EdgeMesh() = default;
    virtual ~EdgeMesh() = default;

    [[nodiscard]] std::string className() const;
};

} // namespace cppTDGL
