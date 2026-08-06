#pragma once

#include "Device.hpp"
#include "Layer.hpp"
#include "MeshGenerator.hpp"
#include "Polygon2D.hpp"
#include "SolverOptions.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace cppTDGL {

enum class GeometryEditorSelectionKind {
    Film,
    Hole,
    Terminal,
    ProbePoint,
};

struct GeometryEditorSelection {
    bool hasSelection = false;
    GeometryEditorSelectionKind kind = GeometryEditorSelectionKind::Film;
    std::size_t index = 0;

    [[nodiscard]] static GeometryEditorSelection none() noexcept;
    [[nodiscard]] static GeometryEditorSelection film() noexcept;
    [[nodiscard]] static GeometryEditorSelection hole(std::size_t index) noexcept;
    [[nodiscard]] static GeometryEditorSelection terminal(std::size_t index) noexcept;
    [[nodiscard]] static GeometryEditorSelection probePoint(std::size_t index) noexcept;
};

class GeometryEditorModel {
public:
    GeometryEditorModel() = default;

    [[nodiscard]] static GeometryEditorModel createDefault();
    [[nodiscard]] static GeometryEditorModel fromDevice(const Device& device);

    [[nodiscard]] const std::string& projectName() const noexcept;
    void setProjectName(std::string value);

    [[nodiscard]] const Layer& layer() const noexcept;
    [[nodiscard]] Layer& layer() noexcept;
    void setLayer(Layer value);

    [[nodiscard]] const SolverOptions& solverOptions() const noexcept;
    [[nodiscard]] SolverOptions& solverOptions() noexcept;
    void setSolverOptions(SolverOptions value);

    [[nodiscard]] const Polygon2D& film() const noexcept;
    void setFilm(Polygon2D value);

    [[nodiscard]] const std::vector<Polygon2D>& holes() const noexcept;
    [[nodiscard]] const std::vector<Polygon2D>& terminals() const noexcept;
    [[nodiscard]] const std::vector<Point2D>& probePoints() const noexcept;

    [[nodiscard]] std::size_t holeCount() const noexcept;
    [[nodiscard]] std::size_t terminalCount() const noexcept;
    [[nodiscard]] std::size_t probePointCount() const noexcept;

    void addHole(Polygon2D value);
    void addRectangularHole(std::string name, double width, double height, Point2D center = {});
    void addCircularHole(std::string name, double radius, std::size_t segmentCount = 64, Point2D center = {});
    void removeHole(std::size_t index);
    void clearHoles();

    void addTerminal(Polygon2D value);
    void addRectangularTerminal(std::string name, double width, double height, Point2D center = {});
    void removeTerminal(std::size_t index);
    void clearTerminals();

    void addProbePoint(Point2D value);
    void removeProbePoint(std::size_t index);
    void clearProbePoints();

    void selectFilm() noexcept;
    void selectHole(std::size_t index);
    void selectTerminal(std::size_t index);
    void selectProbePoint(std::size_t index);
    void clearSelection() noexcept;
    [[nodiscard]] const GeometryEditorSelection& selection() const noexcept;

    [[nodiscard]] bool hasValidGeometry() const;
    [[nodiscard]] bool isValid() const;
    [[nodiscard]] std::vector<std::string> validationErrors() const;

    [[nodiscard]] Device buildDevice(
        bool attachGeneratedMesh = false,
        const MeshGenerationOptions& meshGenerationOptions = {}
    ) const;

private:
    std::string projectName_ = "cppTDGL_geometry_editor_project";
    Layer layer_{};
    SolverOptions solverOptions_{};
    Polygon2D film_ = Polygon2D::rectangle("film", 10.0, 4.0);
    std::vector<Polygon2D> holes_;
    std::vector<Polygon2D> terminals_;
    std::vector<Point2D> probePoints_;
    GeometryEditorSelection selection_ = GeometryEditorSelection::none();
};

} // namespace cppTDGL
