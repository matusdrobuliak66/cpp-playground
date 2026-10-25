#pragma once
#include <vector>
#include <iostream>

struct Matrix {
    int rows;
    int cols;
    std::vector<double> data;   // heap-backed storage

    Matrix(int r, int c) : rows(r), cols(c), data(r * c, 0.0) {}

    double& at(int r, int c)             { return data[r * cols + c]; }
    double  at(int r, int c) const       { return data[r * cols + c]; }

    void print() const;
};

Matrix operator+(const Matrix& a, const Matrix& b);
