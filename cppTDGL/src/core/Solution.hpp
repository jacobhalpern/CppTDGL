#pragma once

#include <string>

namespace cppTDGL {

class Solution {
public:
    Solution() = default;
    virtual ~Solution() = default;

    [[nodiscard]] std::string className() const;
};

} // namespace cppTDGL
