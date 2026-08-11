#include "DependencyFeatures.hpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void failIfFalse(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

const cppTDGL::ExternalDependencyFeature& findFeature(
    const std::vector<cppTDGL::ExternalDependencyFeature>& features,
    const std::string& key
) {
    const auto iterator = std::find_if(
        features.begin(),
        features.end(),
        [&key](const cppTDGL::ExternalDependencyFeature& feature) {
            return feature.key == key;
        }
    );

    if (iterator == features.end()) {
        throw std::runtime_error("Could not find dependency feature: " + key);
    }

    return *iterator;
}

} // namespace

int main() {
    try {
        const std::vector<cppTDGL::ExternalDependencyFeature> features =
            cppTDGL::DependencyFeatures::all();

        failIfFalse(features.size() == 4, "There should be four optional dependency gates.");

        const cppTDGL::ExternalDependencyFeature& qt = findFeature(features, "qt_widgets");
        const cppTDGL::ExternalDependencyFeature& eigen = findFeature(features, "eigen");
        const cppTDGL::ExternalDependencyFeature& hdf5 = findFeature(features, "native_hdf5");
        const cppTDGL::ExternalDependencyFeature& vtk = findFeature(features, "vtk");

        failIfFalse(qt.displayName == "Qt Widgets", "Qt display name should be stable.");
        failIfFalse(eigen.displayName == "Eigen", "Eigen display name should be stable.");
        failIfFalse(hdf5.displayName == "Native HDF5", "HDF5 display name should be stable.");
        failIfFalse(vtk.displayName == "VTK", "VTK display name should be stable.");

        failIfFalse(qt.cmakeOption == "CPPTDGL_ENABLE_QT", "Qt CMake option should be stable.");
        failIfFalse(eigen.cmakeOption == "CPPTDGL_ENABLE_EIGEN", "Eigen CMake option should be stable.");
        failIfFalse(hdf5.cmakeOption == "CPPTDGL_ENABLE_NATIVE_HDF5", "HDF5 CMake option should be stable.");
        failIfFalse(vtk.cmakeOption == "CPPTDGL_ENABLE_VTK", "VTK CMake option should be stable.");

        failIfFalse(qt.compileDefinition == "CPPTDGL_WITH_QT", "Qt compile definition should be stable.");
        failIfFalse(eigen.compileDefinition == "CPPTDGL_WITH_EIGEN", "Eigen compile definition should be stable.");
        failIfFalse(hdf5.compileDefinition == "CPPTDGL_WITH_NATIVE_HDF5", "HDF5 compile definition should be stable.");
        failIfFalse(vtk.compileDefinition == "CPPTDGL_WITH_VTK", "VTK compile definition should be stable.");

        for (const cppTDGL::ExternalDependencyFeature& feature : features) {
            failIfFalse(
                !feature.requiredForDefaultBuild,
                "No optional dependency should be required for the default build."
            );

            failIfFalse(
                cppTDGL::DependencyFeatures::isEnabled(feature.kind) == feature.enabled,
                "Kind-based dependency lookup should match feature state."
            );

            failIfFalse(
                cppTDGL::DependencyFeatures::isEnabledByKey(feature.key) == feature.enabled,
                "Key-based dependency lookup should match feature state."
            );
        }

        bool unknownKeyThrew = false;
        try {
            static_cast<void>(cppTDGL::DependencyFeatures::isEnabledByKey("not_a_dependency"));
        } catch (const std::invalid_argument&) {
            unknownKeyThrew = true;
        }

        failIfFalse(unknownKeyThrew, "Unknown dependency key lookup should throw.");

        const std::vector<cppTDGL::ExternalDependencyFeature> enabled =
            cppTDGL::DependencyFeatures::enabled();
        const std::vector<cppTDGL::ExternalDependencyFeature> disabled =
            cppTDGL::DependencyFeatures::disabled();

        failIfFalse(
            enabled.size() + disabled.size() == features.size(),
            "Enabled and disabled dependency lists should partition all features."
        );

        const std::vector<std::string> enabledKeys = cppTDGL::DependencyFeatures::enabledKeys();
        const std::vector<std::string> disabledKeys = cppTDGL::DependencyFeatures::disabledKeys();

        failIfFalse(
            enabledKeys.size() == enabled.size(),
            "Enabled key count should equal enabled feature count."
        );

        failIfFalse(
            disabledKeys.size() == disabled.size(),
            "Disabled key count should equal disabled feature count."
        );

        const std::string summary = cppTDGL::DependencyFeatures::summary();

        failIfFalse(summary.find("qt_widgets=") != std::string::npos, "Summary should include Qt state.");
        failIfFalse(summary.find("eigen=") != std::string::npos, "Summary should include Eigen state.");
        failIfFalse(summary.find("native_hdf5=") != std::string::npos, "Summary should include HDF5 state.");
        failIfFalse(summary.find("vtk=") != std::string::npos, "Summary should include VTK state.");

        std::cout << "Dependency features smoke test passed. " << summary << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Dependency features smoke test exception: " << error.what() << '\n';
        return 1;
    }
}
