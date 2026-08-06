#include "ApplicationInfo.hpp"

namespace cppTDGL {

std::string ApplicationInfo::applicationName() {
    return "CppTDGL";
}

std::string ApplicationInfo::organizationName() {
    return "CppTDGL Project";
}

std::string ApplicationInfo::defaultWindowTitle() {
    return applicationName() + " - TDGL Simulation Workbench";
}

std::string ApplicationInfo::aboutText() {
    return "CppTDGL Qt application shell for creating, running, and inspecting TDGL simulation projects.";
}

std::string ApplicationInfo::projectFileFilter() {
    return "CppTDGL Project Files (*.cpptdgl)";
}

std::string ApplicationInfo::solutionFileFilter() {
    return "CppTDGL Solution Files (*.h5)";
}

} // namespace cppTDGL
