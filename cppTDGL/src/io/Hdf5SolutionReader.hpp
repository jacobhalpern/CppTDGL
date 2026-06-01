#pragma once

#include <string>

namespace cppTDGL {

class Hdf5SolutionReader {
public:
    Hdf5SolutionReader() = default;
    virtual ~Hdf5SolutionReader() = default;

    [[nodiscard]] std::string className() const;
};

} // namespace cppTDGL
