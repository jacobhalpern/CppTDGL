#include "Device.hpp"
#include "Layer.hpp"
#include "Polygon2D.hpp"
#include "SolverOptions.hpp"

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void failIfFalse(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void printErrors(const std::vector<std::string>& errors) {
    for (const std::string& error : errors) {
        std::cerr << "  - " << error << '\n';
    }
}

} // namespace

int main() {
    try {
        cppTDGL::Layer layer(
            0.5,  // coherence length
            2.0,  // London penetration depth
            0.1,  // thickness
            1.0,  // conductivity
            1.0,  // gamma
            "um"
        );

        cppTDGL::SolverOptions options;
        options.solveTime = 10.0;
        options.initialTimeStep = 1.0e-3;
        options.maximumTimeStep = 0.05;
        options.saveEvery = 10;
        options.outputPath = "smoke_test_solution.h5";

        cppTDGL::Polygon2D film = cppTDGL::Polygon2D::rectangle(
            "film",
            10.0,
            4.0
        );

        cppTDGL::Device device(
            "core_model_smoke_test",
            layer,
            film,
            options
        );

        device.addHole(cppTDGL::Polygon2D::circleApproximation(
            "center_hole",
            0.5,
            32,
            {0.0, 0.0}
        ));

        device.addTerminal(cppTDGL::Polygon2D::rectangle(
            "left_terminal",
            0.5,
            1.5,
            {-4.75, 0.0}
        ));

        device.addProbePoint({0.0, 1.5});

        failIfFalse(layer.isValid(), "Layer should be valid.");
        failIfFalse(options.isValid(), "SolverOptions should be valid.");
        failIfFalse(film.isValid(), "Film polygon should be valid.");
        failIfFalse(film.area() == 40.0, "Film area should be exactly 40.0 for a 10 x 4 rectangle.");
        failIfFalse(film.containsPoint({0.0, 0.0}), "Film should contain the origin.");
        failIfFalse(!film.containsPoint({100.0, 100.0}), "Film should not contain a distant point.");
        failIfFalse(device.hasValidGeometry(), "Device geometry should be valid.");

        const std::vector<std::string> errors = device.validationErrors();
        if (!errors.empty()) {
            std::cerr << "Device validation failed:\n";
            printErrors(errors);
            return 1;
        }

        std::cout << "Core model smoke test passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Core model smoke test exception: " << error.what() << '\n';
        return 1;
    }
}
