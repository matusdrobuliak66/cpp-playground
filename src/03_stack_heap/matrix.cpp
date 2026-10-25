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

Matrix operator*(const Matrix& a, const Matrix& b) {
    if (a.cols != b.rows)
        throw std::invalid_argument("dimension mismatch");

    Matrix result(a.rows, b.cols);
    for (int r = 0; r < a.rows; ++r) {
        for (int c = 0; c < b.cols; ++c) {
            result.at(r, c) = 0;
            for (int k = 0; k < a.cols; ++k)
                result.at(r, c) += a.at(r, k) * b.at(k, c);
        }
    }
    return result;
}