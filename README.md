# clox
A while ago I implemented Lox language from the book [Crafting Interpreters](https://craftinginterpreters.com/) in Java and shared it on [MyLox Repository](https://github.com/OmarAzizi/MyLox), but this was only the first part of the book.

## Why clox?
The first part of the book mainly focused on teaching the reader how a programming language might be implemented, so it neglected the low-level details (.e.g Code generation & Virtual machines). 

So the language implemented in the first part is mainly dependant on JVM, because it was [Transpilied](https://en.wikipedia.org/wiki/Source-to-source_compiler) (in other words source-to-source compiler). Despite being an easy approach it is not efficent.
