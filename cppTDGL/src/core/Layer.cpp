#include "Layer.hpp"

#include <utility>

namespace cppTDGL {

Layer::Layer(
    double coherenceLength,
    double londonLambda,
    double thickness,
    double conductivity,
    double gamma,
    std::string units
)
    : coherenceLength_(coherenceLength),
      londonLambda_(londonLambda),
      thickness_(thickness),
      conductivity_(conductivity),
      gamma_(gamma),
      units_(std::move(units)) {}

double Layer::coherenceLength() const noexcept { return coherenceLength_; }
double Layer::londonLambda() const noexcept { return londonLambda_; }
double Layer::thickness() const noexcept { return thickness_; }
double Layer::conductivity() const noexcept { return conductivity_; }
double Layer::gamma() const noexcept { return gamma_; }
const std::string& Layer::units() const noexcept { return units_; }

void Layer::setCoherenceLength(double value) { coherenceLength_ = value; }
void Layer::setLondonLambda(double value) { londonLambda_ = value; }
void Layer::setThickness(double value) { thickness_ = value; }
void Layer::setConductivity(double value) { conductivity_ = value; }
void Layer::setGamma(double value) { gamma_ = value; }
void Layer::setUnits(std::string value) { units_ = std::move(value); }

bool Layer::isValid() const {
    return validationErrors().empty();
}

std::vector<std::string> Layer::validationErrors() const {
    std::vector<std::string> errors;

    if (coherenceLength_ <= 0.0) {
        errors.emplace_back("Layer coherence length must be greater than zero.");
    }
    if (londonLambda_ <= 0.0) {
        errors.emplace_back("Layer London penetration depth must be greater than zero.");
    }
    if (thickness_ <= 0.0) {
        errors.emplace_back("Layer thickness must be greater than zero.");
    }
    if (conductivity_ < 0.0) {
        errors.emplace_back("Layer conductivity must be non-negative.");
    }
    if (gamma_ <= 0.0) {
        errors.emplace_back("Layer gamma must be greater than zero.");
    }
    if (units_.empty()) {
        errors.emplace_back("Layer units string must not be empty.");
    }

    return errors;
}

} // namespace cppTDGL
