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

    Matrix c = a * b;
    // c.print();

    std::cout << "orig:   " << &c << "\n";
    by_value(c);   // prints COPY, and a DIFFERENT address
    by_ref(c);     // no COPY, and the SAME address
}


// struct Noisy {
//     std::string name;
//     Noisy(std::string n) : name(std::move(n)) { std::cout << name << " born\n"; }
//     ~Noisy() { std::cout << name << " dies\n"; }
// };

// int main() {
//     Noisy a("a");
//     {
//         Noisy b("b");
//         std::cout << "-- inner scope end --\n";
//     }
//     std::cout << "-- main end --\n";
// }


// int main() {
//     double huge[2'000'000];   // ~16 MB on the stack
//     huge[0] = 1.0;
// }