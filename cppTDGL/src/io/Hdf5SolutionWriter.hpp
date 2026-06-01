#pragma once

#include <string>

namespace cppTDGL {

class Hdf5SolutionWriter {
public:
    Hdf5SolutionWriter() = default;
    virtual ~Hdf5SolutionWriter() = default;

    [[nodiscard]] std::string className() const;
};

} // namespace cppTDGL
