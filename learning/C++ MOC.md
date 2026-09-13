---
title: "C++ for Python Developers — Map of Content"
tags: [cpp, learning, moc]
standard: C++17
build-system: CMake
started: 2026-09-13
status: in-progress
---

# C++ for Python Developers — MOC

Entry point for the whole path. Each module gets its own note, created as I reach it.

## Context

I already know how to program. What I need is to know **where C++ forces me to think
differently than Python**, and to feel that difference by writing code.

| | |
|---|---|
| Background | Python developer |
| Goal | Contribute to my company's existing C++ codebase |
| Standard | **C++17** |
| Build system | **CMake** |
| Mode | Modules in order, toy projects |

**The one adjustment that shapes everything:** most C++ material teaches you to *write*
greenfield modern C++. My actual job is to *read and modify* existing code. Different
skill, different bottlenecks — which is why the compilation model, debugging, and error
handling conventions got promoted into their own modules.

---

## Roadmap

| # | Module | Project | Status |
|---|---|---|---|
| 00 | [[C++ 00 - Setup and the Compile Step]] | Hello, by hand and by CMake | |
| 01 | [[C++ 01 - Headers, Translation Units, Linking]] | Split a program across files | |
| 02 | [[C++ 02 - Static Types]] | Unit converter CLI | |
| 03 | [[C++ 03 - Stack and Heap]] | `Matrix` struct | |
| 04 | [[C++ 04 - Pointers and References]] | Linked list, by hand | |
| 05 | [[C++ 05 - Debugging and Sanitizers]] | Debug the crashes from 04 | |
| 06 | [[C++ 06 - RAII and Smart Pointers]] | Leak-free linked list | |
| 07 | [[C++ 07 - Classes and const Correctness]] | `BankAccount` | |
| 08 | [[C++ 08 - The STL]] | Word-frequency counter | |
| 09 | [[C++ 09 - Templates]] | Generic `Stack<T>` | |
| 10 | [[C++ 10 - Move Semantics]] | Copy vs move benchmark | |
| 11 | [[C++ 11 - Error Handling Conventions]] | Audit my work codebase | |
| 12 | [[C++ 12 - Concurrency]] | Parallel word counter *(optional)* | |
| 13 | [[C++ 13 - Capstone]] | CSV task tracker → a real ticket | |

Mark status as `todo` / `doing` / `done` as I go.

### Why modules 01, 05 and 11 were added

- **01 — Headers and linking.** You cannot navigate a C++ repo without this. Open a
  header, find no implementation, have no idea where to look. Also the source of the
  most confusing error class for newcomers: `undefined reference to ...`.
- **05 — Debugging and sanitizers.** Python hands you a traceback for free. C++ hands
  you `Segmentation fault (core dumped)`. ASan is the closest thing to getting that
  traceback back.
- **11 — Error handling conventions.** Exceptions vs error codes vs `std::optional`.
  Plenty of production codebases compile with `-fno-exceptions`. This changes how every
  function gets written, so it's worth knowing early what mine does.

### Timeboxed on purpose

- **Module 06's `unique_ptr` linked list** — one evening, no more. It's a great way to
  *feel* ownership, but a `unique_ptr`-linked list destructs recursively and blows the
  stack on a long list, and reversing it is genuinely awkward. Teaching device, not a
  model of production code.
- **Module 12 concurrency** — skip entirely unless the work codebase uses threads. A lot
  of effort for something I may never touch.

---

## C++17 quick reference

What I **do** get in C++17 (and should reach for):

| Feature | Python analogue |
|---|---|
| `std::optional<T>` | a value or `None` |
| `std::string_view` | a non-owning slice of a string |
| Structured bindings — `auto [k, v] = pair;` | tuple unpacking |
| `for (const auto& [word, n] : counts)` | `for word, n in counts.items()` |
| `std::filesystem` | `pathlib` |
| CTAD — `std::vector v{1, 2, 3};` | inferred element type |
| `if (auto it = m.find(k); it != m.end())` | walrus operator in an `if` |

What I **don't** get — ignore tutorials using these:

- Concepts and `requires` (C++20)
- Ranges, `views::filter` etc. (C++20)
- `std::format` (C++20) — use streams or `fmt` instead
- `std::expected` (C++23)

**Expect old code.** Files in the work repo will still use `new`/`delete`, raw owning
pointers, and pre-C++11 idioms. That's normal history, not a bug to fix on sight.

---

## Standing conventions

Compiler flags for everything in this course:

```bash
g++ -std=c++17 -Wall -Wextra -g -O0 file.cpp -o file
```

- `-Wall -Wextra` — C++ is silent about dangerous things by default
- `-g` — keep debug symbols so gdb shows line numbers
- `-O0` — no optimization, so the debugger doesn't lie about variable values

Baseline `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.16)
project(cpp_course LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(hello hello.cpp)
target_compile_options(hello PRIVATE -Wall -Wextra)
```

Build and run:

```bash
cmake -B build
cmake --build build
./build/hello
```

---

## Running log

Things that surprised me, cost me time, or clicked. Append freely, link out to module
notes.

- 

## Questions for the team

Things about the work codebase I should just ask someone.

- Does the codebase build with exceptions enabled, or `-fno-exceptions`?
- What's the testing framework?
- Is there a style guide / clang-format config?

## After this

Direction depends on what I actually want C++ for:

- **Performance-critical Python extensions** → `pybind11`
- **Systems / embedded** → manual memory, `constexpr`, no-STL environments
- **Games / graphics** → OpenGL or a small engine tutorial
- **General mastery** → *Effective Modern C++* (Scott Meyers). Modules 06 and 10 are
  straight out of that book, compressed.
