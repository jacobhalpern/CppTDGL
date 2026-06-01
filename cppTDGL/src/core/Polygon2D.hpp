#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace cppTDGL {

struct Point2D {
    double x = 0.0;
    double y = 0.0;
};

struct BoundingBox {
    Point2D minimum{};
    Point2D maximum{};

    [[nodiscard]] double width() const noexcept;
    [[nodiscard]] double height() const noexcept;
    [[nodiscard]] bool isValid() const noexcept;
};

class Polygon2D {
public:
    Polygon2D() = default;
    Polygon2D(std::string name, std::vector<Point2D> vertices);

    [[nodiscard]] static Polygon2D rectangle(
        std::string name,
        double width,
        double height,
        Point2D center = {}
    );

    [[nodiscard]] static Polygon2D circleApproximation(
        std::string name,
        double radius,
        std::size_t segmentCount = 64,
        Point2D center = {}
    );

    [[nodiscard]] const std::string& name() const noexcept;
    void setName(std::string value);

    [[nodiscard]] const std::vector<Point2D>& vertices() const noexcept;
    void setVertices(std::vector<Point2D> value);
    void addVertex(Point2D point);

    [[nodiscard]] std::size_t vertexCount() const noexcept;
    [[nodiscard]] bool isValid() const;
    [[nodiscard]] std::vector<std::string> validationErrors() const;

    [[nodiscard]] double signedArea() const noexcept;
    [[nodiscard]] double area() const noexcept;
    [[nodiscard]] BoundingBox boundingBox() const;
    [[nodiscard]] bool containsPoint(Point2D point) const noexcept;

    void translate(double dx, double dy);
    void scale(double sx, double sy, Point2D origin = {});
    void rotate(double radians, Point2D origin = {});

private:
    std::string name_;
    std::vector<Point2D> vertices_;
};

} // namespace cppTDGL
