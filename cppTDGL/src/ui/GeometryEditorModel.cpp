#include "GeometryEditorModel.hpp"

#include <stdexcept>
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

void throwIfIndexOutOfRange(std::size_t index, std::size_t size, const std::string& label) {
    if (index >= size) {
        throw std::out_of_range(label + " index is out of range.");
    }
}

bool pointInsideAnyPolygon(Point2D point, const std::vector<Polygon2D>& polygons) {
    for (const Polygon2D& polygon : polygons) {
        if (polygon.containsPoint(point)) {
            return true;
        }
    }
    return false;
}

} // namespace

GeometryEditorSelection GeometryEditorSelection::none() noexcept {
    return GeometryEditorSelection{};
}

GeometryEditorSelection GeometryEditorSelection::film() noexcept {
    GeometryEditorSelection selection;
    selection.hasSelection = true;
    selection.kind = GeometryEditorSelectionKind::Film;
    selection.index = 0;
    return selection;
}

GeometryEditorSelection GeometryEditorSelection::hole(std::size_t index) noexcept {
    GeometryEditorSelection selection;
    selection.hasSelection = true;
    selection.kind = GeometryEditorSelectionKind::Hole;
    selection.index = index;
    return selection;
}

GeometryEditorSelection GeometryEditorSelection::terminal(std::size_t index) noexcept {
    GeometryEditorSelection selection;
    selection.hasSelection = true;
    selection.kind = GeometryEditorSelectionKind::Terminal;
    selection.index = index;
    return selection;
}

GeometryEditorSelection GeometryEditorSelection::probePoint(std::size_t index) noexcept {
    GeometryEditorSelection selection;
    selection.hasSelection = true;
    selection.kind = GeometryEditorSelectionKind::ProbePoint;
    selection.index = index;
    return selection;
}

GeometryEditorModel GeometryEditorModel::createDefault() {
    return GeometryEditorModel{};
}

GeometryEditorModel GeometryEditorModel::fromDevice(const Device& device) {
    GeometryEditorModel model;
    model.setProjectName(device.projectName());
    model.setLayer(device.layer());
    model.setSolverOptions(device.solverOptions());
    model.setFilm(device.film());

    for (const Polygon2D& hole : device.holes()) {
        model.addHole(hole);
    }

    for (const Polygon2D& terminal : device.terminals()) {
        model.addTerminal(terminal);
    }

    for (const Point2D& probePoint : device.probePoints()) {
        model.addProbePoint(probePoint);
    }

    return model;
}

const std::string& GeometryEditorModel::projectName() const noexcept {
    return projectName_;
}

void GeometryEditorModel::setProjectName(std::string value) {
    projectName_ = std::move(value);
}

const Layer& GeometryEditorModel::layer() const noexcept {
    return layer_;
}

Layer& GeometryEditorModel::layer() noexcept {
    return layer_;
}

void GeometryEditorModel::setLayer(Layer value) {
    layer_ = std::move(value);
}

const SolverOptions& GeometryEditorModel::solverOptions() const noexcept {
    return solverOptions_;
}

SolverOptions& GeometryEditorModel::solverOptions() noexcept {
    return solverOptions_;
}

void GeometryEditorModel::setSolverOptions(SolverOptions value) {
    solverOptions_ = std::move(value);
}

const Polygon2D& GeometryEditorModel::film() const noexcept {
    return film_;
}

void GeometryEditorModel::setFilm(Polygon2D value) {
    film_ = std::move(value);
    selection_ = GeometryEditorSelection::film();
}

const std::vector<Polygon2D>& GeometryEditorModel::holes() const noexcept {
    return holes_;
}

const std::vector<Polygon2D>& GeometryEditorModel::terminals() const noexcept {
    return terminals_;
}

const std::vector<Point2D>& GeometryEditorModel::probePoints() const noexcept {
    return probePoints_;
}

std::size_t GeometryEditorModel::holeCount() const noexcept {
    return holes_.size();
}

std::size_t GeometryEditorModel::terminalCount() const noexcept {
    return terminals_.size();
}

std::size_t GeometryEditorModel::probePointCount() const noexcept {
    return probePoints_.size();
}

void GeometryEditorModel::addHole(Polygon2D value) {
    holes_.push_back(std::move(value));
    selection_ = GeometryEditorSelection::hole(holes_.size() - 1);
}

void GeometryEditorModel::addRectangularHole(
    std::string name,
    double width,
    double height,
    Point2D center
) {
    addHole(Polygon2D::rectangle(std::move(name), width, height, center));
}

void GeometryEditorModel::addCircularHole(
    std::string name,
    double radius,
    std::size_t segmentCount,
    Point2D center
) {
    addHole(Polygon2D::circleApproximation(std::move(name), radius, segmentCount, center));
}

void GeometryEditorModel::removeHole(std::size_t index) {
    throwIfIndexOutOfRange(index, holes_.size(), "Hole");
    holes_.erase(holes_.begin() + static_cast<std::ptrdiff_t>(index));
    selection_ = GeometryEditorSelection::none();
}

void GeometryEditorModel::clearHoles() {
    holes_.clear();
    selection_ = GeometryEditorSelection::none();
}

void GeometryEditorModel::addTerminal(Polygon2D value) {
    terminals_.push_back(std::move(value));
    selection_ = GeometryEditorSelection::terminal(terminals_.size() - 1);
}

void GeometryEditorModel::addRectangularTerminal(
    std::string name,
    double width,
    double height,
    Point2D center
) {
    addTerminal(Polygon2D::rectangle(std::move(name), width, height, center));
}

void GeometryEditorModel::removeTerminal(std::size_t index) {
    throwIfIndexOutOfRange(index, terminals_.size(), "Terminal");
    terminals_.erase(terminals_.begin() + static_cast<std::ptrdiff_t>(index));
    selection_ = GeometryEditorSelection::none();
}

void GeometryEditorModel::clearTerminals() {
    terminals_.clear();
    selection_ = GeometryEditorSelection::none();
}

void GeometryEditorModel::addProbePoint(Point2D value) {
    probePoints_.push_back(value);
    selection_ = GeometryEditorSelection::probePoint(probePoints_.size() - 1);
}

void GeometryEditorModel::removeProbePoint(std::size_t index) {
    throwIfIndexOutOfRange(index, probePoints_.size(), "Probe point");
    probePoints_.erase(probePoints_.begin() + static_cast<std::ptrdiff_t>(index));
    selection_ = GeometryEditorSelection::none();
}

void GeometryEditorModel::clearProbePoints() {
    probePoints_.clear();
    selection_ = GeometryEditorSelection::none();
}

void GeometryEditorModel::selectFilm() noexcept {
    selection_ = GeometryEditorSelection::film();
}

void GeometryEditorModel::selectHole(std::size_t index) {
    throwIfIndexOutOfRange(index, holes_.size(), "Hole");
    selection_ = GeometryEditorSelection::hole(index);
}

void GeometryEditorModel::selectTerminal(std::size_t index) {
    throwIfIndexOutOfRange(index, terminals_.size(), "Terminal");
    selection_ = GeometryEditorSelection::terminal(index);
}

void GeometryEditorModel::selectProbePoint(std::size_t index) {
    throwIfIndexOutOfRange(index, probePoints_.size(), "Probe point");
    selection_ = GeometryEditorSelection::probePoint(index);
}

void GeometryEditorModel::clearSelection() noexcept {
    selection_ = GeometryEditorSelection::none();
}

const GeometryEditorSelection& GeometryEditorModel::selection() const noexcept {
    return selection_;
}

bool GeometryEditorModel::hasValidGeometry() const {
    return validationErrors().empty();
}

bool GeometryEditorModel::isValid() const {
    return validationErrors().empty();
}

std::vector<std::string> GeometryEditorModel::validationErrors() const {
    std::vector<std::string> errors;

    if (projectName_.empty()) {
        errors.emplace_back("Project name must not be empty.");
    }

    appendErrors(errors, "Layer: ", layer_.validationErrors());
    appendErrors(errors, "Film: ", film_.validationErrors());
    appendErrors(errors, "Solver: ", solverOptions_.validationErrors());

    for (std::size_t i = 0; i < holes_.size(); ++i) {
        appendErrors(errors, "Hole " + std::to_string(i) + ": ", holes_[i].validationErrors());
    }

    for (std::size_t i = 0; i < terminals_.size(); ++i) {
        appendErrors(errors, "Terminal " + std::to_string(i) + ": ", terminals_[i].validationErrors());
    }

    if (film_.isValid()) {
        for (std::size_t i = 0; i < probePoints_.size(); ++i) {
            const Point2D point = probePoints_[i];
            if (!film_.containsPoint(point)) {
                errors.emplace_back("Probe point " + std::to_string(i) + " must be inside the film polygon.");
            }
            if (pointInsideAnyPolygon(point, holes_)) {
                errors.emplace_back("Probe point " + std::to_string(i) + " must not be inside a hole polygon.");
            }
        }
    }

    return errors;
}

Device GeometryEditorModel::buildDevice(
    bool attachGeneratedMesh,
    const MeshGenerationOptions& meshGenerationOptions
) const {
    const std::vector<std::string> errors = validationErrors();

    if (!errors.empty()) {
        throw std::invalid_argument(errors.front());
    }

    Device device(projectName_, layer_, film_, solverOptions_);

    for (const Polygon2D& hole : holes_) {
        device.addHole(hole);
    }

    for (const Polygon2D& terminal : terminals_) {
        device.addTerminal(terminal);
    }

    for (const Point2D& probePoint : probePoints_) {
        device.addProbePoint(probePoint);
    }

    if (attachGeneratedMesh) {
        const std::vector<std::string> meshErrors = meshGenerationOptions.validationErrors();
        if (!meshErrors.empty()) {
            throw std::invalid_argument(meshErrors.front());
        }

        device = MeshGenerator::attachGeneratedFilmBoundingBoxMesh(
            std::move(device),
            meshGenerationOptions
        );
    }

    return device;
}

} // namespace cppTDGL
