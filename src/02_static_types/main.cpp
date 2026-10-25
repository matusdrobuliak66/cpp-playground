#include <iostream>
#include <stdexcept>
#include <string>
#include "converter.h"

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "usage: convert <value> <unit>\n";
        return 1;
    }

    try {
        double value = std::stod(argv[1]);
        std::cout << convert(value, argv[2]) << "\n";
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
}