#include "ApplicationInfo.hpp"

#include <iostream>
#include <stdexcept>
#include <string>

namespace {

void failIfFalse(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

} // namespace

int main() {
    try {
        const std::string applicationName = cppTDGL::ApplicationInfo::applicationName();
        const std::string organizationName = cppTDGL::ApplicationInfo::organizationName();
        const std::string windowTitle = cppTDGL::ApplicationInfo::defaultWindowTitle();
        const std::string aboutText = cppTDGL::ApplicationInfo::aboutText();
        const std::string projectFilter = cppTDGL::ApplicationInfo::projectFileFilter();
        const std::string solutionFilter = cppTDGL::ApplicationInfo::solutionFileFilter();

        failIfFalse(!applicationName.empty(), "Application name should not be empty.");
        failIfFalse(applicationName == "CppTDGL", "Application name should be CppTDGL.");
        failIfFalse(!organizationName.empty(), "Organization name should not be empty.");
        failIfFalse(windowTitle.find(applicationName) != std::string::npos, "Window title should contain application name.");
        failIfFalse(aboutText.find("Qt application shell") != std::string::npos, "About text should describe the Qt shell.");
        failIfFalse(projectFilter.find("*.cpptdgl") != std::string::npos, "Project file filter should include .cpptdgl.");
        failIfFalse(solutionFilter.find("*.h5") != std::string::npos, "Solution file filter should include .h5.");

        std::cout << "Application info smoke test passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Application info smoke test exception: " << error.what() << '\n';
        return 1;
    }
}
