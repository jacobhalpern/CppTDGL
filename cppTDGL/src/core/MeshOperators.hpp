#pragma once

#include <string>

namespace cppTDGL {

class MeshOperators {
public:
    MeshOperators() = default;
    virtual ~MeshOperators() = default;

    [[nodiscard]] std::string className() const;
};

} // namespace cppTDGL
