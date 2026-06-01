#pragma once

#include <string>

namespace cppTDGL {

class SimulationRunner {
public:
    SimulationRunner() = default;
    virtual ~SimulationRunner() = default;

    [[nodiscard]] std::string className() const;
};

} // namespace cppTDGL
