#include "matrix.h"
#include <stdexcept>

void Matrix::print() const {
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) std::cout << at(r, c) << ' ';
        std::cout << '\n';
    }
}

Matrix operator+(const Matrix& a, const Matrix& b) {
    if (a.rows != b.rows || a.cols != b.cols)
        throw std::invalid_argument("dimension mismatch");

    Matrix result(a.rows, a.cols);
    for (int i = 0; i < a.rows * a.cols; ++i)
        result.data[i] = a.data[i] + b.data[i];
    return result;
}
