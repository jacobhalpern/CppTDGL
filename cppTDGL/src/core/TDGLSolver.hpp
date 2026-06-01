#pragma once

#include <string>

namespace cppTDGL {

class TDGLSolver {
public:
    TDGLSolver() = default;
    virtual ~TDGLSolver() = default;

    [[nodiscard]] std::string className() const;
};

} // namespace cppTDGL
