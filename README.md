# Ferrum Programming Language

Ferrum, Rust ve Go'dan esinlenen, sistem programlama için tasarlanmış bir programlama dilidir. Dil, güvenli bellek yönetimi ve eşzamanlı programlama özelliklerine odaklanır.

## Özellikler

- Güvenli bellek yönetimi
- Goroutine benzeri eşzamanlı programlama desteği
- Kanal tabanlı iletişim
- Select ifadesi ile çoklu kanal işlemleri
- Platform bağımsız I/O operasyonları
- Thread-safe hata yönetimi

## Yapı Durumu

Proje şu anda aktif geliştirme aşamasındadır. Bazı temel bileşenler çalışır durumdadır ancak dil henüz production kullanımına hazır değildir.

### Bilinen Sorunlar

1. Windows platformunda bazı güvenlik uyarıları (`strcpy` ve `strncpy` fonksiyonları için)
2. Thread ve Mutex yapılarıyla ilgili tanımlama sorunları
3. Bellek yönetimi fonksiyonlarında iyileştirmeler gerekiyor

### Yapılacaklar

- [ ] Windows güvenlik uyarılarının çözülmesi
- [ ] Thread ve Mutex yapılarının platform bağımsız implementasyonunun iyileştirilmesi
- [ ] Bellek yönetimi fonksiyonlarının güvenlik kontrollerinin artırılması
- [ ] Test coverage'ın artırılması
- [ ] Dökümantasyonun genişletilmesi

## Kurulum

```bash
git clone https://github.com/yourusername/FerrumLang.git
cd FerrumLang
cmake -B build
cmake --build build --config Release
```

## Katkıda Bulunma

Projeye katkıda bulunmak isteyenler için:

1. Repository'yi fork edin
2. Feature branch oluşturun (`git checkout -b feature/amazing-feature`)
3. Değişikliklerinizi commit edin (`git commit -m 'Add some amazing feature'`)
4. Branch'inizi push edin (`git push origin feature/amazing-feature`)
5. Pull Request açın

## Lisans

[Lisans bilgisi eklenecek]

## İletişim

[İletişim bilgileri eklenecek]

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
