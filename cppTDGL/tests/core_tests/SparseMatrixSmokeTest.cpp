#include "SparseMatrix.hpp"

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

constexpr double kTolerance = 1.0e-12;

void failIfFalse(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void failIfNotNear(double actual, double expected, const std::string& message) {
    if (std::abs(actual - expected) > kTolerance) {
        throw std::runtime_error(
            message + " Expected " + std::to_string(expected) +
            ", got " + std::to_string(actual) + "."
        );
    }
}

} // namespace

int main() {
    try {
        cppTDGL::SparseMatrix matrix(3, 3);

        matrix.addEntry(0, 0, 2.0);
        matrix.addEntry(1, 1, 3.0);
        matrix.addEntry(2, 2, 4.0);
        matrix.addEntry(0, 2, 5.0);

        failIfFalse(matrix.isValid(), "Sparse matrix should be valid.");
        failIfFalse(matrix.rowCount() == 3, "Sparse matrix should have 3 rows.");
        failIfFalse(matrix.columnCount() == 3, "Sparse matrix should have 3 columns.");
        failIfFalse(matrix.entries().size() == 4, "Sparse matrix should have 4 entries.");

        failIfNotNear(matrix.valueAt(0, 0), 2.0, "Matrix value (0, 0) should be 2.");
        failIfNotNear(matrix.valueAt(1, 1), 3.0, "Matrix value (1, 1) should be 3.");
        failIfNotNear(matrix.valueAt(2, 2), 4.0, "Matrix value (2, 2) should be 4.");
        failIfNotNear(matrix.valueAt(0, 2), 5.0, "Matrix value (0, 2) should be 5.");
        failIfNotNear(matrix.valueAt(1, 2), 0.0, "Missing sparse entry should read as 0.");

        const std::vector<double> vector{1.0, 2.0, 3.0};
        const std::vector<double> result = matrix.multiply(vector);

        failIfFalse(result.size() == 3, "Sparse matrix multiply result should have 3 rows.");
        failIfNotNear(result[0], 17.0, "Result row 0 should equal 2*1 + 5*3.");
        failIfNotNear(result[1], 6.0, "Result row 1 should equal 3*2.");
        failIfNotNear(result[2], 12.0, "Result row 2 should equal 4*3.");

        const cppTDGL::SparseMatrix transposed = matrix.transpose();

        failIfFalse(transposed.isValid(), "Transposed sparse matrix should be valid.");
        failIfFalse(transposed.rowCount() == 3, "Transposed matrix should have 3 rows.");
        failIfFalse(transposed.columnCount() == 3, "Transposed matrix should have 3 columns.");
        failIfFalse(transposed.entries().size() == 4, "Transposed matrix should preserve entry count.");

        failIfNotNear(transposed.valueAt(0, 0), 2.0, "Transposed value (0, 0) should be 2.");
        failIfNotNear(transposed.valueAt(1, 1), 3.0, "Transposed value (1, 1) should be 3.");
        failIfNotNear(transposed.valueAt(2, 2), 4.0, "Transposed value (2, 2) should be 4.");
        failIfNotNear(transposed.valueAt(2, 0), 5.0, "Transposed value (2, 0) should be 5.");

        cppTDGL::SparseMatrix duplicateEntryMatrix(2, 2);
        duplicateEntryMatrix.addEntry(0, 1, 2.0);
        duplicateEntryMatrix.addEntry(0, 1, 3.0);

        failIfNotNear(
            duplicateEntryMatrix.valueAt(0, 1),
            5.0,
            "Duplicate entries should be summed by valueAt."
        );

        const std::vector<double> duplicateResult =
            duplicateEntryMatrix.multiply({10.0, 4.0});

        failIfNotNear(
            duplicateResult[0],
            20.0,
            "Duplicate entries should both contribute during multiplication."
        );

        std::cout << "Sparse matrix smoke test passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Sparse matrix smoke test exception: " << error.what() << '\n';
        return 1;
    }
}
