# Qamar
A while ago I went through the first part of [Crafting Interpreters](https://craftinginterpreters.com/) and implemented **Lox** in Java as the book did and shared it on GitHub in [MyLox Repository](https://github.com/OmarAzizi/MyLox), but this was only the first part of the book.

## Why Qamar?
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
After working on the hash table implementation for the language, I utilized it to implement and support global variables. This allows us to define variables using the keyword `var`, assign them values, and subsequently modify their values. The retrieval of variables is entirely facilitated through the hash table mechanism.

![image](https://github.com/OmarAzizi/Onyx/assets/110500643/14083a3f-cb7d-4eb3-a4e3-2e6b1d16be71)

### Local Variables & Lexical Scoping
So far, all variables supported by the language have been global. Recently, I focused on extending the language to incorporate blocks, block scopes, and local variables. Additionally, I introduced a completely new runtime representation for variables using the new struct `Compiler` that keeps track of the scope depth, and the number of the local variables in that scope.

So given this example test program provided in [block.ox](https://github.com/OmarAzizi/Onyx/blob/main/tests/block.ox)

![image](https://github.com/OmarAzizi/Onyx/assets/110500643/73b9704e-5d18-47e0-96c0-3d6e2df3fd86)

If we run the code it will produce the following output as expected

![image](https://github.com/OmarAzizi/Onyx/assets/110500643/c6cf395a-a62d-4f03-b8b3-b01aab95c208)

```
Our programming language is finally starting to look like a legitimate programming language.
```

### `if` & `if else` Statements
I don't feel like typing a lot now, but I implemented `if` statements and `if else`, so given this program in [if_statement.ox](https://github.com/OmarAzizi/Onyx/blob/main/tests/if_statement.ox):

![image](https://github.com/OmarAzizi/Onyx/assets/110500643/217714f3-3109-4f51-8457-e88c1d75bdcb)

It produces the following output:

![image](https://github.com/OmarAzizi/Onyx/assets/110500643/696928bc-436b-47db-a24e-0dbb8d8ad910)

### `while` loops
There's not much to explain here except for the fact that we've added a new feature: looping, specifically while loops, along with a new test file that contains a code snippet to verify its functionality [while_loop.ox](https://github.com/OmarAzizi/Onyx/blob/main/tests/while_loop.ox) that counts down from 10 to 1

![image](https://github.com/OmarAzizi/Onyx/assets/110500643/f131e4cb-0628-4a48-beb6-0939df855924)

It produces the following output:

![image](https://github.com/OmarAzizi/Onyx/assets/110500643/00b742c6-f508-4005-a3d0-e81c3880912c)

### `for` loops
This is an example of a `for` loop that prints even numbers from **1** through **10** provided in [for_loop.ox](https://github.com/OmarAzizi/Onyx/blob/main/tests/for_loop.ox):

![image](https://github.com/OmarAzizi/Onyx/assets/110500643/4a124fab-4566-47b6-983b-8de32bbb4e1c)

It produces the following output:

![image](https://github.com/OmarAzizi/Onyx/assets/110500643/fbf18df7-16c0-4f64-9cf7-2785201f99d6)

With all of these control structures in place, Onyx is finally considered to be a Turing-complete programming language.

### Functions & Recursion
I have implemented support for defining and calling functions. Additionally, I have incorporated Native Functions, which are functions defined in C that can be accessed by users (similar to standard library functions). As an example, I have implemented a `clock` function that provides the elapsed time since the program started. The following example is provided in [fib.ox](https://github.com/OmarAzizi/Onyx/blob/main/tests/fib.ox):

![image](https://github.com/OmarAzizi/Onyx/assets/110500643/cd57da57-f4ac-44ee-aed0-12b495d39b26)

Running it produces the following:

![image](https://github.com/OmarAzizi/Onyx/assets/110500643/293e9a35-71bc-450e-b56a-adbab9734f4d)

### More Native Functions
Given this program in [native.qmr](https://github.com/OmarAzizi/Qamar/blob/main/tests/native.qmr):

![image](https://github.com/OmarAzizi/Qamar/assets/110500643/876b0dcd-e40c-467c-beec-65746b147b0d)

It produces the following output:

![image](https://github.com/OmarAzizi/Qamar/assets/110500643/49b567fe-34cb-41ae-9c0b-8cd3de588280)


