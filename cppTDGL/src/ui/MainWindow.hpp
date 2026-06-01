#pragma once

#include <string>

namespace cppTDGL {

class MainWindow {
public:
    MainWindow() = default;
    virtual ~MainWindow() = default;

    [[nodiscard]] std::string className() const;
};

} // namespace cppTDGL
