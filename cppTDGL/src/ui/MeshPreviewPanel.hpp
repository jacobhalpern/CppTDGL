#pragma once

#include <string>

namespace cppTDGL {

class MeshPreviewPanel {
public:
    MeshPreviewPanel() = default;
    virtual ~MeshPreviewPanel() = default;

    [[nodiscard]] std::string className() const;
};

} // namespace cppTDGL
