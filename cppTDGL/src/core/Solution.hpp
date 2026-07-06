#pragma once

#include "Mesh.hpp"
#include "SolverState.hpp"

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace cppTDGL {

class Solution {
public:
    Solution() = default;
    Solution(std::string projectName, std::filesystem::path resultPath, int savedFrameCount);
    Solution(
        std::string projectName,
        Mesh mesh,
        std::vector<SolverFrame> frames = {},
        std::filesystem::path resultPath = {}
    );

    [[nodiscard]] static Solution fromSolverState(
        std::string projectName,
        const SolverState& state,
        std::filesystem::path resultPath = {}
    );

    [[nodiscard]] const std::string& projectName() const noexcept;
    void setProjectName(std::string value);

    [[nodiscard]] const std::filesystem::path& resultPath() const noexcept;
    void setResultPath(std::filesystem::path value);
    [[nodiscard]] bool hasResultPath() const;

    [[nodiscard]] int savedFrameCount() const noexcept;
    [[nodiscard]] std::size_t frameCount() const noexcept;
    [[nodiscard]] bool hasFrames() const noexcept;

    [[nodiscard]] bool hasMesh() const noexcept;
    [[nodiscard]] const Mesh& mesh() const;
    [[nodiscard]] Mesh& mesh();
    void setMesh(Mesh value);
    void clearMesh();

    [[nodiscard]] const std::vector<SolverFrame>& frames() const noexcept;
    [[nodiscard]] const SolverFrame& frame(std::size_t index) const;
    [[nodiscard]] const SolverFrame& latestFrame() const;

    void appendFrame(SolverFrame frame);
    void setFrames(std::vector<SolverFrame> frames);
    void clearFrames();

    [[nodiscard]] bool completed() const noexcept;
    void setCompleted(bool value) noexcept;

    [[nodiscard]] const std::vector<std::string>& diagnostics() const noexcept;
    void addDiagnostic(std::string diagnostic);
    void clearDiagnostics();

    [[nodiscard]] bool isValid() const;
    [[nodiscard]] std::vector<std::string> validationErrors() const;

private:
    std::string projectName_ = "untitled";
    std::filesystem::path resultPath_;
    int savedFrameCount_ = 0;
    std::optional<Mesh> mesh_;
    std::vector<SolverFrame> frames_;
    bool completed_ = false;
    std::vector<std::string> diagnostics_;

    void validateFrameOrThrow(const SolverFrame& frame) const;
};

} // namespace cppTDGL
