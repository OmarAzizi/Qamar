#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chunk.h"
#include "compiler.h"
#include "object.h"
#include "common.h"
#include "scanner.h"
#include "value.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

typedef struct {
    Token current;
    Token previous;
    bool  hadError;
    bool  panicMode;
} Parser;

/* Precedence levels as an Enum */
typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT,  // =
    PREC_OR,          // or
    PREC_AND,         // and
    PREC_EQUALITY,    // == !=
    PREC_COMPARISON,  // < > <= >=
    PREC_TERM,        // + -
    PREC_FACTOR,      // * /
    PREC_UNARY,       // ! -
    PREC_CALL,        // . ()
    PREC_PRIMARY
} Precedence;

typedef void (*ParseFn)(bool canAssign); /* This is a typedef for a function that takes no parameters and returns nothing */

/* This struct represents a single row in the parser table */
typedef struct {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

typedef struct {
    Token name;
    int  depth;  /* records the scope depth of the block where the local variable was declared */
} Local;

/* This lets the compiler tell when it’s compiling top-level code versus the body of a function */
typedef enum {
    TYPE_FUNCTION,
    TYPE_SCRIPT
} FunctionType;

typedef struct Compiler {
    struct Compiler* enclosing; /* Each compiler points bacj to the compiler fo the function that encloses it all the way back to the root Compiler for top-level code */

    ObjFunction* function;
    FunctionType type;

    Local locals[UINT8_COUNT];  /* Simple array of all locals that are in scope during each point in the compilation */
    int localCount;             /* Tracks how many locals are in scope*/
    int scopeDepth;             /* The number of bits surrounding the current but we are compiling */
} Compiler;

Parser parser;
Compiler* current = NULL;
Chunk* compilingChunk;

static Chunk* currentChunk() { 
/* 
    Every place in the compiler that was writing to the Chunk now needs to go through that function pointer 
    The current chunk is always the chunk owned by the function we're in the middle of compiling
*/
    return &current->function->chunk;
}

static void errorAt(Token* token, const char* message) {
    if (parser.panicMode) return;
    parser.panicMode = true;
    fprintf(stderr, "[line %d] Error", token->line);

    if (token->type == TOKEN_EOF) fprintf(stderr, " at end");
    else if (token->type == TOKEN_ERROR) { /* Nothing */ }
    else fprintf(stderr, " at '%.*s'", token->length, token->start);

    fprintf(stderr, ": %s\n", message);
    parser.hadError = true;
}

static void error(const char* message) {
    errorAt(&parser.previous, message);
}

static void errorAtCurrent(const char* message) { 
    errorAt(&parser.current, message); 
}

static void advance() {
    parser.previous = parser.current;

    for (;;) {
        parser.current = scanToken();
        if (parser.current.type != TOKEN_ERROR) break;

        errorAtCurrent(parser.current.start);
    }
}

/*
    It’s similar to 'advance' in that it reads the next token. 
    But it also validates that the token has an expected type.
*/
static void consume(TokenType type, const char* message) {
    if (parser.current.type == type) {
        advance();
        return;
    }
    errorAtCurrent(message);
}

static bool check(TokenType type) {
    return parser.current.type == type;
}

static bool match(TokenType type) {
    if (!check(type)) return false;
    advance();
    return true;
}

static void emitByte(uint8_t byte) {
    writeChunk(currentChunk(), byte, parser.previous.line);
}

static void emitBytes(uint8_t byte1, uint8_t byte2) {
    emitByte(byte1);
    emitByte(byte2);
}

/*
    It’s a bit like `emitJump` and `patchJump` combined. It emits a new loop instruction, which unconditionally jumps backwards by a given offset.
*/
static void emitLoop(int loopStart) {
    emitByte(OP_LOOP);

    int offset = currentChunk()->count - loopStart + 2;
    if (offset > UINT16_MAX) error("Loop body too large.");
    
    emitByte((offset >> 8) & 0xFF);
    emitByte(offset & 0xFF);
}

/*
    The `emitJump` function reserves space for the jump offset and returns 
    the index of the first byte of the emitted jump instruction.
*/
static int emitJump(uint8_t instruction) {
    emitByte(instruction);
    emitByte(0xFF);
    emitByte(0xFF);
    return currentChunk()->count - 2; /* Subracting 2 to give us the index of the first byte of the jump instruction */
}

static void emitReturn() { 
    emitByte(OP_NIL); /* In case we were returning a function that returns nothing */
    emitByte(OP_RETURN); 
}

static uint8_t makeConstant(Value value) {
    int constant = addConstant(currentChunk(), value);
    if (constant > UINT8_MAX) {
        error("Too many constants in one chunk.");
        return 0;
    }
    return (uint8_t)constant;
}

static void emitConstant(Value value) {
    emitBytes(OP_CONSTANT, makeConstant(value));
}

static void patchJump(int offset) {
    int jump = currentChunk()->count - offset - 2;
    if (jump > UINT16_MAX) {
        error("Too much code to jump over.");
    }
    currentChunk()->code[offset] = (jump >> 8) & 0xFF;
    currentChunk()->code[offset + 1] = jump & 0xFF;
}

static void initCompiler(Compiler* compiler, FunctionType type) {
    /* Initialize the new Compiler fields */

    compiler->enclosing = current; /* When initializing a new Compiler, we capture the about-to-no-longer-be-current one in that pointer. */
    compiler->function = NULL;
    compiler->type = type;

    compiler->localCount = 0;
    compiler->scopeDepth = 0;
    
    compiler->function = newFunction(); /* Then we allocate a new function object to compile into */

    current = compiler;

    if (type != TYPE_SCRIPT) {
        current->function->name = copyString(parser.previous.start, parser.previous.length);
    }

/*
     compiler’s locals array keeps track of which stack slots are associated with which local variables or temporaries. 
     From now on, the compiler implicitly claims stack slot zero for the VM’s own internal use. We give it an empty name so 
     that the user can’t write an identifier that refers to it.
*/
    Local* local = &current->locals[current->localCount++];
    local->depth = 0;
    local->name.start = "";
    local->name.length = 0;
}

static ObjFunction* endCompiler() { 
    emitReturn();

/*
    Previously, when `interpret()` called into the compiler, it passed in a Chunk to be written to. 
    Now that the compiler creates the function object itself, we return that function.
*/
    ObjFunction* function = current->function;

#ifdef DEBUG_PRINT_CODE
    if(!parser.hadError) {
        disassembleChunk(currentChunk(), function->name != NULL ? function->name->chars : "<script>");
    }
#endif
    
    current = current->enclosing; /* When a Compiler finishes, it pops itself off the stack */
    return function;
}

static void beginScope() {
    /* In order to “create” a scope, all we do is increment the current depth. */
    ++current->scopeDepth;
}

static void endScope() {
    --current->scopeDepth;

    /* Deleting (discarding) the local variables in a specific scope aftr it ends */
    while (current->localCount > 0 && current->locals[current->localCount - 1].depth > current->scopeDepth) {
        emitByte(OP_POP);
        --current->localCount;
    }
}

static void expression();
static void statement();
static void declaration();
static ParseRule* getRule(TokenType type);
static void parsePrecedence(Precedence precedence);
static uint8_t identifierConstant(Token* name);
static uint8_t parseVariable(const char* errorMessage);
static void defineVariable(uint8_t global);
static int resolveLocal(Compiler* compiler, Token* name);
static void and_(bool canAssign);
static void markInitialized();
static uint8_t argumentList();

static void binary(bool canAssign) {
    TokenType operatorType = parser.previous.type;
    ParseRule* rule = getRule(operatorType);
    parsePrecedence((Precedence)(rule->precedence + 1));

    switch (operatorType) {
        case TOKEN_BANG_EQUAL:      emitBytes(OP_EQUAL, OP_NOT); break;
        case TOKEN_EQUAL_EQUAL:     emitByte(OP_EQUAL); break;
        case TOKEN_GREATER:         emitByte(OP_GREATER); break;
        case TOKEN_GREATER_EQUAL:   emitBytes(OP_LESS, OP_NOT); break;
        case TOKEN_LESS:            emitByte(OP_LESS); break;
        case TOKEN_LESS_EQUAL:      emitBytes(OP_GREATER, OP_NOT); break;
        case TOKEN_PLUS:            emitByte(OP_ADD); break;
        case TOKEN_MINUS:           emitByte(OP_SUBTRACT); break;
        case TOKEN_STAR:            emitByte(OP_MULTIPLY); break;
        case TOKEN_SLASH:           emitByte(OP_DIVIDE); break;
        default:                    return; // Unreachable
    }
}

static void call(bool canAssign) {
    /* We’ve already consumed the ( token, so next we compile the arguments using a separate `argumentList()` helper. */
    uint8_t argCount = argumentList();
    emitBytes(OP_CALL, argCount);
}

/*
    When the parser encouters false, nil, or true it calls this new parser function
*/
static void literal(bool canAssign) {
    switch (parser.previous.type) {
        case TOKEN_FALSE:           emitByte(OP_FALSE); break;
        case TOKEN_NIL:             emitByte(OP_NIL); break;
        case TOKEN_TRUE:            emitByte(OP_TRUE); break;
        default:                    return; // Unreachable
    }
}

static void expression() {
    parsePrecedence(PREC_ASSIGNMENT);
}

static void block() {
    /* This keeps parsing declarations and statements untill it his a closing brace or enf of file token */
    while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)) {
        declaration();
    }
    consume(TOKEN_RIGHT_BRACE, "Expect '}' after block.");
}

static void function(FunctionType type) {
    Compiler compiler;
    initCompiler(&compiler, type);
    beginScope();

    consume(TOKEN_LEFT_PAREN, "Expect '(' after function name.");
    
    /* Compiling the function parameters */
    if (!check(TOKEN_RIGHT_PAREN)) {
        do {
            current->function->arity++;
            if (current->function->arity > 255) {
                errorAtCurrent("Can't have more than 255 parameters.");
            }
            uint8_t constant = parseVariable("Expect parameter name.");
            defineVariable(constant);
        } while (match(TOKEN_COMMA));
    }

    consume(TOKEN_RIGHT_PAREN, "Expect ')' after parameters.");
    consume(TOKEN_LEFT_BRACE, "Expect '{' before function body.");
    block();

    ObjFunction* function = endCompiler();
    emitBytes(OP_CONSTANT, makeConstant(OBJ_VAL(function)));
}

static void funDeclaration() {
/*
    Functions are first-class values, and a function declaration simply creates and stores one in a newly declared variable. 
    So we parse the name just like any other variable declaration.

    A function declaration at the top level will bind the function to a global variable. Inside a block or other function, 
    a function declaration creates a local variable.
*/
    uint8_t global = parseVariable("Expect function name.");
    markInitialized(); /* Marking function as initialized before we compile the body. That way the name can be referenced inside the body without generating errors */
    function(TYPE_FUNCTION);
    defineVariable(global);
}

static void varDecleration() {
    /* The `var` keyword is followed by a variable name that's compiled by `parseVariable` */
    uint8_t global = parseVariable("Expect variable name.");

/*
    Then we look for an = followed by an initializer expression. If the user doesn’t initialize the variable, 
    the compiler implicitly initializes it to nil by emitting an OP_NIL instruction.
*/
    if (match(TOKEN_EQUAL)) {
        expression();
    } else {
        emitByte(OP_NIL);
    }

    consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration."); /* statement should be terminated using a semicolon */
    defineVariable(global);
}

/*
    An "expression statement" is simply an expression followed by a semicolon
    Semanitcally, an expression statement evaluates the exptession and discard the results.
*/
static void expressionStatement() {
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after expression.");
    emitByte(OP_POP); /* Discarding the results */
}

static void forStatement() {
    beginScope(); /* If a for statement declares a variable, that variable should be scoped to the loop body. We ensure that by wrapping the whole statement in a scope. */
    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'for'.");
    
    /* Initializer clause */
    if (match(TOKEN_SEMICOLON)) {
        /* No initializer*/
    } else if (match(TOKEN_VAR)) {
        varDecleration();
    } else {
        expressionStatement();
    }

    int loopStart = currentChunk()->count; /* points (address of) at the condition clause */
    int exitJump = -1;
   
    /* Next, is the condition clause/expression that can be used to exit the loop */
    if (!match(TOKEN_SEMICOLON)) {
        expression();
        consume(TOKEN_SEMICOLON, "Expect ';' after loop condition.");

        /* Jump out of the loop if the condition is false */
        exitJump = emitJump(OP_JUMP_IF_FALSE);
        emitByte(OP_POP); /* Discard the value of condition if it was true */
    }
     
    if (!match(TOKEN_RIGHT_PAREN)) {
        int bodyJump = emitJump(OP_JUMP);
        int incrementStart = currentChunk()->count; /* The address of the increment clause */
        expression(); /* `incrementStart` points here */
        emitByte(OP_POP);
        consume(TOKEN_RIGHT_PAREN, "Expect ')' after for clauses.");

        emitLoop(loopStart);
        loopStart = incrementStart;
        patchJump(bodyJump);
    }

    statement(); /* Body statement */
    emitLoop(loopStart);

    /* After the loop body we need to patch that jump */
    if (exitJump != -1) {
        patchJump(exitJump);
        emitByte(OP_POP);
    }

    endScope();
}

static void ifStatement() {
    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'if'.");
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

    int thenJump = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);
    statement();
    
    int elseJump = emitJump(OP_JUMP);   /* Emitting a jump after the `if` statement body so we wont fall through and execute the else branch if the `if` condition was true */

    patchJump(thenJump);
    emitByte(OP_POP);

    if (match(TOKEN_ELSE)) statement(); /* Checking for else statement */
    patchJump(elseJump);
}

/*
    if we did match the `print` token, then we compile the rest of the statement here.
*/
static void printStatement() {
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after value.");
    emitByte(OP_PRINT); 
}

static void whileStatement() {
    int loopStart = currentChunk()->count;

    /* Compile the condition expression with the paranthses */
    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'while'.");
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

    int exitJump = emitJump(OP_JUMP_IF_FALSE); /* Exiting the loop if the condition was false */
    emitByte(OP_POP);
    statement(); /* compiling the body of the `while` */

    /* After the body, we call this function to emit a “loop” instruction. That instruction needs to know how far back to jump. */
    emitLoop(loopStart);

    patchJump(exitJump);
    emitByte(OP_POP);
}

/*
    If we hit a compile error while parsing the previous statement, we enter panic mode. 
    When that happens, after the statement we start synchronizing
*/
static void synchronize() {
    parser.panicMode = false;

    while (parser.current.type != TOKEN_EOF) {

/*
        We skip tokens indiscriminately until we reach something that looks like a statement boundary.
*/
        if (parser.previous.type == TOKEN_SEMICOLON) return;
        switch (parser.current.type) {
            case TOKEN_CLASS:
            case TOKEN_FUN:
            case TOKEN_VAR:
            case TOKEN_FOR:
            case TOKEN_IF:
            case TOKEN_WHILE:
            case TOKEN_PRINT:
            case TOKEN_RETURN:
                return;

            default: 
                ; // Do nothing
        }
        advance();
    }
}

static void declaration() {
    if (match(TOKEN_FUN)) {
        funDeclaration();
    } else if (match(TOKEN_VAR)) {
        varDecleration(); 
    } else {
        statement();
    }

    if (parser.panicMode) synchronize();
}

static void statement() {
    if (match(TOKEN_PRINT)) {
        printStatement();
    } else if (match(TOKEN_FOR)) {
        forStatement();
    } else if (match(TOKEN_IF)) {
        ifStatement();
    } else if (match(TOKEN_WHILE)) {
        whileStatement();
    } else if (match(TOKEN_LEFT_BRACE)) {
        beginScope();
        block();
        endScope();
    } else {
        expressionStatement();
    }
}

static void grouping(bool canAssign) {
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

static void number(bool canAssign) {
    double value = strtod(parser.previous.start, NULL);
    emitConstant(NUMBER_VAL(value));
}

static void or_(bool canAssign) {
/*
    In an or expression, if the left-hand side is truthy, then we skip over the right operand.

    When the left-hand side is falsey, it does a tiny jump over the next statement. 
    That statement is an unconditional jump over the code for the right operand. 
*/
    int elseJump = emitJump(OP_JUMP_IF_FALSE);
    int endJump = emitJump(OP_JUMP);
    
    patchJump(elseJump);
    emitByte(OP_POP);

    parsePrecedence(PREC_OR);
    patchJump(endJump);
}
 
static void string(bool canAssign) {
    emitConstant(OBJ_VAL(copyString(parser.previous.start + 1, parser.previous.length - 2)));
}

static void namedVariable(Token name, bool canAssign) {
    uint8_t getOp, setOp;
    int arg = resolveLocal(current, &name); /* First we try to find a local variable with the given name */

    if (arg != -1) {
        /* If we found a local we use the instructions for working with locals */
        getOp = OP_GET_LOCAL;
        setOp = OP_SET_LOCAL;
    } else {
        /* Otherwise its a global */
        arg = identifierConstant(&name);
        getOp = OP_GET_GLOBAL;
        setOp = OP_SET_GLOBAL;
    }

    if (canAssign && match(TOKEN_EQUAL)) {
        expression();
        emitBytes(setOp, (uint8_t)arg);
    } else {
        emitBytes(getOp, (uint8_t)arg);
    }
}

static void variable(bool canAssign) {
    namedVariable(parser.previous, canAssign);
}

static void unary(bool canAssign) {
    TokenType operatorType = parser.previous.type;
     
    // Compile the operand
    parsePrecedence(PREC_UNARY);

    // Emit the operator instruction
    switch (operatorType) {
        case TOKEN_BANG:            emitByte(OP_NOT); break;
        case TOKEN_MINUS:           emitByte(OP_NEGATE); break;
        default:                    return; // Unreachable
    }
}

/* The table that drives our whole parser is an array of ParseRules */
ParseRule rules[] = {
    [TOKEN_LEFT_PAREN]    = {grouping,  call,         PREC_CALL},
    [TOKEN_RIGHT_PAREN]   = {NULL,      NULL,         PREC_NONE},
    [TOKEN_LEFT_BRACE]    = {NULL,      NULL,         PREC_NONE}, 
    [TOKEN_RIGHT_BRACE]   = {NULL,      NULL,         PREC_NONE},
    [TOKEN_COMMA]         = {NULL,      NULL,         PREC_NONE},
    [TOKEN_DOT]           = {NULL,      NULL,         PREC_NONE},
    [TOKEN_MINUS]         = {unary,     binary,       PREC_TERM},
    [TOKEN_PLUS]          = {NULL,      binary,       PREC_TERM},
    [TOKEN_SEMICOLON]     = {NULL,      NULL,         PREC_NONE},
    [TOKEN_SLASH]         = {NULL,      binary,     PREC_FACTOR},
    [TOKEN_STAR]          = {NULL,      binary,     PREC_FACTOR},
    [TOKEN_BANG]          = {unary,     NULL,         PREC_NONE},
    [TOKEN_BANG_EQUAL]    = {NULL,      binary,   PREC_EQUALITY},
    [TOKEN_EQUAL]         = {NULL,      NULL,         PREC_NONE},
    [TOKEN_EQUAL_EQUAL]   = {NULL,      binary,   PREC_EQUALITY},
    [TOKEN_GREATER]       = {NULL,      binary, PREC_COMPARISON},
    [TOKEN_GREATER_EQUAL] = {NULL,      binary, PREC_COMPARISON},
    [TOKEN_LESS]          = {NULL,      binary, PREC_COMPARISON},
    [TOKEN_LESS_EQUAL]    = {NULL,      binary, PREC_COMPARISON},
    [TOKEN_IDENTIFIER]    = {variable,  NULL,         PREC_NONE},
    [TOKEN_STRING]        = {string,    NULL,         PREC_NONE},
    [TOKEN_NUMBER]        = {number,    NULL,         PREC_NONE},
    [TOKEN_AND]           = {NULL,      and_,          PREC_AND},
    [TOKEN_CLASS]         = {NULL,      NULL,         PREC_NONE},
    [TOKEN_ELSE]          = {NULL,      NULL,         PREC_NONE},
    [TOKEN_FALSE]         = {literal,   NULL,         PREC_NONE},
    [TOKEN_FOR]           = {NULL,      NULL,         PREC_NONE},
    [TOKEN_FUN]           = {NULL,      NULL,         PREC_NONE},
    [TOKEN_IF]            = {NULL,      NULL,         PREC_NONE},
    [TOKEN_NIL]           = {literal,   NULL,         PREC_NONE},
    [TOKEN_OR]            = {NULL,      or_,            PREC_OR},
    [TOKEN_PRINT]         = {NULL,      NULL,         PREC_NONE},
    [TOKEN_RETURN]        = {NULL,      NULL,         PREC_NONE},
    [TOKEN_SUPER]         = {NULL,      NULL,         PREC_NONE},
    [TOKEN_THIS]          = {NULL,      NULL,         PREC_NONE},
    [TOKEN_TRUE]          = {literal,   NULL,         PREC_NONE},
    [TOKEN_VAR]           = {NULL,      NULL,         PREC_NONE},
    [TOKEN_WHILE]         = {NULL,      NULL,         PREC_NONE},
    [TOKEN_ERROR]         = {NULL,      NULL,         PREC_NONE},
    [TOKEN_EOF]           = {NULL,      NULL,         PREC_NONE},
};

/*
    This function starts at the current token and recursively parses any expression at the given precedence level or higher
*/
static void parsePrecedence(Precedence precedence) {
    advance();
    ParseFn prefixRule = getRule(parser.previous.type)->prefix;
    if (prefixRule == NULL) {
        error("Expect expression.");
        return;
    }
    bool canAssign = precedence <= PREC_ASSIGNMENT;
    prefixRule(canAssign);

    while (precedence <= getRule(parser.current.type)->precedence) {
        advance();
        ParseFn infixRule = getRule(parser.previous.type)->infix;
        infixRule(canAssign);
    }

    if (canAssign && match(TOKEN_EQUAL)) {
        error("Invalid assignment target.");
    }
}

/*
    This function takes the given token and adds its lexeme to the chunk’s constant table as a string. It then returns the index of that constant in the constant table.
*/
static uint8_t identifierConstant(Token* name) {
    return makeConstant(OBJ_VAL(copyString(name->start, name->length)));
}

static bool identifiersEqual(Token* a, Token* b) {
    if (a->length != b->length) return false;
    return memcmp(a->start, b->start, a->length) == 0;
}

/*
    We resolve local variables using this function
*/
static int resolveLocal(Compiler* compiler, Token* name) {
    for (int i = compiler->localCount - 1; i >= 0; --i) {
        Local* local = &compiler->locals[i];
        if (identifiersEqual(name, &local->name)) {
            /* When we resolve a reference to a local variable, we check the scope depth to see if it’s fully defined. */
            if (local->depth == -1) 
                error("Can't read local variable in its own initializer.");
           return i; 
        }
    }
    return -1;
}

/*  
    To see if two identifiers are the same, we use this function.
*/
static void addLocal(Token name) {
    if (current->localCount == UINT8_COUNT) {
        error("Too many local variables in function.");
        return;
    }

    Local* local = &current->locals[current->localCount++];
    local->name = name;
    local->depth = -1; /* -1 indicates uninitialized state of the variable */
}

/*
    This initializes the next available Local in the compiler’s array of variables. It stores the variable’s name and the depth of the scope that owns the variable.
*/
static void declareVariable() {
    if (current->scopeDepth == 0) return;

    Token* name = &parser.previous;
    for (int i = current->localCount - 1; i >= 0; --i) {
        Local* local = &current->locals[i];
        if (local->depth != -1 && local->depth < current->scopeDepth)
            break;
        if (identifiersEqual(name, &local->name))
            error("Redeclaration of the variable at the same scope.");
    }
    addLocal(*name);
}

static uint8_t parseVariable(const char* errorMessage) {
    consume(TOKEN_IDENTIFIER, errorMessage);
    
    declareVariable(); /* Declare the variable */
    if (current->scopeDepth > 0) return 0; /* we exit the function if we’re in a local scope and return a dummy index */ 

    return identifierConstant(&parser.previous);
}

/*
    Once the variable’s initializer has been compiled, we mark it initialized by changing the depth from `-1` to the current scope depth.
*/
static void markInitialized() {
    if (current->scopeDepth == 0) return;
    current->locals[current->localCount - 1].depth = current->scopeDepth;
}

static void defineVariable(uint8_t global) {
    if (current->scopeDepth > 0) {
        markInitialized();
        return;
    }
    emitBytes(OP_DEFINE_GLOBAL, global);
}

static uint8_t argumentList() {
    uint8_t argCount = 0;
    if (!check(TOKEN_RIGHT_PAREN)) {
        do {
        /* 
            Each argument expression generates code that leaves its value on the stack in preparation for the call 
        */
            expression(); 
            if (argCount == 255) {
                error("Can't have more than 255 arguments.");
            }
            argCount++;
        } while (match(TOKEN_COMMA));
    }
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after arguments.");
    return argCount;
}

static void and_(bool canAssign) {
/*
    he left-hand side expression has already been compiled. That means at runtime, its value will be on top of the stack. 
    If that value is falsey, then we know the entire and must be false, so we skip the right operand and leave the left-hand 
    side value as the result of the entire expression.

    Otherwise, we discard the left-hand value and evaluate the right operand which becomes the result of the whole and expression.
*/
    int endJump = emitJump(OP_JUMP_IF_FALSE);

    emitByte(OP_POP);
    parsePrecedence(PREC_AND);

    patchJump(endJump);
}

static ParseRule* getRule(TokenType type) {
    return &rules[type];
}

ObjFunction* compile(const char* source) {
    initScanner(source);
    Compiler compiler;
    initCompiler(&compiler, TYPE_SCRIPT);

    parser.hadError = false;
    parser.panicMode = false;

    advance();
   
    /* We keep compiling declerations until we hit the end of a source file */
    while (!match(TOKEN_EOF)) {
        declaration();
    }

    ObjFunction* function = endCompiler();
    return parser.hadError ? NULL : function;
}
