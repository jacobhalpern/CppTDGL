#include "PackagingInfo.hpp"

#include <sstream>
#include <utility>

namespace cppTDGL {
namespace {

[[nodiscard]] bool isEmpty(const std::string& value) noexcept {
    return value.empty();
}

[[nodiscard]] std::vector<PackagingManifestEntry> filterByRequiredState(
    const std::vector<PackagingManifestEntry>& entries,
    bool required
) {
    std::vector<PackagingManifestEntry> filtered;

    for (const PackagingManifestEntry& entry : entries) {
        if (entry.required == required) {
            filtered.push_back(entry);
        }
    }

    return filtered;
}

} // namespace

std::string PackagingInfo::applicationName() {
    return "CppTDGL";
}

std::string PackagingInfo::packageName() {
    return "CppTDGL";
}

std::string PackagingInfo::packageVendor() {
    return "CppTDGL Project";
}

std::string PackagingInfo::packageVersion() {
    return "0.1.0";
}

std::string PackagingInfo::defaultArchiveBaseName() {
    return packageName() + "-" + packageVersion();
}

PackagingInstallLayout PackagingInfo::installLayout() {
    return PackagingInstallLayout{};
}

std::vector<PackagingManifestEntry> PackagingInfo::manifest() {
    return {
        {
            "cppTDGL static library",
            "lib/cppTDGL",
            "Native C++ TDGL core library artifact.",
            true,
        },
        {
            "CppTDGLSmokeCli",
            "bin/CppTDGLSmokeCli",
            "Command-line smoke executable for end-to-end simulation validation.",
            true,
        },
        {
            "public headers",
            "include/cppTDGL",
            "Public CppTDGL C++ headers needed by downstream consumers.",
            true,
        },
        {
            "CppTDGLQtApp",
            "bin/CppTDGLQtApp",
            "Optional Qt Widgets application shell when CPPTDGL_ENABLE_QT is enabled.",
            false,
        },
        {
            "package documentation",
            "share/cppTDGL",
            "Packaging and deployment notes for release artifacts.",
            false,
        },
    };
}

std::vector<PackagingManifestEntry> PackagingInfo::requiredRuntimeArtifacts() {
    return filterByRequiredState(manifest(), true);
}

std::vector<PackagingManifestEntry> PackagingInfo::optionalRuntimeArtifacts() {
    return filterByRequiredState(manifest(), false);
}

std::string PackagingInfo::deploymentSummary() {
    const PackagingInstallLayout layout = installLayout();
    const std::vector<PackagingManifestEntry> entries = manifest();

    std::ostringstream output;
    output << packageName()
           << " " << packageVersion()
           << " deployment scaffold: bin=" << layout.binaryDirectory
           << ", lib=" << layout.libraryDirectory
           << ", include=" << layout.includeDirectory
           << ", docs=" << layout.documentationDirectory
           << ", manifestEntries=" << entries.size();

    return output.str();
}

bool PackagingInfo::isPackageConfigurationValid() {
    return validationErrors().empty();
}

std::vector<std::string> PackagingInfo::validationErrors() {
    std::vector<std::string> errors;

    if (isEmpty(applicationName())) {
        errors.emplace_back("Application name must not be empty.");
    }

    if (isEmpty(packageName())) {
        errors.emplace_back("Package name must not be empty.");
    }

    if (isEmpty(packageVendor())) {
        errors.emplace_back("Package vendor must not be empty.");
    }

    if (isEmpty(packageVersion())) {
        errors.emplace_back("Package version must not be empty.");
    }

    const PackagingInstallLayout layout = installLayout();
    if (isEmpty(layout.binaryDirectory)) {
        errors.emplace_back("Packaging binary directory must not be empty.");
    }

    if (isEmpty(layout.libraryDirectory)) {
        errors.emplace_back("Packaging library directory must not be empty.");
    }

    if (isEmpty(layout.includeDirectory)) {
        errors.emplace_back("Packaging include directory must not be empty.");
    }

    const std::vector<PackagingManifestEntry> entries = manifest();
    if (entries.empty()) {
        errors.emplace_back("Packaging manifest must contain at least one entry.");
    }

    bool hasRequiredEntry = false;
    for (const PackagingManifestEntry& entry : entries) {
        if (isEmpty(entry.logicalName)) {
            errors.emplace_back("Packaging manifest entry logical name must not be empty.");
        }

        if (isEmpty(entry.relativePath)) {
            errors.emplace_back("Packaging manifest entry relative path must not be empty.");
        }

        if (isEmpty(entry.description)) {
            errors.emplace_back("Packaging manifest entry description must not be empty.");
        }

        hasRequiredEntry = hasRequiredEntry || entry.required;
    }

    if (!hasRequiredEntry) {
        errors.emplace_back("Packaging manifest must contain at least one required entry.");
    }

    return errors;
}

} // namespace cppTDGL
