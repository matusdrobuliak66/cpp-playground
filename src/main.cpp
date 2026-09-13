#include <iostream>
#include "mathutils.h"

int main() {
    std::cout << add(2, 3) << "\n";
    std::cout << multiply(4, 5) << "\n";

    int data[] = {1, 2, 3, 4};
    std::cout << average(data, 4) << "\n";
}