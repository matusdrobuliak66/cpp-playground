---
title: "C++ 02 — Static Types"
tags: [cpp, learning, module, types]
module: 02
status: todo
up: "[[C++ MOC]]"
prev: "[[C++ 01 - Headers, Translation Units, Linking]]"
next: "[[C++ 03 - Stack and Heap]]"
---

# Module 02 — Static Types

> **Goal:** internalise that every variable has one fixed type decided at compile time,
> and learn the conversion traps that come with it.
> **Project:** a unit converter CLI, split across header and source (practising Module 01).
> **Time:** ~60 minutes.

## The concept

Python: a name is a label that can point at anything, and the *object* carries the type.
Rebinding to a different type is legal. Type errors surface at runtime, when the offending
line executes.

```python
def to_fahrenheit(c):
    return c * 9/5 + 32

to_fahrenheit(100)      # fine
to_fahrenheit("hot")    # TypeError, but only when this line runs
```

C++: the *variable* carries the type, fixed at compile time and never changing. The
compiler checks every call before the program exists.

```cpp
double to_fahrenheit(double celsius) {
    return celsius * 9.0 / 5.0 + 32.0;
}

to_fahrenheit(100);      // fine — int converts to double
to_fahrenheit("hot");    // compile error, caught before anything runs
```

That's the trade: less flexibility, but a whole category of Python bugs becomes
impossible rather than merely untested.

### `auto` is inference, not dynamism

```cpp
auto x = 5;        // x IS int, permanently
x = "hello";       // compile error
```

`auto` means "work out the type from the initialiser and fix it there." It's closer to a
Python type annotation being *derived* than to Python's dynamic binding. Use it when the
type is obvious or unbearably long (iterators), spell it out when the type is the point.

---

## The types I'll actually meet

| Type | Notes | Python-ish |
|---|---|---|
| `int` | usually 32-bit, signed. Overflow is **undefined behaviour** | `int` (but Python's is unbounded) |
| `double` | default floating point. Use this, not `float` | `float` |
| `bool` | `true` / `false`, lowercase | `bool` |
| `char` | one byte, a character | — |
| `std::string` | owns its text, `#include <string>` | `str` |
| `std::size_t` | **unsigned**, used for sizes and indices | — |
| `std::int32_t` etc. | fixed width, `#include <cstdint>` | — |

`int` in C++ is a fixed-size machine integer. `2'000'000'000 + 2'000'000'000` doesn't grow
a bigger number the way Python does — it overflows, and overflow of a signed int is UB,
not "wraps around".

---

## Build this — a unit converter CLI

```
./convert 100 c-to-f     →  212
./convert 5 km-to-mi     →  3.10686
./convert 10 kg-to-lb    →  22.0462
```

Three files, reinforcing [[C++ 01 - Headers, Translation Units, Linking]].

### `converter.h`

```cpp
#pragma once
#include <string>

double convert(double value, const std::string& unit);
```

`const std::string&` — pass by reference so the string isn't copied, `const` so the
function can't modify the caller's string. This is *the* standard way to take a string
parameter. Module 07 covers why in full.

### `converter.cpp`

```cpp
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
```

`unit == "c-to-f"` compares *contents*, because `unit` is a `std::string` and it overloads
`==`. See the gotcha below for why this is worth noticing.

### `main.cpp`

```cpp
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
```

### `int argc, char* argv[]`

Python hides all of this behind `sys.argv`. Unpacked:

- `argc` — how many arguments, **including the program name**
- `argv` — an array of pointers to C-strings
- `argv[0]` is the program name, so real arguments start at `argv[1]`
- A **C-string** is a `char*` pointing at bytes terminated by a `\0`. It carries no length
  — finding the end means scanning for the terminator
- `argv[argc]` is guaranteed to be `nullptr`

`std::stod(argv[1])` converts the C-string to a `double` and throws
`std::invalid_argument` if it can't. `argv[2]` converts implicitly to `std::string` at the
call, because `std::string` has a constructor taking `const char*`.

### `CMakeLists.txt`

```cmake
add_executable(convert main.cpp converter.cpp)
target_compile_options(convert PRIVATE -Wall -Wextra)
```

---

## Exercises

### 1. Integer division

```cpp
std::cout << 5 / 2 << "\n";          // 2   — not 2.5
std::cout << 5.0 / 2 << "\n";        // 2.5 — one operand is double
std::cout << 5 / 2.0 << "\n";        // 2.5
int a = 5, b = 2;
std::cout << a / b << "\n";          // 2
std::cout << static_cast<double>(a) / b << "\n";   // 2.5
```

Both operands `int` → integer division, result truncated toward zero. Python 3 made `/`
always float and `//` the truncating one; C++ decides based on operand types. **No
warning.** This is why the converter writes `9.0 / 5.0` and not `9 / 5` — the latter is
`1`, and every temperature would come out wrong.

Break it deliberately: change `value * 9.0 / 5.0` to `value * 9 / 5` and check 100°C.

### 2. `auto` is not dynamic

```cpp
auto x = 5;
x = 3.7;                 // compiles! but x is int — prints 3
// x = "hello";          // compile error — read it

auto y = 5 / 2;          // y is int, y == 2
auto z = 5.0 / 2;        // z is double, z == 2.5
```

`x = 3.7` silently truncating is the implicit-conversion trap. It compiles because
`double → int` is a legal conversion, just lossy.

### 3. Catch the narrowing with braces

```cpp
int a = 3.7;     // compiles, a == 3, silent
int b{3.7};      // compile ERROR: narrowing conversion
```

Brace initialisation refuses lossy conversions. A good habit, and one reason modern C++
prefers `{}`.

### 4. The signed/unsigned trap

```cpp
std::vector<int> v = {1, 2, 3};
for (int i = 0; i < v.size(); ++i) { }   // warning with -Wextra
```

`v.size()` returns `std::size_t`, which is **unsigned**. Comparing it against a signed
`int` triggers a warning, and the underlying hazard is real:

```cpp
std::size_t n = 0;
std::cout << n - 1 << "\n";    // 18446744073709551615, not -1
```

Unsigned types wrap rather than go negative. `for (size_t i = v.size() - 1; i >= 0; --i)`
is an infinite loop — `i >= 0` is always true. Use `size_t` for the loop variable, or a
range-for: `for (int x : v)`.

### 5. Feed it garbage

```bash
./convert abc c-to-f
./convert 100 c-to-k
./convert 100
```

All three should fail cleanly rather than crash. Note that `std::stod("12abc")` returns
`12` without complaint — it parses a prefix and stops.

---

## Gotchas

- **Integer division.** `5 / 2 == 2`. No warning. Bites everyone once.
- **Implicit conversions are silent and lossy.** `int x = 3.9;` gives `3`. `{}`
  initialisation is the defence.
- **`==` on `char*` compares pointers, not text.** `std::string` overloads `==` to compare
  contents, so `unit == "c-to-f"` works — but `argv[1] == "abc"` compares *addresses* and
  is essentially always false. A pure Python-brain trap. Convert to `std::string` first.
- **`size_t` is unsigned** and wraps below zero instead of going negative.
- **Signed overflow is undefined behaviour**, not wraparound. The optimizer is allowed to
  assume it never happens.
- **`argv[0]` is the program name.** Real arguments start at index 1.
- **`float` vs `double`** — use `double` unless something forces otherwise.
- **Never compare floats with `==`.** `0.1 + 0.2 != 0.3` here exactly as in Python.

---

## My notes

- 

## Work codebase

- Does it use `int`, `size_t`, or fixed-width `std::int32_t` for sizes and counts?
- Grep for `static_cast` — where is the team converting deliberately?
- Find a function taking `const std::string&`. Anything taking `std::string` by value?
- Is `auto` used liberally or sparingly? Match the house style.
- 

## Done when

- [ ] Converter builds from three files and handles all six units
- [ ] Reproduced the integer-division bug and fixed it
- [ ] Saw `auto x = 5; x = 3.7;` truncate silently
- [ ] Saw `int b{3.7}` refuse to compile
- [ ] Saw `size_t(0) - 1` wrap to a huge number
- [ ] All three garbage inputs fail cleanly

---

← [[C++ 01 - Headers, Translation Units, Linking]] | [[C++ MOC]] | → [[C++ 03 - Stack and Heap]]
