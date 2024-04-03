# Onyx
A while ago I went through the first part of [Crafting Interpreters](https://craftinginterpreters.com/) and implemented **Lox** in Java as the book did and shared it on GitHub in [MyLox Repository](https://github.com/OmarAzizi/MyLox), but this was only the first part of the book.

## Why Onyx?
The first part of the book mainly focused on teaching the reader how a programming language might be implemented, so it neglected the low-level details (e.g. Code generation & Virtual machines). 

So the language implemented in the first part is heavily dependent on JVM because it was [Transpilied](https://en.wikipedia.org/wiki/Source-to-source_compiler) (in other words source-to-source compiler). Despite being an easy approach it is not efficient.

## About This Repository
In this repository, I will be going through the second part of the book (starting at **chapter 14**), which implements the same language in the first part, but this time it does it in C and it implements a Virtual machine similar to JDK, and try to document my journey.

## My Current Progress
### Chunks of Bytecode
So far, I've implemented the Bytecode along with a couple of instructions. Additionally, I created a simple disassembler that takes those chunks and disassembles them for debugging purposes. This photo shows a hardcoded example in main for the disassembly of the instructions:

![image](https://github.com/OmarAzizi/clox/assets/110500643/a3101208-9e3b-4ab4-8bca-00531782d7f6)

### A Virtual Machine
I have also developed the Virtual Machine upon which the interpreter operates. Additionally, I have introduced new Bytecode instructions supported by a disassembler and integrated them into the interpreter loop. As a result, I am now able to evaluate complex arithmetic expressions seamlessly. This screenshot shows a disassembled chunk of bytecode calculating the expression `-((1.2 + 3.4) / 5.6)`

![image](https://github.com/OmarAzizi/clox/assets/110500643/cba5db4c-2a1f-4a9a-b25d-beb85acc580c)

### Scanning (Lexing)
After completing a significant portion of the backend for the compiler, I have now begun working on the frontend. Today, I focused on the initial component essential for a compiler's frontend: the Scanner (also known as the Lexer). I successfully implemented all the lexical grammars, resulting in an artifact capable of processing source code and generating corresponding tokens.
<br><br>
This screenshot shows the output of the lexer where:
- `1` is the line number
- `|` means that this token is also at line 1
- `36` being `TOKEN_VAR` defined in the `TokenType` Enum in [scanner.h](https://github.com/OmarAzizi/clox/blob/main/scanner.h)
- `19`, `13`, `21`, `10`, `8`, and `39` being `TOKEN_IDENTIFIER`, `TOKEN_EQUAL`, `TOKEN_NUMBER`, `TOKEN_STAR`, `TOKEN_SEMICOLON`, and `TOKEN_EOF` respectively

![image](https://github.com/OmarAzizi/clox/assets/110500643/97d4fd60-d601-4162-af51-2d7edd77d56d)

### Compiling Expressions
Today, I completed the final segment of the VM's execution pipeline and developed a compiler capable of parsing high-level source code using Vaughan Pratt’s “top-down operator precedence parsing” technique. The compiler outputs a series of low-level instructions (Bytecode instructions) that can be executed by the VM. The types of instructions I can parse and execute include:
- Number literals
- Parentheses for grouping
- Unary Expressions
- The Four Horsemen of the Arithmetic +, -, *, /

So basically now I've got myself an over-engineered calculator

![image](https://github.com/OmarAzizi/clox/assets/110500643/1040976c-dd01-43de-b2db-dabcbf589f79)

### New Datatypes and Operators
So far, our language has only had one data type (double), represented as the `typedef` **Value**. However, in the past few days, I've been working on implementing new data types. I successfully added three new data types: `NIL`, `BOOL`, and `STRING`. Additionally, I've included some helper macros to facilitate type-checking and conversion between the language's representation of these data types and their C counterparts.

![image](https://github.com/OmarAzizi/clox/assets/110500643/7fdcbeb9-b64f-40fb-bc3e-1bbf8aa241a2)

Additionally, I've introduced several new operators to handle operations associated with the new data types. These include logical operators such as `<`, `<=`, `>`, and `==`, among others. Furthermore, I've implemented **string concatenation** using the `+` operator.

![image](https://github.com/OmarAzizi/clox/assets/110500643/c0b671e9-17da-4b5e-9ee4-3c1c6b8137b2)

### Global Variables
