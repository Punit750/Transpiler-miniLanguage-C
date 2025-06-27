# Transpiler_C
# 🧠 Mini-Language Interpreter in C

This project reads and executes programs written in a custom mini-language (`.ml`), implemented in pure C using `runml.c`.

---

## 📁 Files

- `runml.c` – C interpreter for the mini-language
- `sample01.ml`, `sample02.ml`, ... – Test programs in the mini-language

---

## 🚀 How to Run

### 1. Compile the interpreter
```bash
gcc runml.c -o runml
```
### 2. Run it on a .ml file
```bash
./runml sample01.ml
```