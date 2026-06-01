#pragma once

#include "Device.hpp"

#include <filesystem>

namespace cppTDGL {

class ProjectSerializer {
public:
    static void save(const Device& device, const std::filesystem::path& path);
    [[nodiscard]] static Device load(const std::filesystem::path& path);
};

} // namespace cppTDGL
