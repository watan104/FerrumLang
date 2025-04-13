# Ferrum Language Overview

> **Note**: Ferrum is under development and not yet fully working. This document represents the intended design.

---

## 📌 Introduction
Ferrum is a statically typed, expression-oriented language inspired by Rust and Go. The goal is to provide a minimal but expressive syntax suitable for learning and experimentation.

---

## 🔤 Syntax Overview

### Variable Declaration
```ferrum
let x = 10;
let name = "Ferrum";
```

### Functions
```ferrum
fn add(a, b) {
    return a + b;
}

let result = add(5, 3);
```

### Conditionals
```ferrum
if x > 10 {
    print("big");
} else {
    print("small");
}
```

### Match (Pattern Matching - planned)
```ferrum
match value {
    0 => print("zero"),
    1 => print("one"),
    _ => print("many"),
}
```

### Types (planned)
```ferrum
type Point {
    x,
    y,
}

let p = Point { x: 10, y: 20 };
```

---

## ⚙️ Features (Planned)
- First-class functions
- Algebraic data types
- Optional garbage collection or manual memory control
- Simple concurrency model (`go` blocks)
- No nulls, focus on safe defaults

---

## 🚫 Known Limitations
- No operator precedence parsing for some expressions yet
- Type system is not implemented
- Scoping and closures are still under consideration
- No codegen or runtime yet

---

## 📈 Example Program
```ferrum
fn factorial(n) {
    if n == 0 {
        return 1;
    } else {
        return n * factorial(n - 1);
    }
}

let result = factorial(5);
print(result);
```

---

## 🔚 Conclusion
This document will grow as the Ferrum language evolves. For now, it's a snapshot of the ideas and planned features that will be supported in the future. Contributions and feedback are very welcome!