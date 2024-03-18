# clox
A while ago I implemented Lox language from the book [Crafting Interpreters](https://craftinginterpreters.com/) in Java and shared it on [MyLox Repository](https://github.com/OmarAzizi/MyLox), but this was only the first part of the book.

## Why clox?
The first part of the book mainly focused on teaching the reader how a programming language might be implemented, so it neglected the low-level details (.e.g Code generation & Virtual machines). 

So the language implemented in the first part is heavily dependant on JVM, because it was [Transpilied](https://en.wikipedia.org/wiki/Source-to-source_compiler) (in other words source-to-source compiler). Despite being an easy approach it is not efficent.

## About This Repository
In this repositry I will be going through the second part of the book (starting at **chapter 14**), which implements the same language in the first part, but this time it does it in C and it implements a Virtual machine similar to JDK, and try to document my journey.
