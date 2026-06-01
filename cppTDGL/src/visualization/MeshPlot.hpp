#pragma once

#include <string>

namespace cppTDGL {

class MeshPlot {
public:
    MeshPlot() = default;
    virtual ~MeshPlot() = default;

    [[nodiscard]] std::string className() const;
};

} // namespace cppTDGL
