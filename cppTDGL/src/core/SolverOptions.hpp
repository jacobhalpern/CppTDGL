#pragma once

#include <string>

namespace cppTDGL {

class SolverOptions {
public:
    SolverOptions() = default;
    virtual ~SolverOptions() = default;

    [[nodiscard]] std::string className() const;
};

} // namespace cppTDGL
