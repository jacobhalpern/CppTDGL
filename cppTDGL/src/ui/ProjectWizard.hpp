#pragma once

#include <string>

namespace cppTDGL {

class ProjectWizard {
public:
    ProjectWizard() = default;
    virtual ~ProjectWizard() = default;

    [[nodiscard]] std::string className() const;
};

} // namespace cppTDGL
