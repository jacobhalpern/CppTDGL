#include "PackagingInfo.hpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {

void failIfFalse(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

[[nodiscard]] bool containsText(const std::string& value, std::string_view expected) {
    return value.find(expected) != std::string::npos;
}

[[nodiscard]] bool containsEntryNamed(
    const std::vector<cppTDGL::PackagingManifestEntry>& entries,
    std::string_view logicalName
) {
    return std::any_of(
        entries.begin(),
        entries.end(),
        [logicalName](const cppTDGL::PackagingManifestEntry& entry) {
            return entry.logicalName == logicalName;
        }
    );
}

} // namespace

int main() {
    try {
        failIfFalse(cppTDGL::PackagingInfo::applicationName() == "CppTDGL", "Application name should be CppTDGL.");
        failIfFalse(cppTDGL::PackagingInfo::packageName() == "CppTDGL", "Package name should be CppTDGL.");
        failIfFalse(cppTDGL::PackagingInfo::packageVersion() == "0.1.0", "Package version should match project version.");
        failIfFalse(cppTDGL::PackagingInfo::defaultArchiveBaseName() == "CppTDGL-0.1.0", "Default archive base name should combine package name and version.");

        const cppTDGL::PackagingInstallLayout layout = cppTDGL::PackagingInfo::installLayout();

        failIfFalse(layout.binaryDirectory == "bin", "Packaging binary directory should be bin.");
        failIfFalse(layout.libraryDirectory == "lib", "Packaging library directory should be lib.");
        failIfFalse(layout.includeDirectory == "include/cppTDGL", "Packaging include directory should be include/cppTDGL.");
        failIfFalse(layout.documentationDirectory == "share/cppTDGL", "Packaging documentation directory should be share/cppTDGL.");

        const std::vector<cppTDGL::PackagingManifestEntry> manifest = cppTDGL::PackagingInfo::manifest();
        failIfFalse(manifest.size() >= 5, "Packaging manifest should include required and optional artifacts.");
        failIfFalse(containsEntryNamed(manifest, "cppTDGL static library"), "Manifest should include the core library.");
        failIfFalse(containsEntryNamed(manifest, "CppTDGLSmokeCli"), "Manifest should include the smoke CLI.");
        failIfFalse(containsEntryNamed(manifest, "public headers"), "Manifest should include public headers.");
        failIfFalse(containsEntryNamed(manifest, "CppTDGLQtApp"), "Manifest should include the optional Qt app.");

        const std::vector<cppTDGL::PackagingManifestEntry> required = cppTDGL::PackagingInfo::requiredRuntimeArtifacts();
        const std::vector<cppTDGL::PackagingManifestEntry> optional = cppTDGL::PackagingInfo::optionalRuntimeArtifacts();

        failIfFalse(!required.empty(), "Packaging info should expose required runtime artifacts.");
        failIfFalse(!optional.empty(), "Packaging info should expose optional runtime artifacts.");
        failIfFalse(containsEntryNamed(required, "CppTDGLSmokeCli"), "Required artifacts should include the smoke CLI.");
        failIfFalse(containsEntryNamed(optional, "CppTDGLQtApp"), "Optional artifacts should include the Qt app.");

        failIfFalse(cppTDGL::PackagingInfo::isPackageConfigurationValid(), "Packaging info should validate successfully.");
        failIfFalse(cppTDGL::PackagingInfo::validationErrors().empty(), "Packaging validation should have no errors.");

        const std::string summary = cppTDGL::PackagingInfo::deploymentSummary();

        failIfFalse(containsText(summary, "CppTDGL"), "Deployment summary should include the package name.");
        failIfFalse(containsText(summary, "0.1.0"), "Deployment summary should include the package version.");
        failIfFalse(containsText(summary, "include/cppTDGL"), "Deployment summary should include the include directory.");
        failIfFalse(containsText(summary, "manifestEntries"), "Deployment summary should include manifest entry count.");

        std::cout << "Packaging info smoke test passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Packaging info smoke test exception: " << error.what() << '\n';
        return 1;
    }
}
