#pragma once

#include <string>

namespace cppTDGL {

class Device {
public:
    Device() = default;
    virtual ~Device() = default;

    [[nodiscard]] std::string className() const;
};

} // namespace cppTDGL
