#include "DependencyFeatures.hpp"

#include <algorithm>
#include <sstream>
#include <stdexcept>

#ifndef CPPTDGL_WITH_QT
#define CPPTDGL_WITH_QT 0
#endif

#ifndef CPPTDGL_WITH_EIGEN
#define CPPTDGL_WITH_EIGEN 0
#endif

#ifndef CPPTDGL_WITH_NATIVE_HDF5
#define CPPTDGL_WITH_NATIVE_HDF5 0
#endif

#ifndef CPPTDGL_WITH_VTK
#define CPPTDGL_WITH_VTK 0
#endif

namespace cppTDGL {
namespace {

[[nodiscard]] bool macroEnabled(int value) noexcept {
    return value != 0;
}

[[nodiscard]] ExternalDependencyFeature makeFeature(
    ExternalDependencyKind kind,
    std::string key,
    std::string displayName,
    std::string cmakeOption,
    std::string compileDefinition,
    std::string purpose,
    bool enabled
) {
    return ExternalDependencyFeature{
        kind,
        std::move(key),
        std::move(displayName),
        std::move(cmakeOption),
        std::move(compileDefinition),
        std::move(purpose),
        enabled,
        false,
    };
}

} // namespace

bool DependencyFeatures::qtWidgetsEnabled() noexcept {
    return macroEnabled(CPPTDGL_WITH_QT);
}

bool DependencyFeatures::eigenEnabled() noexcept {
    return macroEnabled(CPPTDGL_WITH_EIGEN);
}

bool DependencyFeatures::nativeHdf5Enabled() noexcept {
    return macroEnabled(CPPTDGL_WITH_NATIVE_HDF5);
}

bool DependencyFeatures::vtkEnabled() noexcept {
    return macroEnabled(CPPTDGL_WITH_VTK);
}

bool DependencyFeatures::anyOptionalDependencyEnabled() noexcept {
    return qtWidgetsEnabled() || eigenEnabled() || nativeHdf5Enabled() || vtkEnabled();
}

bool DependencyFeatures::isEnabled(ExternalDependencyKind kind) noexcept {
    switch (kind) {
    case ExternalDependencyKind::QtWidgets:
        return qtWidgetsEnabled();
    case ExternalDependencyKind::Eigen:
        return eigenEnabled();
    case ExternalDependencyKind::NativeHdf5:
        return nativeHdf5Enabled();
    case ExternalDependencyKind::Vtk:
        return vtkEnabled();
    }

    return false;
}

bool DependencyFeatures::isEnabledByKey(const std::string& key) {
    const std::vector<ExternalDependencyFeature> features = all();
    const auto iterator = std::find_if(
        features.begin(),
        features.end(),
        [&key](const ExternalDependencyFeature& feature) {
            return feature.key == key;
        }
    );

    if (iterator == features.end()) {
        throw std::invalid_argument("Unknown optional dependency feature key: " + key);
    }

    return iterator->enabled;
}

std::vector<ExternalDependencyFeature> DependencyFeatures::all() {
    return {
        makeFeature(
            ExternalDependencyKind::QtWidgets,
            "qt_widgets",
            "Qt Widgets",
            "CPPTDGL_ENABLE_QT",
            "CPPTDGL_WITH_QT",
            "Optional desktop GUI shell and future interactive widgets.",
            qtWidgetsEnabled()
        ),
        makeFeature(
            ExternalDependencyKind::Eigen,
            "eigen",
            "Eigen",
            "CPPTDGL_ENABLE_EIGEN",
            "CPPTDGL_WITH_EIGEN",
            "Optional future sparse/dense numerical linear algebra backend.",
            eigenEnabled()
        ),
        makeFeature(
            ExternalDependencyKind::NativeHdf5,
            "native_hdf5",
            "Native HDF5",
            "CPPTDGL_ENABLE_NATIVE_HDF5",
            "CPPTDGL_WITH_NATIVE_HDF5",
            "Optional future replacement for the current text-backed HDF5 scaffold.",
            nativeHdf5Enabled()
        ),
        makeFeature(
            ExternalDependencyKind::Vtk,
            "vtk",
            "VTK",
            "CPPTDGL_ENABLE_VTK",
            "CPPTDGL_WITH_VTK",
            "Optional future scientific visualization and mesh rendering backend.",
            vtkEnabled()
        ),
    };
}

std::vector<ExternalDependencyFeature> DependencyFeatures::enabled() {
    std::vector<ExternalDependencyFeature> result;

    for (const ExternalDependencyFeature& feature : all()) {
        if (feature.enabled) {
            result.push_back(feature);
        }
    }

    return result;
}

std::vector<ExternalDependencyFeature> DependencyFeatures::disabled() {
    std::vector<ExternalDependencyFeature> result;

    for (const ExternalDependencyFeature& feature : all()) {
        if (!feature.enabled) {
            result.push_back(feature);
        }
    }

    return result;
}

std::vector<std::string> DependencyFeatures::enabledKeys() {
    std::vector<std::string> result;

    for (const ExternalDependencyFeature& feature : enabled()) {
        result.push_back(feature.key);
    }

    return result;
}

std::vector<std::string> DependencyFeatures::disabledKeys() {
    std::vector<std::string> result;

    for (const ExternalDependencyFeature& feature : disabled()) {
        result.push_back(feature.key);
    }

    return result;
}

std::string DependencyFeatures::summary() {
    std::ostringstream output;
    output << "Optional dependencies:";

    for (const ExternalDependencyFeature& feature : all()) {
        output << ' ' << feature.key << '=' << (feature.enabled ? "on" : "off") << ';';
    }

    return output.str();
}

} // namespace cppTDGL
