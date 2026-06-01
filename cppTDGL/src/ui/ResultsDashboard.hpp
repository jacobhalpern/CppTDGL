#pragma once

#include <string>

namespace cppTDGL {

class ResultsDashboard {
public:
    ResultsDashboard() = default;
    virtual ~ResultsDashboard() = default;

    [[nodiscard]] std::string className() const;
};

} // namespace cppTDGL
