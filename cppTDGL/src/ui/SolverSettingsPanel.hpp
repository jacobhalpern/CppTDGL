#pragma once

#include <string>

namespace cppTDGL {

class SolverSettingsPanel {
public:
    SolverSettingsPanel() = default;
    virtual ~SolverSettingsPanel() = default;

    [[nodiscard]] std::string className() const;
};

} // namespace cppTDGL
