#pragma once

#include <string>

namespace cppTDGL {

class ProjectSerializer {
public:
    ProjectSerializer() = default;
    virtual ~ProjectSerializer() = default;

    [[nodiscard]] std::string className() const;
};

} // namespace cppTDGL
