#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace cppTDGL {

struct SparseEntry {
    std::size_t row = 0;
    std::size_t column = 0;
    double value = 0.0;
};

class SparseMatrix {
public:
    SparseMatrix() = default;
    SparseMatrix(std::size_t rowCount, std::size_t columnCount);

    [[nodiscard]] std::size_t rowCount() const noexcept;
    [[nodiscard]] std::size_t columnCount() const noexcept;
    [[nodiscard]] const std::vector<SparseEntry>& entries() const noexcept;

    void resize(std::size_t rowCount, std::size_t columnCount);
    void clearEntries() noexcept;
    void addEntry(std::size_t row, std::size_t column, double value);

    [[nodiscard]] double valueAt(std::size_t row, std::size_t column) const;
    [[nodiscard]] std::vector<double> multiply(const std::vector<double>& vector) const;
    [[nodiscard]] SparseMatrix transpose() const;

    [[nodiscard]] bool isValid() const;
    [[nodiscard]] std::vector<std::string> validationErrors() const;

private:
    std::size_t rowCount_ = 0;
    std::size_t columnCount_ = 0;
    std::vector<SparseEntry> entries_;
};

} // namespace cppTDGL
