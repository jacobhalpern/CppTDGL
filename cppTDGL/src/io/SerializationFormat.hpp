#pragma once

#include <string_view>

namespace cppTDGL::SerializationFormat {

inline constexpr int earliestSupportedProjectFileVersion = 1;
inline constexpr int latestProjectFileVersion = 2;
inline constexpr int latestMeshFileVersion = 1;
inline constexpr int latestSolutionScaffoldFileVersion = 1;

inline constexpr std::string_view projectFileExtension = ".cpptdgl";
inline constexpr std::string_view meshFileExtension = ".mesh";
inline constexpr std::string_view solutionFileExtension = ".h5";

inline constexpr std::string_view projectFileSignature = "CPPTDGL_PROJECT_VERSION";
inline constexpr std::string_view meshFileSignature = "CPPTDGL_MESH_VERSION";
inline constexpr std::string_view solutionScaffoldFileSignature = "CPPTDGL_SOLUTION_SCAFFOLD_VERSION";

[[nodiscard]] constexpr bool isSupportedProjectFileVersion(int version) noexcept {
    return version >= earliestSupportedProjectFileVersion &&
           version <= latestProjectFileVersion;
}

[[nodiscard]] constexpr bool isSupportedMeshFileVersion(int version) noexcept {
    return version == latestMeshFileVersion;
}

[[nodiscard]] constexpr bool isSupportedSolutionScaffoldFileVersion(int version) noexcept {
    return version == latestSolutionScaffoldFileVersion;
}

} // namespace cppTDGL::SerializationFormat
