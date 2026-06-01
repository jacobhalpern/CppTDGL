#pragma once

#include <string>

namespace cppTDGL {

class Mesh {
public:
    Mesh() = default;
    virtual ~Mesh() = default;

    [[nodiscard]] std::string className() const;
};

} // namespace cppTDGL
