#include "FieldPlot.hpp"

#include <algorithm>
#include <cmath>
#include <string>
#include <utility>
#include <vector>

namespace cppTDGL {

namespace {

[[nodiscard]] std::vector<double> magnitudes(const VertexComplexField& field) {
    const std::size_t count = std::min(field.real.size(), field.imaginary.size());
    std::vector<double> values;
    values.reserve(count);

    for (std::size_t i = 0; i < count; ++i) {
        values.push_back(std::hypot(field.real[i], field.imaginary[i]));
    }

    return values;
}

[[nodiscard]] FieldPlotData fromValues(
    const Mesh& mesh,
    const std::vector<double>& values,
    std::string fieldName,
    std::vector<std::string> diagnostics
) {
    FieldPlotData data;
    data.fieldName = std::move(fieldName);
    data.diagnostics = std::move(diagnostics);

    const std::vector<std::string> meshErrors = mesh.validationErrors();
    data.diagnostics.insert(data.diagnostics.end(), meshErrors.begin(), meshErrors.end());

    if (!mesh.isValid()) {
        return data;
    }

    if (values.size() != mesh.vertexCount()) {
        data.diagnostics.push_back(
            "Field value count must equal mesh vertex count. Expected " +
            std::to_string(mesh.vertexCount()) + ", got " +
            std::to_string(values.size()) + "."
        );
        return data;
    }

    data.boundingBox = mesh.boundingBox();
    data.range = FieldPlot::valueRange(values);
    data.samples.reserve(values.size());

    for (std::size_t i = 0; i < values.size(); ++i) {
        data.samples.push_back({i, mesh.vertices()[i], values[i]});
    }

    return data;
}

} // namespace

double PlotValueRange::span() const noexcept {
    if (!hasValues) {
        return 0.0;
    }

    return maximum - minimum;
}

bool FieldPlotData::hasSamples() const noexcept {
    return !samples.empty();
}

bool FieldPlotData::isValid() const noexcept {
    return diagnostics.empty() && hasSamples() && range.hasValues;
}

std::string FieldPlot::className() const {
    return "FieldPlot";
}

FieldPlotData FieldPlot::fromVertexScalarField(
    const Mesh& mesh,
    const VertexScalarField& field,
    std::string fieldName
) {
    return fromValues(mesh, field.values, std::move(fieldName), field.validationErrors(mesh, "FieldPlot vertex field"));
}

FieldPlotData FieldPlot::orderParameterMagnitude(
    const Mesh& mesh,
    const VertexComplexField& field,
    std::string fieldName
) {
    return fromValues(mesh, magnitudes(field), std::move(fieldName), field.validationErrors(mesh, "FieldPlot order parameter"));
}

PlotValueRange FieldPlot::valueRange(const std::vector<double>& values) {
    PlotValueRange range;

    if (values.empty()) {
        return range;
    }

    const auto [minimum, maximum] = std::minmax_element(values.begin(), values.end());
    range.minimum = *minimum;
    range.maximum = *maximum;
    range.hasValues = true;
    return range;
}

} // namespace cppTDGL
