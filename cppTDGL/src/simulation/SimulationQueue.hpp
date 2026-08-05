#pragma once

#include "SimulationJob.hpp"
#include "SimulationRunner.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace cppTDGL {

class SimulationQueue {
public:
    SimulationQueue() = default;
    virtual ~SimulationQueue() = default;

    [[nodiscard]] std::string className() const;

    void enqueue(SimulationJob job);
    [[nodiscard]] bool empty() const noexcept;
    [[nodiscard]] std::size_t size() const noexcept;
    void clear() noexcept;

    [[nodiscard]] const std::vector<SimulationJob>& jobs() const noexcept;
    [[nodiscard]] const SimulationJob& front() const;

    [[nodiscard]] SimulationJob popNext();
    [[nodiscard]] std::vector<SimulationRunResult> runAll();

private:
    std::vector<SimulationJob> jobs_;
};

} // namespace cppTDGL
