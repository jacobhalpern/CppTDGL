#pragma once

#include <string>

namespace cppTDGL {

class SimulationQueue {
public:
    SimulationQueue() = default;
    virtual ~SimulationQueue() = default;

    [[nodiscard]] std::string className() const;
};

} // namespace cppTDGL
