#pragma once

#include "Solution.hpp"

#include <filesystem>
#include <string>

namespace cppTDGL {

class Hdf5SolutionReader {
public:
    Hdf5SolutionReader() = default;
    virtual ~Hdf5SolutionReader() = default;

    [[nodiscard]] std::string className() const;

    [[nodiscard]] static Solution read(const std::filesystem::path& path);
};

} // namespace cppTDGL
