#pragma once
#include <vector>
#include <iostream>

struct Matrix
{
    int rows;
    int cols;
    std::vector<double> data; // heap-backed storage

    Matrix(int r, int c) : rows(r), cols(c), data(r * c, 0.0) {}
    Matrix(const Matrix &other)
        : rows(other.rows), cols(other.cols), data(other.data)
    {
        std::cout << "COPY of " << rows << "x" << cols << "\n";
    }

    double &at(int r, int c) { return data[r * cols + c]; }
    double at(int r, int c) const { return data[r * cols + c]; }

    void print() const;
};

Matrix operator+(const Matrix &a, const Matrix &b);

Matrix operator*(const Matrix &a, const Matrix &b);


void by_value(Matrix m);
void by_ref(const Matrix& m);