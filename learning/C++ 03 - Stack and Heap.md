---
title: "C++ 03 — Stack and Heap"
tags: [cpp, learning, module, memory, value-semantics]
module: 03
status: todo
up: "[[C++ MOC]]"
prev: "[[C++ 02 - Static Types]]"
next: "[[C++ 04 - Pointers and References]]"
---

# Module 03 — Stack and Heap

> **Goal:** feel the difference between an object that *is* a value and a name that
> *refers* to one, and see deterministic destruction.
> **Project:** a `Matrix` struct.
> **Time:** ~90 minutes.

## The concept

In Python **everything** is a heap object behind a reference, and the runtime cleans up
when the last reference goes away. There is one storage model and you never choose it.

C++ gives you two, and you choose per object:

**Stack** — automatic storage. Created when control reaches the declaration, destroyed at
the closing `}`, *deterministically*. Allocation is a pointer bump, so it's nearly free.
Size must be known at compile time, and total space is small (often 8 MB).

**Heap** — created with `new` (or by a container internally), lives until something frees
it. Size can be decided at runtime, lifetime can outlive the scope that made it. Slower,
and somebody has to own the cleanup.

This choice is the reason C++ exists as a language.

### Value semantics — the real shock

```python
a = [1, 2, 3]
b = a          # b and a are the SAME list
b.append(4)    # a is now [1, 2, 3, 4]
```

```cpp
std::vector<int> a = {1, 2, 3};
std::vector<int> b = a;   // b is a SEPARATE vector with its own buffer
b.push_back(4);           // a is still {1, 2, 3}
```

In C++ a variable **is** the object, not a name pointing at one. Assignment and
pass-by-value **copy**. For a `std::vector<int>` of a million elements, that's a million
elements copied, silently. Python never does this, which is exactly why it catches people
out. [[C++ 10 - Move Semantics]] is about avoiding the copies you don't want.

---

## Build this — `Matrix`

### `matrix.h`

```cpp
#pragma once
#include <vector>
#include <iostream>

struct Matrix {
    int rows;
    int cols;
    std::vector<double> data;   // heap-backed storage

    Matrix(int r, int c) : rows(r), cols(c), data(r * c, 0.0) {}

    double& at(int r, int c)             { return data[r * cols + c]; }
    double  at(int r, int c) const       { return data[r * cols + c]; }

    void print() const;
};

Matrix operator+(const Matrix& a, const Matrix& b);
```

### What the diagram shows

The `Matrix` object itself — `rows`, `cols`, and the vector's internal handle — sits in
the **stack frame**. The nine `double`s live on the **heap**, allocated by the vector.
`sizeof(Matrix)` is a couple of dozen bytes regardless of how big the matrix is, because
the elements aren't *in* the object.

This split is the single most common shape in C++: a small stack-based handle owning a
large heap buffer. `std::string` and `std::unique_ptr` work the same way. It's also
[[C++ 06 - RAII and Smart Pointers]] in embryo — the vector's destructor frees the buffer,
automatically, when the `Matrix` goes out of scope.

**Because the vector does that work, `Matrix` needs no destructor, no copy constructor,
and no assignment operator.** Copying a `Matrix` copies the vector, which copies its
buffer. That's the *rule of zero*: build from members that manage themselves and write
none of it yourself.

### Member initializer list

```cpp
Matrix(int r, int c) : rows(r), cols(c), data(r * c, 0.0) {}
//                   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
```

Everything after the `:` and before the `{` runs *before* the constructor body. Members
are initialised directly rather than default-constructed and then assigned. Assigning
inside the `{}` body instead is the classic "written like Python" habit — it works for
`int`s but does redundant work for anything that owns a resource, and won't compile at all
for `const` or reference members.

**Members initialise in declaration order**, not the order you write them in the list.
`-Wall` warns if they disagree. Keep both orders the same.

### `matrix.cpp`

```cpp
#include "matrix.h"
#include <stdexcept>

void Matrix::print() const {
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) std::cout << at(r, c) << ' ';
        std::cout << '\n';
    }
}

Matrix operator+(const Matrix& a, const Matrix& b) {
    if (a.rows != b.rows || a.cols != b.cols)
        throw std::invalid_argument("dimension mismatch");

    Matrix result(a.rows, a.cols);
    for (int i = 0; i < a.rows * a.cols; ++i)
        result.data[i] = a.data[i] + b.data[i];
    return result;
}
```

Then write `multiply` yourself and multiply two small matrices.

---

## Exercises

### 1. Watch the copy happen

Add a loud copy constructor:

```cpp
Matrix(const Matrix& other)
    : rows(other.rows), cols(other.cols), data(other.data) {
    std::cout << "COPY of " << rows << "x" << cols << "\n";
}
```

Then:

```cpp
void by_value(Matrix m)         { std::cout << "inside: " << &m << "\n"; }
void by_ref(const Matrix& m)    { std::cout << "inside: " << &m << "\n"; }

Matrix m(3, 3);
std::cout << "orig:   " << &m << "\n";
by_value(m);   // prints COPY, and a DIFFERENT address
by_ref(m);     // no COPY, and the SAME address
```

The address is the proof: `by_ref` is looking at the caller's object, `by_value` built its
own. This is why `const T&` is the default way to pass anything bigger than a machine word.

### 2. Deterministic destruction

```cpp
struct Noisy {
    std::string name;
    Noisy(std::string n) : name(std::move(n)) { std::cout << name << " born\n"; }
    ~Noisy() { std::cout << name << " dies\n"; }
};

int main() {
    Noisy a("a");
    {
        Noisy b("b");
        std::cout << "-- inner scope end --\n";
    }
    std::cout << "-- main end --\n";
}
```

`b` dies at the inner `}`, exactly there — not "whenever the GC runs". Note the output
order: **destruction is reverse of construction**. The last one built is the first to go,
because it's the top of the stack.

### 3. `sizeof` doesn't measure the data

```cpp
Matrix small(1, 1);
Matrix big(1000, 1000);
std::cout << sizeof(small) << " " << sizeof(big) << "\n";   // identical
std::cout << big.data.size() * sizeof(double) << "\n";       // 8,000,000
```

`sizeof` is a compile-time property of the *type*. It counts the handle, never the heap
buffer behind it.

### 4. Blow the stack

```cpp
int main() {
    double huge[2'000'000];   // ~16 MB on the stack
    huge[0] = 1.0;
}
```

Segfault, usually without reaching a single line of your code. The same data in a
`std::vector<double> huge(2'000'000);` is fine, because the elements go on the heap.
Rule: anything big, or sized at runtime, belongs on the heap — which in practice means a
container.

### 5. The dangling reference

```cpp
Matrix& make_matrix() {
    Matrix local(3, 3);
    return local;          // local dies at the }
}
```

With `-Wall` you get *reference to local variable returned*. Use the returned reference
anyway and it's undefined behaviour — the stack memory has been reused. This bug cannot
exist in Python, where returning an object keeps it alive.

**Don't lean on the warning.** It fires for the obvious case and not once there's any
indirection. That's what ASan is for — [[C++ 05 - Debugging and Sanitizers]].

Returning `Matrix` *by value* is correct and not slow: the compiler elides the copy
(RVO), so nothing is duplicated. See [[C++ 10 - Move Semantics]].

---

## Gotchas

- **Assignment copies.** `Matrix b = a;` is a full duplicate, not an alias. The single
  biggest Python-brain trap in the language.
- **Pass-by-value copies too.** Default to `const T&` for anything non-trivial.
- **Stack objects die at `}`**, deterministically, in reverse order of construction.
- **Never return a reference or pointer to a local.** The object is gone.
- **`sizeof` ignores heap data.**
- **Stack space is small** — a few MB. Large or runtime-sized data goes in a container.
- **Members initialise in declaration order**, not initializer-list order.
- **Rule of zero:** if every member manages itself, write no destructor, no copy
  constructor, no assignment operator. Writing one badly is worse than writing none.

---

## My notes

- 

## Work codebase

- Find a function taking a big object **by value**. Is the copy intended?
- Grep for `&` in parameter lists — how much is passed by reference?
- Any class with a hand-written destructor? Why wasn't rule-of-zero enough?
- Find a type that's a small handle over a heap buffer, like `Matrix` here.
- 

## Done when

- [ ] `Matrix` builds, with `at`, `operator+`, `print`, and a `multiply` I wrote
- [ ] Saw the copy fire for by-value and not for `const&`, with different addresses
- [ ] Saw `Noisy` destruct in reverse order at the exact closing brace
- [ ] Confirmed `sizeof(Matrix)` is the same for 1×1 and 1000×1000
- [ ] Crashed the stack with a huge local array, fixed it with a vector
- [ ] Triggered the returned-reference-to-local warning

---

← [[C++ 02 - Static Types]] | [[C++ MOC]] | → [[C++ 04 - Pointers and References]]
