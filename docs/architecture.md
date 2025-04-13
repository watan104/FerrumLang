# Ferrum Compiler Architecture

> This document outlines the current and planned architecture of the Ferrum compiler. It is a work-in-progress and subject to change as the project evolves.

---

## 🧱 High-Level Overview
Ferrum is implemented in C and follows a traditional compiler pipeline:

```
[Source Code] -> [Lexer] -> [Parser] -> [AST] -> [Codegen / VM] (planned)
```

Each stage is modular, allowing for step-by-step experimentation and testing.

---

## 1. Lexer (Tokenizer)
Located in `src/compiler/lexer.c`

- Responsible for converting source code into a stream of tokens.
- Recognizes identifiers, numbers, symbols, keywords, and string literals.
- Uses a combination of character inspection and DFA-style logic.

### Example Output:
Input: `let x = 5;`
```
[LET, IDENT(x), EQUAL, INT(5), SEMICOLON]
```

---

## 2. Parser (Pratt-based)
Located in `src/compiler/parser.c`

- Uses Pratt parsing to handle expressions with different precedence levels.
- Builds an Abstract Syntax Tree (AST) from tokens.
- Statement types include variable declarations, function calls, blocks, and conditionals.

### Example:
```ferrum
let a = 3 + 4 * 2;
```
Becomes:
```
LetStatement {
  name: "a",
  value: BinaryExpr(
    left: Int(3),
    operator: '+',
    right: BinaryExpr(Int(4), '*', Int(2))
  )
}
```

---

## 3. AST (Abstract Syntax Tree)
Planned for full implementation.

- A tree structure representing the semantic meaning of the code.
- Nodes include literals, binary operations, control flow, function declarations, etc.
- Will be used for semantic analysis and code generation.

---

## 4. Code Generation / Virtual Machine (Planned)

- The long-term plan is to either:
  - Compile to bytecode and run on a simple VM
  - Or generate C code or LLVM IR

- This part is not implemented yet.

---

## 🔁 Testing

- Minimal unit tests under `tests/unit/`
- Lexer and parser tests will be extended as language features are added.

---

## ⚠️ Challenges
- Error handling is primitive
- Parser needs better synchronization logic for malformed input
- AST optimization and semantic analysis are not yet addressed

---

## 🚀 Next Steps
- Finalize and document AST node types
- Add support for type annotations
- Implement simple bytecode and VM
- Improve error diagnostics and reporting

---

## 🤝 Contributions Welcome
If you're interested in compiler design or want to learn how languages are built, contributions are encouraged! See `contributing.md` for details.

This document will be kept up to date as the compiler architecture evolves.