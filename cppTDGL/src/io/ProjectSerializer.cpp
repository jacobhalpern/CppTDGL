#include "ProjectSerializer.hpp"

#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace cppTDGL {
namespace {

constexpr int kProjectFileVersion = 1;

[[nodiscard]] std::runtime_error parseError(const std::string& message) {
    return std::runtime_error("CppTDGL project parse error: " + message);
}

void expectToken(std::istream& input, const std::string& expected) {
    std::string actual;
    if (!(input >> actual)) {
        throw parseError("Expected token '" + expected + "', but reached end of file.");
    }

    if (actual != expected) {
        throw parseError("Expected token '" + expected + "', but found '" + actual + "'.");
    }
}

void writePolygon(std::ostream& output, const std::string& token, const Polygon2D& polygon) {
    output << token << ' ' << std::quoted(polygon.name()) << ' ' << polygon.vertices().size() << '\n';

    for (const Point2D& point : polygon.vertices()) {
        output << "POINT " << point.x << ' ' << point.y << '\n';
    }

    output << "END_POLYGON\n";
}

[[nodiscard]] Polygon2D readPolygon(std::istream& input, const std::string& expectedToken) {
    expectToken(input, expectedToken);

    std::string name;
    std::size_t vertexCount = 0;

    if (!(input >> std::quoted(name) >> vertexCount)) {
        throw parseError("Failed to read polygon header for '" + expectedToken + "'.");
    }

    std::vector<Point2D> vertices;
    vertices.reserve(vertexCount);

    for (std::size_t i = 0; i < vertexCount; ++i) {
        expectToken(input, "POINT");

        Point2D point{};
        if (!(input >> point.x >> point.y)) {
            throw parseError("Failed to read polygon point " + std::to_string(i) + ".");
        }

        vertices.push_back(point);
    }

    expectToken(input, "END_POLYGON");

    return Polygon2D(std::move(name), std::move(vertices));
}

} // namespace

void ProjectSerializer::save(const Device& device, const std::filesystem::path& path) {
    std::ofstream output(path);

    if (!output) {
        throw std::runtime_error("Failed to open project file for writing: " + path.string());
    }

    output << std::setprecision(17);

    output << "CPPTDGL_PROJECT_VERSION " << kProjectFileVersion << '\n';
    output << "PROJECT_NAME " << std::quoted(device.projectName()) << '\n';

    const Layer& layer = device.layer();
    output << "LAYER "
           << layer.coherenceLength() << ' '
           << layer.londonLambda() << ' '
           << layer.thickness() << ' '
           << layer.conductivity() << ' '
           << layer.gamma() << ' '
           << std::quoted(layer.units()) << '\n';

    const SolverOptions& solver = device.solverOptions();
    output << "SOLVER "
           << solver.solveTime << ' '
           << solver.initialTimeStep << ' '
           << solver.maximumTimeStep << ' '
           << (solver.adaptiveTimeStepping ? 1 : 0) << ' '
           << solver.saveEvery << ' '
           << (solver.includeScreening ? 1 : 0) << ' '
           << std::quoted(solver.outputPath) << ' '
           << std::quoted(solver.sparseSolverName) << '\n';

    writePolygon(output, "FILM", device.film());

    output << "HOLES " << device.holes().size() << '\n';
    for (const Polygon2D& hole : device.holes()) {
        writePolygon(output, "HOLE", hole);
    }

    output << "TERMINALS " << device.terminals().size() << '\n';
    for (const Polygon2D& terminal : device.terminals()) {
        writePolygon(output, "TERMINAL", terminal);
    }

    output << "PROBE_POINTS " << device.probePoints().size() << '\n';
    for (const Point2D& point : device.probePoints()) {
        output << "PROBE " << point.x << ' ' << point.y << '\n';
    }

    output << "END_PROJECT\n";

    if (!output) {
        throw std::runtime_error("Failed while writing project file: " + path.string());
    }
}

Device ProjectSerializer::load(const std::filesystem::path& path) {
    std::ifstream input(path);

    if (!input) {
        throw std::runtime_error("Failed to open project file for reading: " + path.string());
    }

    expectToken(input, "CPPTDGL_PROJECT_VERSION");

    int version = 0;
    if (!(input >> version)) {
        throw parseError("Failed to read project file version.");
    }

    if (version != kProjectFileVersion) {
        throw parseError("Unsupported project file version: " + std::to_string(version) + ".");
    }

    expectToken(input, "PROJECT_NAME");

    std::string projectName;
    if (!(input >> std::quoted(projectName))) {
        throw parseError("Failed to read project name.");
    }

    expectToken(input, "LAYER");

    double coherenceLength = 0.0;
    double londonLambda = 0.0;
    double thickness = 0.0;
    double conductivity = 0.0;
    double gamma = 0.0;
    std::string units;

    if (!(input >> coherenceLength >> londonLambda >> thickness >> conductivity >> gamma >> std::quoted(units))) {
        throw parseError("Failed to read layer values.");
    }

    Layer layer(coherenceLength, londonLambda, thickness, conductivity, gamma, units);

    expectToken(input, "SOLVER");

    SolverOptions solver;
    int adaptiveTimeStepping = 0;
    int includeScreening = 0;

    if (!(input >> solver.solveTime
                >> solver.initialTimeStep
                >> solver.maximumTimeStep
                >> adaptiveTimeStepping
                >> solver.saveEvery
                >> includeScreening
                >> std::quoted(solver.outputPath)
                >> std::quoted(solver.sparseSolverName))) {
        throw parseError("Failed to read solver options.");
    }

    solver.adaptiveTimeStepping = adaptiveTimeStepping != 0;
    solver.includeScreening = includeScreening != 0;

    Polygon2D film = readPolygon(input, "FILM");

    Device device(projectName, layer, film, solver);

    expectToken(input, "HOLES");

    std::size_t holeCount = 0;
    if (!(input >> holeCount)) {
        throw parseError("Failed to read hole count.");
    }

    for (std::size_t i = 0; i < holeCount; ++i) {
        device.addHole(readPolygon(input, "HOLE"));
    }

    expectToken(input, "TERMINALS");

    std::size_t terminalCount = 0;
    if (!(input >> terminalCount)) {
        throw parseError("Failed to read terminal count.");
    }

    for (std::size_t i = 0; i < terminalCount; ++i) {
        device.addTerminal(readPolygon(input, "TERMINAL"));
    }

    expectToken(input, "PROBE_POINTS");

    std::size_t probeCount = 0;
    if (!(input >> probeCount)) {
        throw parseError("Failed to read probe point count.");
    }

    for (std::size_t i = 0; i < probeCount; ++i) {
        expectToken(input, "PROBE");

        Point2D point{};
        if (!(input >> point.x >> point.y)) {
            throw parseError("Failed to read probe point " + std::to_string(i) + ".");
        }

        device.addProbePoint(point);
    }

    expectToken(input, "END_PROJECT");

    return device;
}

} // namespace cppTDGL
