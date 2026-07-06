#include "Solution.hpp"

#include <stdexcept>
#include <string>
#include <utility>

namespace cppTDGL {
namespace {

void appendErrors(
    std::vector<std::string>& destination,
    const std::string& prefix,
    const std::vector<std::string>& source
) {
    for (const std::string& error : source) {
        destination.push_back(prefix + error);
    }
}

} // namespace

Solution::Solution(
    std::string projectName,
    std::filesystem::path resultPath,
    int savedFrameCount
)
    : projectName_(std::move(projectName)),
      resultPath_(std::move(resultPath)),
      savedFrameCount_(savedFrameCount) {}

Solution::Solution(
    std::string projectName,
    Mesh mesh,
    std::vector<SolverFrame> frames,
    std::filesystem::path resultPath
)
    : projectName_(std::move(projectName)),
      resultPath_(std::move(resultPath)),
      mesh_(std::move(mesh)) {
    setFrames(std::move(frames));
}

Solution Solution::fromSolverState(
    std::string projectName,
    const SolverState& state,
    std::filesystem::path resultPath
) {
    Solution solution(
        std::move(projectName),
        state.mesh(),
        state.frames(),
        std::move(resultPath)
    );

    solution.setCompleted(state.hasFrames());
    return solution;
}

const std::string& Solution::projectName() const noexcept {
    return projectName_;
}

void Solution::setProjectName(std::string value) {
    projectName_ = std::move(value);
}

const std::filesystem::path& Solution::resultPath() const noexcept {
    return resultPath_;
}

void Solution::setResultPath(std::filesystem::path value) {
    resultPath_ = std::move(value);
}

bool Solution::hasResultPath() const {
    return !resultPath_.empty();
}

int Solution::savedFrameCount() const noexcept {
    if (!frames_.empty()) {
        return static_cast<int>(frames_.size());
    }

    return savedFrameCount_;
}

std::size_t Solution::frameCount() const noexcept {
    return frames_.size();
}

bool Solution::hasFrames() const noexcept {
    return !frames_.empty();
}

bool Solution::hasMesh() const noexcept {
    return mesh_.has_value();
}

const Mesh& Solution::mesh() const {
    if (!mesh_.has_value()) {
        throw std::logic_error("Solution does not have an attached mesh.");
    }

    return *mesh_;
}

Mesh& Solution::mesh() {
    if (!mesh_.has_value()) {
        throw std::logic_error("Solution does not have an attached mesh.");
    }

    return *mesh_;
}

void Solution::setMesh(Mesh value) {
    mesh_ = std::move(value);

    for (const SolverFrame& frame : frames_) {
        validateFrameOrThrow(frame);
    }
}

void Solution::clearMesh() {
    if (!frames_.empty()) {
        throw std::logic_error("Cannot clear Solution mesh while frames are stored.");
    }

    mesh_.reset();
}

const std::vector<SolverFrame>& Solution::frames() const noexcept {
    return frames_;
}

const SolverFrame& Solution::frame(std::size_t index) const {
    if (index >= frames_.size()) {
        throw std::out_of_range("Solution frame index is out of range.");
    }

    return frames_[index];
}

const SolverFrame& Solution::latestFrame() const {
    if (frames_.empty()) {
        throw std::logic_error("Solution has no frames.");
    }

    return frames_.back();
}

void Solution::appendFrame(SolverFrame frame) {
    validateFrameOrThrow(frame);
    frames_.push_back(std::move(frame));
}

void Solution::setFrames(std::vector<SolverFrame> frames) {
    for (const SolverFrame& frame : frames) {
        validateFrameOrThrow(frame);
    }

    frames_ = std::move(frames);
}

void Solution::clearFrames() {
    frames_.clear();
}

bool Solution::completed() const noexcept {
    return completed_;
}

void Solution::setCompleted(bool value) noexcept {
    completed_ = value;
}

const std::vector<std::string>& Solution::diagnostics() const noexcept {
    return diagnostics_;
}

void Solution::addDiagnostic(std::string diagnostic) {
    diagnostics_.push_back(std::move(diagnostic));
}

void Solution::clearDiagnostics() {
    diagnostics_.clear();
}

bool Solution::isValid() const {
    return validationErrors().empty();
}

std::vector<std::string> Solution::validationErrors() const {
    std::vector<std::string> errors;

    if (projectName_.empty()) {
        errors.emplace_back("Solution project name must not be empty.");
    }

    if (savedFrameCount_ < 0) {
        errors.emplace_back("Solution saved frame count must not be negative.");
    }

    if (frames_.empty()) {
        if (mesh_.has_value()) {
            appendErrors(errors, "Solution mesh: ", mesh_->validationErrors());
        }
        return errors;
    }

    if (!mesh_.has_value()) {
        errors.emplace_back("Solution with frames must have an attached mesh.");
        return errors;
    }

    appendErrors(errors, "Solution mesh: ", mesh_->validationErrors());

    if (!mesh_->isValid()) {
        return errors;
    }

    const EdgeMesh edgeMesh = EdgeMesh::fromMesh(*mesh_);

    for (std::size_t i = 0; i < frames_.size(); ++i) {
        const std::vector<std::string> frameErrors = frames_[i].validationErrors(*mesh_, edgeMesh);
        for (const std::string& error : frameErrors) {
            errors.push_back("Solution frame " + std::to_string(i) + ": " + error);
        }
    }

    return errors;
}

void Solution::validateFrameOrThrow(const SolverFrame& frame) const {
    if (!mesh_.has_value()) {
        throw std::logic_error("Cannot store Solution frames without an attached mesh.");
    }

    if (!mesh_->isValid()) {
        throw std::invalid_argument("Cannot store Solution frames with an invalid mesh.");
    }

    const EdgeMesh edgeMesh = EdgeMesh::fromMesh(*mesh_);
    const std::vector<std::string> errors = frame.validationErrors(*mesh_, edgeMesh);

    if (!errors.empty()) {
        throw std::invalid_argument(errors.front());
    }
}

} // namespace cppTDGL
