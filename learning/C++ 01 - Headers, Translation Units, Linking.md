---
title: "C++ 01 — Headers, Translation Units, Linking"
tags: [cpp, learning, module, linker, headers, cmake]
module: 01
status: doing
up: "[[C++ MOC]]"
prev: "[[C++ 00 - Setup and the Compile Step]]"
next: "[[C++ 02 - Static Types]]"
---

# Module 01 — Headers, Translation Units, Linking

> **Goal:** understand why C++ code is split across `.h` and `.cpp`, and be able to read
> any linker error without panic.
> **Project:** a small `mathutils` library split across files, built with CMake.
> **Time:** ~60 minutes. Worth every one — this is what makes a strange codebase navigable.

## Why this module exists

This isn't in most C++ courses because it teaches nothing about the *language*. But it's
the first wall you hit in an existing codebase: you open a header, find no
implementation, and have no idea where to look. Everything below is about that.

---

## The model

Python: `import foo` — the interpreter finds `foo.py` at runtime, executes it, binds a
module object. One mechanism, resolved while the program runs.

C++ has **no import system.** Instead:

1. **Preprocessor** — expands every `#include` by literally pasting the file's text in.
   The result is one enormous file called a **translation unit** (TU).
2. **Compiler** — compiles each TU *completely independently* into an object file
   (`.o`). It has no knowledge of any other `.cpp` in the project.
3. **Linker** — takes all the `.o` files, resolves every symbol reference against every
   symbol definition, produces one executable.

```
hello.cpp ─┐
           ├─ preprocess → TU → compile → hello.cpp.o ─┐
<iostream>─┘                                           ├─ link → ./hello
mathutils.cpp ── preprocess → TU → compile → .o ───────┘
```

**The key consequence:** when the compiler is working on `main.cpp`, it has never seen
`mathutils.cpp` and never will. The only way it learns that `add` exists is if
`main.cpp`'s text contains a declaration of it. That's the entire reason headers exist.

### Where the header text actually goes

A header is never compiled. It is *pasted*. One file on disk becomes N copies — one per
TU that includes it:

```
                    mathutils.h
                  (body of add)
                        │
          ┌─────────────┴─────────────┐
          │ paste                     │ paste
          ▼                           ▼
┌───────────────────────┐   ┌───────────────────────┐
│ main.cpp — one TU     │   │ mathutils.cpp — one TU│
│  · copy of add        │   │  · copy of add        │
│  · main, calls add    │   │  · multiply, average  │
└───────────┬───────────┘   └───────────┬───────────┘
            │ compile                   │ compile
            ▼                           ▼
     main.cpp.o                   mathutils.cpp.o
     defines add                  defines add
            └─────────────┬─────────────┘
                          ▼
                       linker
                   sees add twice   → duplicate symbol
```

**The two guards act at different levels, which is why one can't fix the other's problem:**

| Guard | Protects against | Level |
|---|---|---|
| `#pragma once` | the paste happening twice **inside one TU** | preprocessor |
| `inline` | two TUs **each** legitimately holding one copy | linker |

Both produce "there are two of these", which is why they feel like the same problem. They
are caught by different tools at different stages.

### Why this is worth the time

Once the three copies are visible, a lot of otherwise-arbitrary C++ follows directly:

- **Implementations go in `.cpp`** — a body in a header is compiled into *every* file that
  includes it. In the `.cpp` it's compiled once.
- **Build times.** A 12-line `main.cpp` expands to ~66k lines. Add a heavy header to a
  widely-included file and that cost multiplies across every TU.
- **Templates must live in headers** — the compiler needs the full body visible in each TU
  that instantiates them. See [[C++ 09 - Templates]].
- **Mismatched definitions are silent** (5c) — the linker deduplicates by symbol name and
  never compares bodies. It cannot know the copies differ.

---

## Declaration vs definition

```cpp
int add(int a, int b);              // declaration: name, params, return type. A promise.
int add(int a, int b) { return a + b; }   // definition: the actual code.
```

- **Declaration** tells the compiler enough to type-check a *call*. Can appear many times.
- **Definition** is the body. Must appear **exactly once** across the whole program.

That last rule is the **One Definition Rule (ODR)**, and it's the source of half of all
linker errors.

### The lesson from Module 00

A declaration with **no call** costs nothing. The linker only hunts for symbols that some
code actually *references* — unused promises are never checked. That's why a header can
declare hundreds of functions and you pay for none of them, and why a missing library
sometimes only breaks the build after you add one new call site.

---

## Build this — `mathutils` across three files

### `mathutils.h` — the interface

```cpp
#pragma once   // include guard: process this file at most once per TU

int add(int a, int b);
int multiply(int a, int b);
double average(const int* values, int count);
```

### `mathutils.cpp` — the implementation

```cpp
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
```

### `main.cpp` — the caller

```cpp
#include <iostream>
#include "mathutils.h"

int main() {
    std::cout << add(2, 3) << "\n";
    std::cout << multiply(4, 5) << "\n";

    int data[] = {1, 2, 3, 4};
    std::cout << average(data, 4) << "\n";
}
```

### `CMakeLists.txt`

```cmake
cmake_minimum_required(VERSION 3.16)
project(cpp_course LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(mathdemo main.cpp mathutils.cpp)
target_compile_options(mathdemo PRIVATE -Wall -Wextra)
```

Both `.cpp` files are listed. The `.h` is **not** — headers aren't compiled, they're
pasted into the files that include them.

```bash
cmake -B build
cmake --build build
./build/mathdemo
```

### The normal arrangement — definition in the `.cpp`

Same header, but holding only `int add(int, int);` with the body in `mathutils.cpp`:

```
                    mathutils.h
                 (declaration only)
                        │
          ┌─────────────┴─────────────┐
          │ paste                     │ paste
          ▼                           ▼
┌───────────────────────┐   ┌───────────────────────┐
│ main.cpp — one TU     │   │ mathutils.cpp — one TU│
│  · pasted declaration │   │  · pasted declaration │
│  · main, calls add    │   │  · body of add ← only │
└───────────┬───────────┘   └───────────┬───────────┘
            │ compile                   │ compile
            ▼                           ▼
     main.cpp.o                   mathutils.cpp.o
     U add — wants it             T add — has it
            └─────────────┬─────────────┘
                          ▼
                       linker
                  matches want to have   → resolved
```

What changes:

1. **Only one copy of the machine code exists.** `main.cpp.o` holds a *request*, not code.
   This is exactly the `U` vs `T` distinction from exercise 4.
2. **`inline` becomes unnecessary.** One TU defines it, ODR satisfied, nothing to dedupe.
3. **Changing the body rebuilds one file.** The practical payoff. With the body in a
   header, *every* file that includes it must recompile — its text changed. On a real
   codebase that's a 4-second rebuild vs a 4-minute one, and it's why "no implementations
   in headers" gets enforced in review.

> **Corollary: touching a header is expensive.** Change `mathutils.h` and every dependent
> TU rebuilds, even for a comment. That's what forward declarations are for, and why
> widely-included headers get treated carefully.

**Trade-off:** with the body in another TU the compiler can't inline the call — it's a
real function call. `-flto` recovers that, and below a few lines it doesn't matter.

### So why do people put implementations in headers anyway?

Four real reasons, roughly in order of how defensible they are.

**1. Necessity — templates.** A template isn't code until instantiated. When a TU writes
`Stack<int>`, the compiler generates that code *there*, so it needs the body in that TU.
Body in a `.cpp` → undefined symbols at every use site. No workaround short of explicit
instantiation. See [[C++ 09 - Templates]].

**2. Performance — inlining.** The optimizer can only inline what it can see. A getter
like `int size() const { return n_; }` should compile to one memory load; across a TU
boundary it's a real call with stack setup. In a hot loop that's measurable. This is the
*original* meaning of `inline`, before it got repurposed for the ODR exemption.
`-flto` mostly erases this argument — not every project enables it, so the habit persists.

**3. Distribution — header-only libraries.** One `#include` and done: no build
integration, no `target_link_libraries`, no matching compiler flags and ABI against a
prebuilt `.a`. Huge adoption advantage — Catch2, doctest, nlohmann/json, Eigen all ship
this way. Paid for on every rebuild; nlohmann/json is ~25k lines pasted into every TU.

**4. `constexpr`.** Compile-time evaluation needs the body visible at the call site, same
reason as templates.

> **A method defined inside the class body is implicitly `inline`.**
> ```cpp
> class Counter {
> public:
>     int value() const { return n_; }   // implicitly inline — no keyword needed
> private:
>     int n_ = 0;
> };
> ```
> That's why in-class definitions never need the keyword and never produce duplicate
> symbols. The class definition itself must be in a header regardless — callers need the
> size and layout to put one on the stack.

**The tension, always the same:** moving an implementation into a header buys inlining or
convenience and sells rebuild speed. On a large codebase rebuild speed usually wins for
anything non-trivial — which is why the **pimpl idiom** exists, deliberately pushing
*everything* out of the header at the cost of a pointer indirection.

### What goes where

| Goes in the `.h` | Goes in the `.cpp` |
|---|---|
| Function declarations | Function bodies |
| Class definitions (the shape) | Method bodies (usually) |
| `constexpr` constants | Non-`inline` globals |
| Templates (must be visible to every user) | Anything private to this file |
| `#pragma once` | |

Rule of thumb: the header is what a *caller* needs to know. Everything else stays hidden
in the `.cpp`, where changing it doesn't force a rebuild of every file that includes you.

---

## Exercises

### 1. See the preprocessor actually paste

```bash
g++ -std=c++17 -E main.cpp | wc -l
```

Thousands of lines, from a 12-line file. That's `<iostream>` and everything it includes,
pasted in. This is why C++ builds are slow and why people care about include hygiene.

Lines I got:
```
```

### 2. Drop a `.cpp` from the build

Remove `mathutils.cpp` from `add_executable`, rebuild.

Result: `undefined reference` / `Undefined symbols` for every function you call. The
declarations in the header still made the compile succeed — the compiler believed the
promise. The code simply wasn't handed to the linker.

**This is the single most common linker error at work:** a new `.cpp` file that nobody
added to `CMakeLists.txt`.

My output:
```
```

### 3. Signature mismatch

In `mathutils.cpp`, change the definition to `int add(long a, long b)` but leave the
header declaring `int add(int, int)`. Rebuild.

Nothing complains at compile time. `mathutils.cpp` sees a declaration of `add(int, int)`
and a definition of `add(long, long)` and is perfectly happy — those are two legitimate
overloads, and no rule says every declaration needs a definition. `main.cpp` calls
`add(2, 3)`, records `U add(int, int)`, and the linker finds only `add(long, long)`.

You get an undefined-symbol error for `add(int, int)` — even though a function named
`add` plainly exists. Because of name mangling, `add(int, int)` and `add(long, long)` are
**different symbols entirely**. The linker isn't being obtuse; it genuinely can't find
the one that was asked for. And there is **no warning**, not even with `-Wall -Wextra`.

> **Trap: changing only the return type is a different error.**
> Header `long add(int, int)` vs source `int add(int, int)` gives a *compile* error:
> `functions that differ only in their return type cannot be overloaded`.
> **Return type is not part of a function's identity.** Both mangle to `__Z3addii`, so
> they can't be two overloads, and the differing return types mean they can't be one
> function either. This is also why C++ can never overload on return type alone.

The version I'll actually meet at work is subtler:

```cpp
void process(const std::string& s);   // header
void process(std::string& s) { }      // source — someone dropped the const
```

Whenever a linker error names a symbol that "obviously exists", diff the two signatures
character by character: a missing `const`, `int` vs `size_t`, `&` vs value.

### 4. See the mangling

```bash
nm build/CMakeFiles/mathdemo.dir/mathutils.cpp.o | c++filt
```

`nm` lists symbols in an object file; `c++filt` demangles them. `T` means defined in the
text (code) section, `U` means undefined — needed from elsewhere. Run `nm` on
`main.cpp.o` too and find the `U` entries. That's the linker's to-do list, made visible.

### 5. Break the ODR — two ways, two different tools

ODR violations come in two flavours, and it matters which one I'm looking at.

#### 5a. Same TU → the **compiler** catches it

Put the body of `add` in `mathutils.h` *and* leave it in `mathutils.cpp`. Because
`mathutils.cpp` includes the header, after preprocessing **both definitions are in the
same translation unit**:

```
error: redefinition of 'add'
note: previous definition is here
```

`inline` does **not** help here. Adding it to both just gives
`redefinition of 'add'` again. `inline` means "this definition may appear once in each of
several TUs"; it never means "twice in one TU."

#### 5b. Different TUs → the **linker** catches it

Define `add` **only** in the header, non-`inline`, and delete it from the `.cpp`:

```cpp
// mathutils.h
#pragma once

int add(int a, int b) { return a + b; }   // definition, NOT inline

int multiply(int a, int b);
double average(const int* values, int count);
```

Both `main.cpp` and `mathutils.cpp` include the header, so each `.o` ends up defining the
symbol:

```
duplicate symbol '__Z3addii' in:
    main.cpp.o
    mathutils.cpp.o
```

**Fix:** mark the header definition `inline`. Or move the body back to the `.cpp` and
leave only a declaration in the header (the normal arrangement).

| Two definitions land in | Caught by | Message |
|---|---|---|
| the same TU | compiler | `redefinition of 'add'` |
| different TUs | linker | `duplicate symbol` |

`inline` today means "multiple identical definitions across TUs are allowed, pick one."
It has essentially nothing to do with inlining for performance any more; that decision is
the optimizer's. This is also why header-only libraries mark everything `inline`.

#### 5c. What if the definitions aren't identical?

Then it's an ODR violation, and the standard calls this **ill-formed, no diagnostic
required** — undefined behaviour that nobody is obliged to report.

Which definition wins is **unspecified**. The linker usually keeps the first one it
encounters, so it's deterministic for a given build but depends on the order object files
were handed to the linker. Reorder files in `add_executable` and the winner can change —
the classic shape of a bug that appears and disappears for no visible reason.

Nobody catches it: the compiler sees one definition per TU and is satisfied; the linker
deduplicates by **symbol name only** and never compares bodies.

The realistic cause is never two different bodies typed by hand. It's one header that
*expands differently* in different TUs:

```cpp
// config.h
struct Config {
#ifdef ENABLE_FEATURE
    int extra;
#endif
    int x;
};
```

Compile `a.cpp` with `-DENABLE_FEATURE` and `b.cpp` without: the two files now disagree
about `sizeof(Config)` and where `x` lives. Compiles, links, then one file writes through
a pointer using the other's layout. Memory corruption, far from the cause.

Same mechanism with mismatched `NDEBUG`, different `-O` levels affecting `assert`-laden
inline functions, or linking a debug-built library into a release binary.

**Defences:**
- Keep compile flags uniform across a target — `target_compile_definitions`, not per-file
  defines.
- Be suspicious of `#ifdef` around anything in a header that affects struct layout or an
  inline function body.
- ASan detects ODR violations for global variables by default → [[C++ 05 - Debugging and Sanitizers]]
- GCC's `-Wodr` catches some cases, but only with LTO enabled.

> This is the first place C++ **trusts** me. The rule exists so headers can contain code
> at all; enforcing it would mean comparing every function body across every object file.
> The language chose fast builds and put the obligation on the programmer. Same tradeoff
> shows up again with `std::move` ([[C++ 10 - Move Semantics]]) and data races
> ([[C++ 12 - Concurrency]]).

### 6. Remove `#pragma once`

Delete it from `mathutils.h`, then in `main.cpp` include the header twice. If the header
only declares functions, nothing breaks (redeclaration is legal). Add a `struct Point {
int x, y; };` to the header and try again: `redefinition of 'Point'`.

That's the whole job of the guard. Real headers include other headers, so the same file
gets reached along several paths in one TU constantly.

`#pragma once` is non-standard but universally supported. Old code uses the portable
form:

```cpp
#ifndef MATHUTILS_H
#define MATHUTILS_H
// ...
#endif
```

---

## Reading any linker error

```
Undefined symbols for architecture arm64:
  "add(int, int)", referenced from:
      _main in hello.cpp.o
ld: symbol(s) not found for architecture arm64
```

| Part | Meaning |
|---|---|
| `ld:` / `Undefined symbols` | **Linker**, not compiler. Code compiled fine. |
| `"add(int, int)"` | The wanted symbol, demangled. Check this signature carefully. |
| `referenced from: _main in hello.cpp.o` | **Who wanted it.** Open this file first. |

Then work through the causes in order of likelihood:

1. The `.cpp` isn't in `CMakeLists.txt`
2. Signature mismatch between declaration and definition
3. Defined, but in a library that wasn't linked (`target_link_libraries`)
4. Declared and called, never defined at all
5. A method declared in a class but never defined out-of-line

---

## Finding where anything is defined

The daily skill in an unfamiliar codebase.

```bash
grep -rn "add(" --include=*.cpp --include=*.h .
rg "^\w[\w:<>&* ]*\badd\s*\("          # ripgrep, roughly "definition-shaped" lines
```

Better: get `clangd` working with `compile_commands.json` (from Module 00) and use
go-to-definition. That's what the file was for.

---

## Gotchas

- **`#include` is textual paste.** No namespacing, no module resolution. Include order
  can matter. Two headers can collide.
- **Each `.cpp` compiles alone.** The compiler never sees your other files. Anything it
  should know must be declared in text it can reach.
- **ODR: exactly one definition.** Zero → undefined symbol. Two → duplicate symbol.
- **Templates can't go in a `.cpp`.** The full definition must be visible wherever it's
  instantiated, so templates live in headers. Comes up in [[C++ 09 - Templates]].
- **`"quotes"` vs `<angle brackets>`** — quotes search relative to the current file
  first, then the include path. Use quotes for your project, angles for system and
  third-party headers.
- **Headers don't go in `add_executable`.** Only `.cpp` files are compiled.

---

## My notes

- 

## Work codebase

- Pick a function I've seen called. Find its declaration (`.h`) and definition (`.cpp`).
- How is the project structured — one big target, or several libraries via
  `add_library` + `target_link_libraries`?
- Does it use `#pragma once` or `#ifndef` guards? Match the local style.
- Find a header that's included nearly everywhere. That's the one that makes builds slow.
- 

## Done when

- [ ] `mathutils` builds and runs across three files
- [ ] Saw the preprocessor output size
- [ ] Reproduced the missing-`.cpp` linker error
- [ ] Reproduced the signature-mismatch error and understood why
- [ ] Ran `nm | c++filt` and identified `T` vs `U` symbols
- [ ] Reproduced `redefinition` (compiler, same TU) and `duplicate symbol` (linker, two TUs)
- [ ] Fixed the linker one with `inline`
- [ ] Reproduced the missing-guard redefinition error
- [ ] Found one function's declaration and definition in the work codebase

---

← [[C++ 00 - Setup and the Compile Step]] | [[C++ MOC]] | → [[C++ 02 - Static Types]]
