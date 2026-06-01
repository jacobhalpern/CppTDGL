#pragma once

#include <string>

namespace cppTDGL {

class Layer {
public:
    Layer() = default;
    virtual ~Layer() = default;

    [[nodiscard]] std::string className() const;
};

} // namespace cppTDGL
