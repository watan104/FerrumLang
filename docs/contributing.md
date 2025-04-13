# Contributing to Ferrum

Thank you for your interest in contributing to Ferrum! This project is a learning-oriented, open-source compiler for a small programming language. Even though it is in an early stage, every improvement helps.

---

## ✍️ Ways to Contribute

- **Report Bugs**: If you spot a bug or strange behavior, open an issue with reproduction steps.
- **Improve the Lexer/Parser**: Help build a more complete and robust front-end.
- **Write Unit Tests**: Many components need coverage — especially the parser.
- **Expand Language Features**: Suggest and/or implement new language constructs.
- **Improve Documentation**: Clarify how things work and document design decisions.

---

## 🔧 Setup Instructions

1. Clone the repository:
   ```bash
   git clone https://github.com/your-username/ferrum
   cd ferrum
   ```

2. Build the project:
   ```bash
   make
   ```

3. Run examples or tests:
   ```bash
   ./ferrum examples/hello.fr
   # Or run test files manually under tests/unit/
   ```

---

## 🧪 Writing Tests

Tests are located in `tests/unit/`. The test files should include small C programs that check:

- Tokenization (lexer)
- Parsing (AST structure)
- Error handling for malformed input

Example test structure:
```c
TEST_CASE("lexer handles numbers") {
    const char* input = "let x = 42;";
    TokenList tokens = lex(input);
    REQUIRE(tokens[0].type == TOKEN_LET);
    REQUIRE(tokens[1].value == "x");
    REQUIRE(tokens[2].type == TOKEN_EQUAL);
    REQUIRE(tokens[3].type == TOKEN_INT);
}
```

---

## 📦 Code Style
- Keep code simple and readable.
- Use consistent indentation (4 spaces preferred).
- Write small, focused functions.
- Avoid macros unless necessary.

---

## 🌍 Code of Conduct
Be respectful and constructive. This project is for learners of all levels — mistakes are part of the process.

---

## 📬 Feedback
Have questions or suggestions? Open an issue or reach out by starting a discussion on GitHub.

---

## 🙏 Thanks
Every contribution — big or small — is appreciated. Your help makes this language and learning experience better for everyone!