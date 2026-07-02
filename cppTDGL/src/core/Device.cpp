#include "Device.hpp"

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

bool pointInsideAnyPolygon(Point2D point, const std::vector<Polygon2D>& polygons) {
    for (const Polygon2D& polygon : polygons) {
        if (polygon.containsPoint(point)) {
            return true;
        }
    }
    return false;
}

} // namespace

Device::Device(
    std::string projectName,
    Layer layer,
    Polygon2D film,
    SolverOptions solverOptions
)
    : projectName_(std::move(projectName)),
      layer_(std::move(layer)),
      film_(std::move(film)),
      solverOptions_(std::move(solverOptions)) {}

const std::string& Device::projectName() const noexcept { return projectName_; }
void Device::setProjectName(std::string value) { projectName_ = std::move(value); }

const Layer& Device::layer() const noexcept { return layer_; }
Layer& Device::layer() noexcept { return layer_; }
void Device::setLayer(Layer value) { layer_ = std::move(value); }

const Polygon2D& Device::film() const noexcept { return film_; }
Polygon2D& Device::film() noexcept { return film_; }
void Device::setFilm(Polygon2D value) { film_ = std::move(value); }

const SolverOptions& Device::solverOptions() const noexcept { return solverOptions_; }
SolverOptions& Device::solverOptions() noexcept { return solverOptions_; }
void Device::setSolverOptions(SolverOptions value) { solverOptions_ = std::move(value); }

const std::vector<Polygon2D>& Device::holes() const noexcept { return holes_; }
const std::vector<Polygon2D>& Device::terminals() const noexcept { return terminals_; }
const std::vector<Point2D>& Device::probePoints() const noexcept { return probePoints_; }

void Device::addHole(Polygon2D hole) {
    holes_.push_back(std::move(hole));
}

void Device::addTerminal(Polygon2D terminal) {
    terminals_.push_back(std::move(terminal));
}

void Device::addProbePoint(Point2D point) {
    probePoints_.push_back(point);
}

bool Device::hasMesh() const noexcept {
    return mesh_.has_value();
}

const Mesh& Device::mesh() const {
    if (!mesh_.has_value()) {
        throw std::logic_error("Device does not have an attached mesh.");
    }

    return *mesh_;
}

Mesh& Device::mesh() {
    if (!mesh_.has_value()) {
        throw std::logic_error("Device does not have an attached mesh.");
    }

    return *mesh_;
}

void Device::setMesh(Mesh value) {
    mesh_ = std::move(value);
}

void Device::clearMesh() {
    mesh_.reset();
}

bool Device::hasValidGeometry() const {
    if (!film_.isValid()) {
        return false;
    }

    for (const Polygon2D& hole : holes_) {
        if (!hole.isValid()) {
            return false;
        }
    }

    for (const Polygon2D& terminal : terminals_) {
        if (!terminal.isValid()) {
            return false;
        }
    }

    if (mesh_.has_value() && !mesh_->isValid()) {
        return false;
    }

    return true;
}

bool Device::isValid() const {
    return validationErrors().empty();
}

std::vector<std::string> Device::validationErrors() const {
    std::vector<std::string> errors;

    if (projectName_.empty()) {
        errors.emplace_back("Device project name must not be empty.");
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

    for (std::size_t i = 0; i < probePoints_.size(); ++i) {
        const Point2D point = probePoints_[i];
        if (!film_.containsPoint(point)) {
            errors.emplace_back("Probe point " + std::to_string(i) + " must be inside the film polygon.");
        }
        if (pointInsideAnyPolygon(point, holes_)) {
            errors.emplace_back("Probe point " + std::to_string(i) + " must not be inside a hole polygon.");
        }
    }

    if (mesh_.has_value()) {
        appendErrors(errors, "Mesh: ", mesh_->validationErrors());
    }

    return errors;
}

} // namespace cppTDGL
