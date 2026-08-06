#pragma once

#include <string>

namespace cppTDGL {

class ApplicationInfo {
public:
    [[nodiscard]] static std::string applicationName();
    [[nodiscard]] static std::string organizationName();
    [[nodiscard]] static std::string defaultWindowTitle();
    [[nodiscard]] static std::string aboutText();
    [[nodiscard]] static std::string projectFileFilter();
    [[nodiscard]] static std::string solutionFileFilter();
};

} // namespace cppTDGL
