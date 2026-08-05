#include "SimulationQueue.hpp"

#include <stdexcept>
#include <utility>

namespace cppTDGL {

std::string SimulationQueue::className() const {
    return "SimulationQueue";
}

void SimulationQueue::enqueue(SimulationJob job) {
    jobs_.push_back(std::move(job));
}

bool SimulationQueue::empty() const noexcept {
    return jobs_.empty();
}

std::size_t SimulationQueue::size() const noexcept {
    return jobs_.size();
}

void SimulationQueue::clear() noexcept {
    jobs_.clear();
}

const std::vector<SimulationJob>& SimulationQueue::jobs() const noexcept {
    return jobs_;
}

const SimulationJob& SimulationQueue::front() const {
    if (jobs_.empty()) {
        throw std::logic_error("SimulationQueue is empty.");
    }

    return jobs_.front();
}

SimulationJob SimulationQueue::popNext() {
    if (jobs_.empty()) {
        throw std::logic_error("SimulationQueue is empty.");
    }

    SimulationJob job = std::move(jobs_.front());
    jobs_.erase(jobs_.begin());
    return job;
}

std::vector<SimulationRunResult> SimulationQueue::runAll() {
    std::vector<SimulationRunResult> results;
    results.reserve(jobs_.size());

    while (!jobs_.empty()) {
        results.push_back(SimulationRunner::run(popNext()));
    }

    return results;
}

} // namespace cppTDGL
