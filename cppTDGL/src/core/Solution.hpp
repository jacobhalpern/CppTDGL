#pragma once

#include <filesystem>
#include <string>

namespace cppTDGL {

class Solution {
public:
    Solution() = default;
    Solution(std::string projectName, std::filesystem::path resultPath, int savedFrameCount);

    [[nodiscard]] const std::string& projectName() const noexcept;
    [[nodiscard]] const std::filesystem::path& resultPath() const noexcept;
    [[nodiscard]] int savedFrameCount() const noexcept;
    [[nodiscard]] bool hasResultPath() const;

private:
    std::string projectName_ = "untitled";
    std::filesystem::path resultPath_;
    int savedFrameCount_ = 0;
};

} // namespace cppTDGL
