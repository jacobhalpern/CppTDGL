#pragma once

#include <string>

namespace cppTDGL {

class SimulationJob {
public:
    SimulationJob() = default;
    virtual ~SimulationJob() = default;

    [[nodiscard]] std::string className() const;
};

} // namespace cppTDGL
