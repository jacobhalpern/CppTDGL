#include "SparseMatrix.hpp"

#include <stdexcept>

namespace cppTDGL {

SparseMatrix::SparseMatrix(std::size_t rowCount, std::size_t columnCount)
    : rowCount_(rowCount), columnCount_(columnCount) {}

std::size_t SparseMatrix::rowCount() const noexcept {
    return rowCount_;
}

std::size_t SparseMatrix::columnCount() const noexcept {
    return columnCount_;
}

const std::vector<SparseEntry>& SparseMatrix::entries() const noexcept {
    return entries_;
}

void SparseMatrix::resize(std::size_t rowCount, std::size_t columnCount) {
    rowCount_ = rowCount;
    columnCount_ = columnCount;
    entries_.clear();
}

void SparseMatrix::clearEntries() noexcept {
    entries_.clear();
}

void SparseMatrix::addEntry(std::size_t row, std::size_t column, double value) {
    entries_.push_back({row, column, value});
}

double SparseMatrix::valueAt(std::size_t row, std::size_t column) const {
    if (row >= rowCount_ || column >= columnCount_) {
        throw std::out_of_range("SparseMatrix valueAt index is out of range.");
    }

    double value = 0.0;

    for (const SparseEntry& entry : entries_) {
        if (entry.row == row && entry.column == column) {
            value += entry.value;
        }
    }

    return value;
}

std::vector<double> SparseMatrix::multiply(const std::vector<double>& vector) const {
    const std::vector<std::string> errors = validationErrors();

    if (!errors.empty()) {
        throw std::logic_error("Cannot multiply with an invalid SparseMatrix.");
    }

    if (vector.size() != columnCount_) {
        throw std::invalid_argument("SparseMatrix multiply vector size must match matrix column count.");
    }

    std::vector<double> result(rowCount_, 0.0);

    for (const SparseEntry& entry : entries_) {
        result[entry.row] += entry.value * vector[entry.column];
    }

    return result;
}

SparseMatrix SparseMatrix::transpose() const {
    const std::vector<std::string> errors = validationErrors();

    if (!errors.empty()) {
        throw std::logic_error("Cannot transpose an invalid SparseMatrix.");
    }

    SparseMatrix transposed(columnCount_, rowCount_);

    for (const SparseEntry& entry : entries_) {
        transposed.addEntry(entry.column, entry.row, entry.value);
    }

    return transposed;
}

bool SparseMatrix::isValid() const {
    return validationErrors().empty();
}

std::vector<std::string> SparseMatrix::validationErrors() const {
    std::vector<std::string> errors;

    if (rowCount_ == 0) {
        errors.emplace_back("SparseMatrix row count must be greater than zero.");
    }

    if (columnCount_ == 0) {
        errors.emplace_back("SparseMatrix column count must be greater than zero.");
    }

    for (std::size_t i = 0; i < entries_.size(); ++i) {
        const SparseEntry& entry = entries_[i];

        if (entry.row >= rowCount_) {
            errors.emplace_back("SparseMatrix entry " + std::to_string(i) + " has a row index outside the matrix dimensions.");
        }

        if (entry.column >= columnCount_) {
            errors.emplace_back("SparseMatrix entry " + std::to_string(i) + " has a column index outside the matrix dimensions.");
        }
    }

    return errors;
}

} // namespace cppTDGL
