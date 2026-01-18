Glyph Programming Language Interpreter
======================================

Glyph is a minimalist, AI-oriented programming language that uses only nine characters and a single literal value. This project is a C++ interpreter for Glyph.

```text
Valid characters: * ( ) + - ^ % _ :
```

Glyph is not designed for humans to write large systems — it is designed for AI to generate, analyze, and evolve code easily.

***

## Language Overview

### Character Set

Only these characters are valid in source code:

```text
( ) + - ^ % _ :
```

Anything else is a syntax error.

### Core Model

Everything is an **expression**.  
There are only four conceptual constructs:

- Values
- Operations
- Bindings
- Conditionals

There are **no statements**, **no blocks**, and **no side effects**.  
A program is exactly **one expression**.  
The result of evaluating this expression is the program output.

***

## Values

### Primitive Literal

There is exactly **one literal value**:

```text
_
```

It represents the unit value, conceptually `1`.

All numbers are derived from `_` using operations.

Examples (conceptual intent):

```text
_        ; 1
(+ _ _)  ; 1 + 1 = 2
(+ (+ _ _) _) ; (1 + 1) + 1 = 3
```

In more compact “symbolic” style you might see:

```text
_        ; 1
+_ _     ; 2
++ _     ; 3
```

***

## Operators

All operators are **prefix** and **binary**.

| Symbol | Operation       | Arity |
|--------|-----------------|-------|
| `+`    | Addition        | 2     |
| `-`    | Subtraction     | 2     |
| `*`    | Multiplication  | 2     |
| `^`    | Exponentiation  | 2     |
| `%`    | Modulo          | 2     |

Canonical parenthesized syntax:

```text
(+ a b)
(- a b)
(* a b)
(^ a b)
(% a b)
```

Whitespace is optional; the interpreter ignores everything except the nine allowed characters.

***

## Grouping

Parentheses group expressions:

```text
(+ _ _)
(* (+ _ _) _)
(^ (+ _ _) _)
```

***

## Bindings (Let Expressions)

Bindings introduce local names structurally.

Syntax:

```text
:(name)(value)(body)
```

Conceptual meaning:

```text
let name = value in body
```

Example (spec-style):

```text
:()()(+_ _)
```

Read as “let something = something in 1 + 1”; in this interpreter, names are also expressions (see below), but in practice you will generally treat bindings as scoping constructs around subexpressions.

***

## Functions

The specification treats functions as expressions with bound parameters, conceptually:

- Function definition:

  ```text
  :(param)(body)
  ```

- Function call:

  ```text
  *(function)(argument)
  ```

The current interpreter focuses on evaluation of values, binary operators, conditionals, and structural bindings. The `*` operator is implemented as arithmetic multiplication.

***

## Conditionals

Conditionals are encoded using `%` with a three-argument form:

```text
%(condition)(then)(else)
```

Rule:

- If `condition % _ == 0` → false → evaluate `else`
- Otherwise → evaluate `then`

In the interpreter, this is implemented as:

- If `condition` evaluates to `0` → evaluate `else`
- Otherwise → evaluate `then`

This matches the spirit of “zero is false, non-zero is true”.

***

## Evaluation Rules

1. Parse the program into an AST.
2. Evaluate innermost expressions first.
3. Apply operators strictly (no lazy evaluation).
4. All evaluation is pure and deterministic.
5. Bindings create new environments; they do not mutate global state.

***

## Project Structure

Single-file interpreter:

- `glyph.cpp` — Lexer, parser, AST, and evaluator for Glyph.

You can pair it with:

- `Makefile` — For building the interpreter with `clang++`.

***

## Building

### Requirements

- C++17-capable compiler (tested with `clang++`)
- `make` (optional but recommended)
- A Unix-like environment (Linux, macOS, MSYS2/MinGW64 on Windows, etc.)

### Build with Makefile

From the project root:

```bash
# Release build
make

# Debug build
make debug

# Clean artifacts
make clean
```

This produces:

- `glyph` — release binary
- `glyph_debug` — debug binary (with symbols)

### Build Manually (Without Makefile)

```bash
clang++ -std=c++17 -Wall -Wextra -pedantic -O2 glyph.cpp -o glyph
```

***

## Running the Interpreter

After building:

```bash
./glyph
```

You will see an interactive prompt (if you kept the sample `main`):

```text
=== Glyph Programming Language Interpreter ===
Valid characters: * ( ) + - ^ % _ :

> _       # press Enter
=> 1
```

Exit with `Ctrl+C` or by closing the terminal.

***

## Example Programs

The interpreter treats input lines as single Glyph expressions.

### Basic Arithmetic

```text
_          ; 1

(+ _ _)    ; 1 + 1 = 2

(+ (+ _ _) _) ; 3

(* (+ _ _) (+ _ _)) ; 2 * 2 = 4

(^ (+ _ _) (+ _ _)) ; 2 ^ 2 = 4
```

### Derived “Short” Style

Depending on how you choose to write them (still subject to proper parentheses if needed):

```text
_          ; 1
+_ _       ; 2
++ _       ; 3
^+_ _ _    ; 4 (conceptually)
```

The interpreter itself expects valid parenthesized prefix expressions; you can adapt your input style to match precisely how you encode operator applications.

### Conditionals

A simple conditional like:

```text
%(cond)(then)(else)
```

For example, “if 0 then 1 else 2”:

```text
%( _ - _ _ )( _ + _ _ )( + _ _ )
```

You can encode `0` via arithmetic (e.g., `(- _ _)` → 0) and then choose branches accordingly.

***

## Design Notes

- **Tiny alphabet**  
  Only 9 characters are legal, which reduces syntactic complexity and ambiguity.

- **Expression-only**  
  There are no statements; everything is an expression with a value.

- **Prefix notation**  
  Binary operators come first, followed by their operands.

- **No identifiers in the usual sense**  
  Names are expression-based; the interpreter uses evaluated “name” expressions as indices in an environment map.

- **Pure and deterministic**  
  No side effects, no mutation of shared state, no I/O intruding into evaluation.

This makes Glyph particularly suitable as a target for AI-based code generation, transformation, and analysis, where extremely low syntactic entropy and fully deterministic semantics are desirable.

***

## Extending the Interpreter

Some ideas for future work:

- Make function values explicit instead of treating `*` solely as arithmetic multiplication.
- Add a small standard library encoded in Glyph itself.
- Implement different numeric backends (e.g., big integers).
- Build tooling around Glyph (e.g., visualizers, debuggers, optimizers).

***

## License

Add your preferred license here, for example:

```text
The MIT License (MIT)

Copyright © 2026 


Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the “Software”), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
```