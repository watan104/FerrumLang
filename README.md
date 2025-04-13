# Ferrum Programming Language

Ferrum is a small, experimental programming language implemented in C. It's currently in early development and **not yet functional**, but it aims to grow into a simple and expressive language inspired by Rust, Go, and other modern systems languages.

This project is being shared as open-source so others can learn from, build upon, or contribute to its development.

---

## 🚧 Project Status

> **Warning**: This project is under construction. The codebase currently contains **numerous bugs** and **incomplete features**.

- ✅ Lexer partially implemented
- ✅ Pratt parser scaffold with token rules
- 🚧 AST and code generation are incomplete
- ❌ No runtime or VM implemented yet
- ❌ No standard library
- ❌ Error handling is rudimentary

Despite these issues, the project is structured in a modular way to encourage contributions and further development.

---

## 💡 Goals

- Build a toy language with clear, readable compiler code.
- Learn about compiler theory (lexer, parser, AST, codegen).
- Inspire others to write their own languages.

---

## 🔧 Building

Ferrum is written in C and uses standard libraries. It is intended to be built with GCC or Clang:

```bash
make   # or use your own build system
./ferrum examples/hello.fr
```

> You may need to set up tests manually under `tests/unit/`.

---

## 📂 Directory Structure

```
Ferrum/
├── src/
│   └── compiler/   # Main compiler logic (lexer, parser, AST)
├── tests/          # Unit tests (WIP)
├── examples/       # Example Ferrum programs
├── docs/           # Language documentation (WIP)
│   ├── language.md         # Ferrum language syntax and features
│   ├── architecture.md     # Compiler architecture and flow
│   └── contributing.md     # How to contribute to the project
└── README.md       # This file
```

---

## 🤝 Contributing

Contributions are welcome! If you're curious about compilers or language design, feel free to fork this repo, file an issue, or open a pull request.

- You can help by improving documentation, fixing bugs, or extending the parser.
- Please keep code simple and readable for educational purposes.

---

## 📜 License

This project is licensed under the MIT License. See `LICENSE` file for details.

---

## 🙋‍♂️ Author's Note

I am sharing Ferrum even in its incomplete state to invite collaboration and learning. This is not a production-ready tool — it's a compiler construction sandbox. If it inspires you to build your own language, it has done its job!

---

## 📄 Docs

### `docs/language.md`
Describes the syntax and semantics of Ferrum. Includes variable declarations, functions, types, control flow, and examples.

### `docs/architecture.md`
Explains the overall design of the compiler, including the lexer, Pratt parser, AST representation, and future code generation plans.

### `docs/contributing.md`
Guidelines and tips for developers who want to contribute to Ferrum.
