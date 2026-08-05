#include "Hdf5SolutionWriter.hpp"

#include "SerializationFormat.hpp"

#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <string>
#include <vector>

namespace cppTDGL {
namespace {

void writeMesh(std::ostream& output, const Mesh& mesh) {
    output << "MESH\n";

    output << "VERTICES " << mesh.vertices().size() << '\n';
    for (const Point2D& point : mesh.vertices()) {
        output << "VERTEX " << point.x << ' ' << point.y << '\n';
    }

    output << "TRIANGLES " << mesh.triangles().size() << '\n';
    for (const Triangle& triangle : mesh.triangles()) {
        output << "TRIANGLE "
               << triangle.a << ' '
               << triangle.b << ' '
               << triangle.c << '\n';
    }

    output << "BOUNDARY_VERTICES " << mesh.boundaryVertexIndices().size() << '\n';
    for (const std::size_t index : mesh.boundaryVertexIndices()) {
        output << "BOUNDARY_VERTEX " << index << '\n';
    }

    output << "END_MESH\n";
}

void writeDoubleVector(
    std::ostream& output,
    const std::string& token,
    const std::vector<double>& values
) {
    output << token << ' ' << values.size() << '\n';

    for (const double value : values) {
        output << "VALUE " << value << '\n';
    }
}

void writeFrame(std::ostream& output, const SolverFrame& frame, std::size_t frameIndex) {
    output << "FRAME " << frameIndex << ' ' << frame.time << '\n';

    output << "ORDER_PARAMETER " << frame.orderParameter.size() << '\n';
    for (std::size_t i = 0; i < frame.orderParameter.size(); ++i) {
        output << "COMPLEX "
               << frame.orderParameter.real[i] << ' '
               << frame.orderParameter.imaginary[i] << '\n';
    }

    writeDoubleVector(output, "SCALAR_POTENTIAL", frame.scalarPotential.values);
    writeDoubleVector(output, "VECTOR_POTENTIAL", frame.vectorPotential.values);
    writeDoubleVector(output, "CURRENT_DENSITY", frame.currentDensity.values);

    output << "END_FRAME\n";
}

} // namespace

std::string Hdf5SolutionWriter::className() const {
    return "Hdf5SolutionWriter";
}

void Hdf5SolutionWriter::write(const Solution& solution) {
    if (!solution.hasResultPath()) {
        throw std::logic_error("Cannot write Solution without a result path.");
    }

    write(solution, solution.resultPath());
}

void Hdf5SolutionWriter::write(
    const Solution& solution,
    const std::filesystem::path& path
) {
    if (path.empty()) {
        throw std::invalid_argument("Solution output path must not be empty.");
    }

    const std::vector<std::string> validationErrors = solution.validationErrors();
    if (!validationErrors.empty()) {
        throw std::invalid_argument("Cannot write invalid Solution: " + validationErrors.front());
    }

    std::ofstream output(path);

    if (!output) {
        throw std::runtime_error("Failed to open solution file for writing: " + path.string());
    }

    output << std::setprecision(17);

    output << SerializationFormat::solutionScaffoldFileSignature
           << ' '
           << SerializationFormat::latestSolutionScaffoldFileVersion
           << '\n';

    output << "PROJECT_NAME " << std::quoted(solution.projectName()) << '\n';
    output << "RESULT_PATH " << std::quoted(solution.resultPath().string()) << '\n';
    output << "COMPLETED " << (solution.completed() ? 1 : 0) << '\n';

    output << "DIAGNOSTICS " << solution.diagnostics().size() << '\n';
    for (const std::string& diagnostic : solution.diagnostics()) {
        output << "DIAGNOSTIC " << std::quoted(diagnostic) << '\n';
    }

    output << "MESH_PRESENT " << (solution.hasMesh() ? 1 : 0) << '\n';
    if (solution.hasMesh()) {
        writeMesh(output, solution.mesh());
    }

    output << "FRAMES " << solution.frames().size() << '\n';
    for (std::size_t i = 0; i < solution.frames().size(); ++i) {
        writeFrame(output, solution.frames()[i], i);
    }

    output << "END_SOLUTION\n";

    if (!output) {
        throw std::runtime_error("Failed while writing solution file: " + path.string());
    }
}

} // namespace cppTDGL
