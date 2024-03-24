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

### Virtual Machine

