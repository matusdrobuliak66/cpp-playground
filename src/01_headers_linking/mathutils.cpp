#include "mathutils.h"   // quotes = my project; angle brackets = system/library

int add(int a, int b) {
    return a + b;
}

int multiply(int a, int b) {
    return a * b;
}

double average(const int* values, int count) {
    if (count == 0) return 0.0;
    int sum = 0;
    for (int i = 0; i < count; ++i) sum += values[i];
    return static_cast<double>(sum) / count;
}