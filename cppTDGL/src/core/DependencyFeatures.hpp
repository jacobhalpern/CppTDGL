#pragma once

#include <string>
#include <vector>

namespace cppTDGL {

enum class ExternalDependencyKind {
    QtWidgets,
    Eigen,
    NativeHdf5,
    Vtk,
};

struct ExternalDependencyFeature {
    ExternalDependencyKind kind = ExternalDependencyKind::QtWidgets;
    std::string key;
    std::string displayName;
    std::string cmakeOption;
    std::string compileDefinition;
    std::string purpose;
    bool enabled = false;
    bool requiredForDefaultBuild = false;
};

class DependencyFeatures {
public:
    [[nodiscard]] static bool qtWidgetsEnabled() noexcept;
    [[nodiscard]] static bool eigenEnabled() noexcept;
    [[nodiscard]] static bool nativeHdf5Enabled() noexcept;
    [[nodiscard]] static bool vtkEnabled() noexcept;

    [[nodiscard]] static bool anyOptionalDependencyEnabled() noexcept;
    [[nodiscard]] static bool isEnabled(ExternalDependencyKind kind) noexcept;
    [[nodiscard]] static bool isEnabledByKey(const std::string& key);

    [[nodiscard]] static std::vector<ExternalDependencyFeature> all();
    [[nodiscard]] static std::vector<ExternalDependencyFeature> enabled();
    [[nodiscard]] static std::vector<ExternalDependencyFeature> disabled();
    [[nodiscard]] static std::vector<std::string> enabledKeys();
    [[nodiscard]] static std::vector<std::string> disabledKeys();

    [[nodiscard]] static std::string summary();
};

} // namespace cppTDGL
