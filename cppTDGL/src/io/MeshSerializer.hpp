#pragma once

#include "Mesh.hpp"

#include <filesystem>

namespace cppTDGL {

class MeshSerializer {
public:
    static void save(const Mesh& mesh, const std::filesystem::path& path);
    [[nodiscard]] static Mesh load(const std::filesystem::path& path);
};

} // namespace cppTDGL
