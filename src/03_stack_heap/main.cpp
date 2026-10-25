#include <iostream>
#include "matrix.h"

int main() {
    std::cout << "03_stack_heap\n";

    Matrix a(2, 2);
    a.at(0, 0) = 1; a.at(0, 1) = 2;
    a.at(1, 0) = 3; a.at(1, 1) = 4;

    Matrix b(2, 2);
    b.at(0, 0) = 10; b.at(0, 1) = 20;
    b.at(1, 0) = 30; b.at(1, 1) = 40;

    Matrix c = a + b;
    c.print();
}
