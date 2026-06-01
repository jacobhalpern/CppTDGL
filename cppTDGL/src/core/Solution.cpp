#include "Solution.hpp"

#include <utility>

namespace cppTDGL {

Solution::Solution(std::string projectName, std::filesystem::path resultPath, int savedFrameCount)
    : projectName_(std::move(projectName)),
      resultPath_(std::move(resultPath)),
      savedFrameCount_(savedFrameCount) {}

const std::string& Solution::projectName() const noexcept {
    return projectName_;
}

const std::filesystem::path& Solution::resultPath() const noexcept {
    return resultPath_;
}

int Solution::savedFrameCount() const noexcept {
    return savedFrameCount_;
}

bool Solution::hasResultPath() const {
    return !resultPath_.empty();
}

} // namespace cppTDGL
