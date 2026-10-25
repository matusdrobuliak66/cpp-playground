#include "converter.h"
#include <stdexcept>

double convert(double value, const std::string& unit) {
    if (unit == "c-to-f")  return value * 9.0 / 5.0 + 32.0;
    if (unit == "f-to-c")  return (value - 32.0) * 5.0 / 9.0;
    if (unit == "km-to-mi") return value * 0.621371;
    if (unit == "mi-to-km") return value / 0.621371;
    if (unit == "kg-to-lb") return value * 2.20462;
    if (unit == "lb-to-kg") return value / 2.20462;
    throw std::invalid_argument("unknown unit: " + unit);
}