---
title: "C++ 04 — Pointers and References"
tags: [cpp, learning, module, pointers, references, memory]
module: 04
status: todo
up: "[[C++ MOC]]"
prev: "[[C++ 03 - Stack and Heap]]"
next: "[[C++ 05 - Debugging and Sanitizers]]"
---

# Module 04 — Pointers and References

> **Goal:** make indirection visible, and know which of the two flavours to reach for.
> **Project:** a singly linked list with a hand-written reverse.
> **Time:** ~2 sessions. Pointers deserve the time.

## The concept

In Python every name is a reference and you never see it. `node.next = other` just rebinds
a label. There's one mechanism and no syntax for it.

C++ makes the indirection explicit and gives you two forms:

| | Pointer `T*` | Reference `T&` |
|---|---|---|
| Can be null | yes (`nullptr`) | **no** |
| Rebindable | yes | **no** — bound once at creation |
| Syntax to use | `*p`, `p->field` | just `r`, `r.field` |
| Is its own object | yes, has its own address | no, it's a second name |
| Taking one | `&x` | happens automatically |

```cpp
int x = 42;

int& r = x;      // r IS x. Another name for the same object.
r = 7;           // x is now 7
// int& r2;      // error — a reference must bind to something

int* p = &x;     // p is a separate variable holding x's address
*p = 7;          // x is now 7
p = nullptr;     // fine — p can be repointed
```

The giveaway is `r = 7`. That doesn't rebind `r`, it assigns through it. A reference can
never be made to refer to something else after birth.

### Which to use

**Default to references.** `Node&` says "this is required and always valid" and the type
itself carries the contract — no null check needed, no caller wondering. Reach for a
pointer only when you need something a reference can't do: representing *absent*
(`nullptr`), or repointing as you walk a structure. Linked-list traversal needs both, which
is why the list is the project.

---

## Build this — a singly linked list

### `list.h`

```cpp
#pragma once

struct Node {
    int value;
    Node* next = nullptr;
};

class List {
public:
    ~List();                       // must free what we new'd

    void push_front(int value);
    void print_all() const;
    Node* find(int value) const;   // pointer: may legitimately find nothing
    void reverse();

private:
    Node* head_ = nullptr;
};
```

`find` returns `Node*` precisely *because* "not found" is a real answer. That's the
pointer's null case earning its place.

### `list.cpp`

```cpp
#include "list.h"
#include <iostream>

List::~List() {
    Node* curr = head_;
    while (curr != nullptr) {
        Node* next = curr->next;   // save before deleting
        delete curr;
        curr = next;
    }
}

void List::push_front(int value) {
    Node* n = new Node{value, head_};
    head_ = n;
}

void List::print_all() const {
    for (Node* curr = head_; curr != nullptr; curr = curr->next)
        std::cout << curr->value << " -> ";
    std::cout << "null\n";
}

Node* List::find(int value) const {
    for (Node* curr = head_; curr != nullptr; curr = curr->next)
        if (curr->value == value) return curr;
    return nullptr;
}
```

This is the manual-memory version on purpose. Every `new` needs a matching `delete`, and
the destructor is the only thing standing between you and a leak. Feel how fragile that is
— [[C++ 06 - RAII and Smart Pointers]] is the fix.

### The reverse — the exercise that makes pointers click

```cpp
void List::reverse() {
    Node* prev = nullptr;
    Node* curr = head_;
    while (curr != nullptr) {
        Node* next = curr->next;   // remember where we were going
        curr->next = prev;         // flip this link backwards
        prev = curr;               // advance prev
        curr = next;               // advance curr
    }
    head_ = prev;
}
```

Four lines, and every one matters. Do it on paper with three fingers before you type it.
Drop the `next` save and you lose the rest of the list on the first iteration.

### `main.cpp`

```cpp
#include "list.h"
#include <iostream>

int main() {
    List l;
    l.push_front(3);
    l.push_front(2);
    l.push_front(1);
    l.print_all();            // 1 -> 2 -> 3 -> null

    l.reverse();
    l.print_all();            // 3 -> 2 -> 1 -> null

    if (Node* n = l.find(2))  // C++17: declare and test in one
        std::cout << "found " << n->value << "\n";
    else
        std::cout << "not found\n";
}
```

---

## Exercises

### 1. Dereference null on purpose

```cpp
Node* n = l.find(999);   // nullptr
std::cout << n->value;   // crash
```

`Segmentation fault`. No message, no line number, no stack trace — the whole experience is
one word. This is Python's `AttributeError: 'NoneType' object has no attribute` one layer
down, minus everything helpful.

Then fix it with `if (n != nullptr)`.

Getting a real diagnosis out of this is exactly what the next module is for —
[[C++ 05 - Debugging and Sanitizers]].

What I got:
```
```

### 2. Let the type carry the contract

Add a free function that must not accept null:

```cpp
void print_node(const Node& n) {      // reference — null is impossible
    std::cout << n.value << "\n";
}

print_node(*some_pointer);            // caller dereferences, so caller checks
```

Compare with `void print_node(const Node* n)`, where the body needs a null check and the
caller can't tell whether it's required. The reference version moved the obligation into
the type. That's the whole argument for references.

### 3. References can't be rebound

```cpp
int a = 1, b = 2;
int& r = a;
r = b;              // does NOT make r refer to b — it sets a = 2
std::cout << a;     // 2
```

Then try `int& r2;` on its own and read the error.

### 4. The declaration trap

```cpp
int* a, b;          // a is int*, b is int!
int *a, *b;         // both pointers
```

The `*` binds to the declarator, not the type. House styles differ (`int* p` vs `int *p`);
the only universal defence is one declaration per line.

### 5. Leak it, then double-free it

```cpp
Node* n = new Node{5};
// no delete — leaked. Nothing warns. The program exits "fine".

Node* m = new Node{6};
delete m;
delete m;            // double free — crash or corruption
```

Neither has any compile-time signal. Run them and note that the leak is completely
invisible — that invisibility is the argument for ASan and for smart pointers.

### 6. `delete` doesn't null the pointer

```cpp
Node* n = new Node{5};
delete n;
std::cout << n->value;    // n still holds the old address. Undefined behaviour.
```

A *dangling pointer* — it points at memory that was yours and isn't any more. It often
appears to work, which is the dangerous part. Setting `n = nullptr;` after `delete` turns a
silent corruption into an honest crash.

---

## Gotchas

- **`*` means two different things.** `int* p` declares; `*p = 5` dereferences. Confusing
  for about a week, then never again.
- **`&` means two different things too.** `int& r = x` declares a reference; `&x` takes an
  address.
- **`int* a, b;` makes only `a` a pointer.** One declaration per line.
- **`.` on an object, `->` through a pointer.** `(*p).value` and `p->value` are identical.
- **Use `nullptr`, not `NULL` or `0`.** Older code uses the others; new code shouldn't.
- **`delete` leaves the pointer dangling.** It frees memory, it doesn't change the variable.
- **Every `new` needs exactly one `delete`.** Zero → leak, two → corruption. No warning
  either way.
- **References can dangle too.** They're safer, not safe — bind one to a local that dies and
  it's as broken as a pointer.
- **Never `delete` something you didn't `new`.** Including stack objects and `this`.

---

## My notes

- 

## Work codebase

- Find a function returning `T*`. Does null mean "not found", or is it just an owning
  pointer in disguise?
- Any raw `new` / `delete` left? Which parts are modernised and which aren't?
- Grep for `nullptr` vs `NULL` — tells you the vintage of each file.
- Find a signature taking `T&` where `T*` would also have worked. Was it deliberate?
- 

## Done when

- [ ] List builds with `push_front`, `print_all`, `find`, destructor
- [ ] `reverse` written by hand and working, understood on paper first
- [ ] Caused and read a segfault, then fixed it with a null check
- [ ] Wrote the `const Node&` version and can say why it's better
- [ ] Confirmed `r = b` assigns rather than rebinds
- [ ] Saw `int* a, b;` declare only one pointer
- [ ] Leaked memory on purpose and confirmed nothing complained

---

← [[C++ 03 - Stack and Heap]] | [[C++ MOC]] | → [[C++ 05 - Debugging and Sanitizers]]
