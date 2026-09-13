---
title: "C++ 00 — Setup and the Compile Step"
tags: [cpp, learning, module, toolchain, cmake]
module: 00
status: doing
up: "[[C++ MOC]]"
next: "[[C++ 01 - Headers, Translation Units, Linking]]"
---

# Module 00 — Setup and the Compile Step

> **Goal:** get a real toolchain, and feel the difference between *compile* and *run*.
> **Time:** ~30 minutes.

## The concept

Python: `python hello.py` — parse and run in one step. Errors surface at runtime, when
the interpreter reaches the broken line.

C++: **compile** (parse, type-check, generate machine code) → **link** (stitch the pieces
into an executable) → **run**. A huge class of Python bugs — typos, wrong types, wrong
argument counts — become *compile errors*, before the program ever executes.

This is the biggest mental shift in the whole course. The compiler is a collaborator, not
an obstacle. It is doing for free what I'd otherwise need tests to catch.

There's also a third step Python has no equivalent of at all: **linking**. See
[[C++ 01 - Headers, Translation Units, Linking]] — but the failure mode shows up already
in the exercises below.

---

## Install

```bash
# Linux
sudo apt install build-essential cmake gdb

# macOS
brew install cmake llvm

# Windows: "Desktop development with C++" via the Visual Studio Installer,
# or use WSL and follow the Linux instructions (recommended)
```

**Editor:** VS Code + the `C/C++` and `CMake Tools` extensions. Skip heavyweight IDEs for
the first few modules — I want to see raw `g++` commands so the build isn't magic.

Verify:

```bash
g++ --version
cmake --version
gdb --version
```

---

## Build 1 — by hand

`hello.cpp`:

```cpp
#include <iostream>

int main() {
    std::cout << "Hello, from a compiled language\n";
}
```

```bash
g++ -std=c++17 -Wall -Wextra -g -O0 hello.cpp -o hello
./hello
```

### The flags, and why each one

| Flag | Why |
|---|---|
| `-std=c++17` | Match the work codebase. Without it, g++ picks its own default. |
| `-Wall -Wextra` | Turn on warnings. C++ is quiet about dangerous things by default. |
| `-g` | Keep debug symbols, so gdb can show line numbers and variable names. |
| `-O0` | No optimization, so the debugger doesn't lie about variable values. |

`-Wall` does **not** mean "all warnings" despite the name. `-Wall -Wextra` together is the
conventional useful baseline.

---

## Build 2 — the same thing with CMake

This is what the work repo actually uses, so get comfortable now.

`CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.16)
project(cpp_course LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(hello hello.cpp)
target_compile_options(hello PRIVATE -Wall -Wextra)
```

```bash
cmake -B build          # configure: generate build files into ./build
cmake --build build     # compile and link
./build/hello
```

Two commands because CMake is a *build-file generator*, not a compiler. The first
generates Makefiles (or Ninja files) in `build/`; the second runs them. Re-run the first
only when `CMakeLists.txt` changes.

### `CMAKE_EXPORT_COMPILE_COMMANDS`

Writes `build/compile_commands.json` — the file that tells editors and tools the exact
flags and include paths for every source file. Without it, VS Code's C/C++ extension
red-squiggles half the codebase because it can't find headers.

**Set this in the work repo's build directory too**, if it isn't already. It's the
difference between a working editor and a useless one. Same file drives `clangd`,
`clang-tidy`, and most other C++ tooling.

---

## Exercise — break it three ways

Do all three. Read each error carefully; the goal is recognising which *tool* is
complaining.

### 1. Misspell `std::cout` as `std::cot`

Compile error, names the line. Easy — and note this is a *compile*-time error where
Python would only fail when execution reached that line.

My output:
```
```

### 2. Delete `#include <iostream>`

The compiler now has no idea what `std::cout` is.

`#include` is literally *"paste that file's text here"*. There is no import system, no
module resolution, no package namespace. The preprocessor does a textual substitution
before the compiler ever sees the file. This is why C++ needs header guards, and why
include order can matter.

My output:
```
```

### 3. Declare a function, never define it

```cpp
#include <iostream>

int add(int a, int b);   // declaration — no body

int main() {
    std::cout << add(1, 2) << "\n";
}
```

**Compiles fine. Fails at link:**

```
undefined reference to `add(int, int)'
```

### Sit with number 3

This is the single most common confusion when a Python dev first touches a C++ repo.

The **compiler** checked that `add` was *declared* — it knew the name, the parameter
types, and the return type, so the call type-checked. It was happy.

The **linker** later went looking for the actual machine code of `add` and found nothing.

Declaration and definition are separate things, they usually live in separate files
(`.h` and `.cpp`), and the tool that complains about each is different. Internalising
this now saves days later.

> **Rule of thumb:** `error:` from the compiler means the *syntax or types* are wrong.
> `undefined reference` from the linker means the *code exists nowhere*, or exists but
> wasn't handed to the linker — a missing `.cpp` in `CMakeLists.txt`, a missing library,
> or a signature mismatch between declaration and definition.

My output:
```
```

---

## Gotchas

- **Two-step build.** Compile errors and link errors are different tools failing.
  Reading the error's *shape* tells me which one before I read the text.
- **`-Wall` isn't all.** Use `-Wall -Wextra`.
- **CMake configure vs build.** Re-run `cmake -B build` only after editing
  `CMakeLists.txt`. `cmake --build build` is the everyday command.
- **The name mangling in linker errors.** `undefined reference to 'add(int, int)'` shows
  the signature because C++ encodes parameter types into symbol names. If the signature
  in the error doesn't match what I think I wrote, the mismatch *is* the bug.

---

## My notes

Observations, surprises, things that cost me time.

- 

## Work codebase

How this maps to the real repo.

- Does it build locally? `cmake -B build && cmake --build build`
- Is `compile_commands.json` generated?
- What flags does it actually use? (check `CMakeLists.txt` / `CMakePresets.json`)
- 

## Done when

- [ ] Toolchain installed, all three `--version` commands work
- [ ] `hello.cpp` built by hand with the full flag set
- [ ] Same program built via CMake
- [ ] All three errors reproduced and pasted above
- [ ] Work codebase builds locally

---

← [[C++ MOC]] | → [[C++ 01 - Headers, Translation Units, Linking]]
