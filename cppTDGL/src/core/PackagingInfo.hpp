#pragma once

#include <string>
#include <vector>

namespace cppTDGL {

struct PackagingInstallLayout {
    std::string binaryDirectory = "bin";
    std::string libraryDirectory = "lib";
    std::string includeDirectory = "include/cppTDGL";
    std::string documentationDirectory = "share/cppTDGL";
};

struct PackagingManifestEntry {
    std::string logicalName;
    std::string relativePath;
    std::string description;
    bool required = true;
};

class PackagingInfo {
public:
    [[nodiscard]] static std::string applicationName();
    [[nodiscard]] static std::string packageName();
    [[nodiscard]] static std::string packageVendor();
    [[nodiscard]] static std::string packageVersion();
    [[nodiscard]] static std::string defaultArchiveBaseName();

    [[nodiscard]] static PackagingInstallLayout installLayout();
    [[nodiscard]] static std::vector<PackagingManifestEntry> manifest();
    [[nodiscard]] static std::vector<PackagingManifestEntry> requiredRuntimeArtifacts();
    [[nodiscard]] static std::vector<PackagingManifestEntry> optionalRuntimeArtifacts();

    [[nodiscard]] static std::string deploymentSummary();
    [[nodiscard]] static bool isPackageConfigurationValid();
    [[nodiscard]] static std::vector<std::string> validationErrors();
};

} // namespace cppTDGL
