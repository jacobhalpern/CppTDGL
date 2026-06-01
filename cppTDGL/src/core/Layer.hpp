#pragma once

#include <string>
#include <vector>

namespace cppTDGL {

class Layer {
public:
    Layer() = default;

    Layer(
        double coherenceLength,
        double londonLambda,
        double thickness,
        double conductivity,
        double gamma,
        std::string units = "um"
    );

    [[nodiscard]] double coherenceLength() const noexcept;
    [[nodiscard]] double londonLambda() const noexcept;
    [[nodiscard]] double thickness() const noexcept;
    [[nodiscard]] double conductivity() const noexcept;
    [[nodiscard]] double gamma() const noexcept;
    [[nodiscard]] const std::string& units() const noexcept;

    void setCoherenceLength(double value);
    void setLondonLambda(double value);
    void setThickness(double value);
    void setConductivity(double value);
    void setGamma(double value);
    void setUnits(std::string value);

    [[nodiscard]] bool isValid() const;
    [[nodiscard]] std::vector<std::string> validationErrors() const;

private:
    double coherenceLength_ = 1.0;
    double londonLambda_ = 1.0;
    double thickness_ = 0.1;
    double conductivity_ = 1.0;
    double gamma_ = 1.0;
    std::string units_ = "um";
};

} // namespace cppTDGL
