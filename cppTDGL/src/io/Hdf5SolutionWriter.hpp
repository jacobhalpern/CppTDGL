#pragma once

#include "Solution.hpp"

#include <filesystem>
#include <string>

namespace cppTDGL {

class Hdf5SolutionWriter {
public:
    Hdf5SolutionWriter() = default;
    virtual ~Hdf5SolutionWriter() = default;

    [[nodiscard]] std::string className() const;

    static void write(const Solution& solution);
    static void write(const Solution& solution, const std::filesystem::path& path);
};

} // namespace cppTDGL
