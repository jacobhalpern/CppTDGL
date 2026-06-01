#include "Polygon2D.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numbers>
#include <stdexcept>
#include <utility>

namespace cppTDGL {
namespace {

constexpr double kAreaTolerance = 1.0e-12;

} // namespace

double BoundingBox::width() const noexcept {
    return maximum.x - minimum.x;
}

double BoundingBox::height() const noexcept {
    return maximum.y - minimum.y;
}

bool BoundingBox::isValid() const noexcept {
    return maximum.x >= minimum.x && maximum.y >= minimum.y;
}

Polygon2D::Polygon2D(std::string name, std::vector<Point2D> vertices)
    : name_(std::move(name)), vertices_(std::move(vertices)) {}

Polygon2D Polygon2D::rectangle(
    std::string name,
    double width,
    double height,
    Point2D center
) {
    if (width <= 0.0) {
        throw std::invalid_argument("Rectangle width must be greater than zero.");
    }
    if (height <= 0.0) {
        throw std::invalid_argument("Rectangle height must be greater than zero.");
    }

    const double halfWidth = width / 2.0;
    const double halfHeight = height / 2.0;

    return Polygon2D(
        std::move(name),
        {
            {center.x - halfWidth, center.y - halfHeight},
            {center.x + halfWidth, center.y - halfHeight},
            {center.x + halfWidth, center.y + halfHeight},
            {center.x - halfWidth, center.y + halfHeight},
        }
    );
}

Polygon2D Polygon2D::circleApproximation(
    std::string name,
    double radius,
    std::size_t segmentCount,
    Point2D center
) {
    if (radius <= 0.0) {
        throw std::invalid_argument("Circle radius must be greater than zero.");
    }
    if (segmentCount < 8) {
        throw std::invalid_argument("Circle approximation requires at least 8 segments.");
    }

    std::vector<Point2D> points;
    points.reserve(segmentCount);

    for (std::size_t i = 0; i < segmentCount; ++i) {
        const double angle = 2.0 * std::numbers::pi * static_cast<double>(i) /
                             static_cast<double>(segmentCount);
        points.push_back({
            center.x + radius * std::cos(angle),
            center.y + radius * std::sin(angle),
        });
    }

    return Polygon2D(std::move(name), std::move(points));
}

const std::string& Polygon2D::name() const noexcept {
    return name_;
}

void Polygon2D::setName(std::string value) {
    name_ = std::move(value);
}

const std::vector<Point2D>& Polygon2D::vertices() const noexcept {
    return vertices_;
}

void Polygon2D::setVertices(std::vector<Point2D> value) {
    vertices_ = std::move(value);
}

void Polygon2D::addVertex(Point2D point) {
    vertices_.push_back(point);
}

std::size_t Polygon2D::vertexCount() const noexcept {
    return vertices_.size();
}

bool Polygon2D::isValid() const {
    return validationErrors().empty();
}

std::vector<std::string> Polygon2D::validationErrors() const {
    std::vector<std::string> errors;

    if (name_.empty()) {
        errors.emplace_back("Polygon name must not be empty.");
    }
    if (vertices_.size() < 3) {
        errors.emplace_back("Polygon must contain at least three vertices.");
    }
    if (std::abs(signedArea()) <= kAreaTolerance) {
        errors.emplace_back("Polygon area must be non-zero.");
    }

    return errors;
}

double Polygon2D::signedArea() const noexcept {
    if (vertices_.size() < 3) {
        return 0.0;
    }

    double sum = 0.0;
    for (std::size_t i = 0; i < vertices_.size(); ++i) {
        const Point2D& a = vertices_[i];
        const Point2D& b = vertices_[(i + 1) % vertices_.size()];
        sum += a.x * b.y - b.x * a.y;
    }

    return 0.5 * sum;
}

double Polygon2D::area() const noexcept {
    return std::abs(signedArea());
}

BoundingBox Polygon2D::boundingBox() const {
    if (vertices_.empty()) {
        throw std::logic_error("Cannot compute a bounding box for an empty polygon.");
    }

    BoundingBox box{
        {std::numeric_limits<double>::max(), std::numeric_limits<double>::max()},
        {std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest()},
    };

    for (const Point2D& point : vertices_) {
        box.minimum.x = std::min(box.minimum.x, point.x);
        box.minimum.y = std::min(box.minimum.y, point.y);
        box.maximum.x = std::max(box.maximum.x, point.x);
        box.maximum.y = std::max(box.maximum.y, point.y);
    }

    return box;
}

bool Polygon2D::containsPoint(Point2D point) const noexcept {
    if (vertices_.size() < 3) {
        return false;
    }

    bool inside = false;
    std::size_t j = vertices_.size() - 1;

    for (std::size_t i = 0; i < vertices_.size(); ++i) {
        const Point2D& pi = vertices_[i];
        const Point2D& pj = vertices_[j];

        const bool edgeStraddlesY = (pi.y > point.y) != (pj.y > point.y);
        if (edgeStraddlesY) {
            const double xIntersection =
                (pj.x - pi.x) * (point.y - pi.y) / (pj.y - pi.y) + pi.x;
            if (point.x < xIntersection) {
                inside = !inside;
            }
        }

        j = i;
    }

    return inside;
}

void Polygon2D::translate(double dx, double dy) {
    for (Point2D& point : vertices_) {
        point.x += dx;
        point.y += dy;
    }
}

void Polygon2D::scale(double sx, double sy, Point2D origin) {
    for (Point2D& point : vertices_) {
        point.x = origin.x + (point.x - origin.x) * sx;
        point.y = origin.y + (point.y - origin.y) * sy;
    }
}

void Polygon2D::rotate(double radians, Point2D origin) {
    const double c = std::cos(radians);
    const double s = std::sin(radians);

    for (Point2D& point : vertices_) {
        const double x = point.x - origin.x;
        const double y = point.y - origin.y;
        point.x = origin.x + x * c - y * s;
        point.y = origin.y + x * s + y * c;
    }
}

} // namespace cppTDGL
