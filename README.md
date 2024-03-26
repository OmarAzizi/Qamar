# clox
A while ago I went through the first part of [Crafting Interpreters](https://craftinginterpreters.com/) and implemented **Lox** in Java as the book did and shared it on GitHub in [MyLox Repository](https://github.com/OmarAzizi/MyLox), but this was only the first part of the book.

## Why clox?
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

![image](https://github.com/OmarAzizi/clox/assets/110500643/f1934414-1bff-4fd1-ad9e-14168814a8e7)
