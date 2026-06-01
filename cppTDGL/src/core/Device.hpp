#pragma once

#include "Layer.hpp"
#include "Polygon2D.hpp"
#include "SolverOptions.hpp"

#include <string>
#include <vector>

namespace cppTDGL {

class Device {
public:
    Device() = default;
    Device(std::string projectName, Layer layer, Polygon2D film, SolverOptions solverOptions = {});

    [[nodiscard]] const std::string& projectName() const noexcept;
    void setProjectName(std::string value);

    [[nodiscard]] const Layer& layer() const noexcept;
    [[nodiscard]] Layer& layer() noexcept;
    void setLayer(Layer value);

    [[nodiscard]] const Polygon2D& film() const noexcept;
    [[nodiscard]] Polygon2D& film() noexcept;
    void setFilm(Polygon2D value);

    [[nodiscard]] const SolverOptions& solverOptions() const noexcept;
    [[nodiscard]] SolverOptions& solverOptions() noexcept;
    void setSolverOptions(SolverOptions value);

    [[nodiscard]] const std::vector<Polygon2D>& holes() const noexcept;
    [[nodiscard]] const std::vector<Polygon2D>& terminals() const noexcept;
    [[nodiscard]] const std::vector<Point2D>& probePoints() const noexcept;

    void addHole(Polygon2D hole);
    void addTerminal(Polygon2D terminal);
    void addProbePoint(Point2D point);

    [[nodiscard]] bool hasValidGeometry() const;
    [[nodiscard]] bool isValid() const;
    [[nodiscard]] std::vector<std::string> validationErrors() const;

private:
    std::string projectName_ = "untitled";
    Layer layer_{};
    Polygon2D film_{};
    SolverOptions solverOptions_{};
    std::vector<Polygon2D> holes_;
    std::vector<Polygon2D> terminals_;
    std::vector<Point2D> probePoints_;
};

} // namespace cppTDGL
