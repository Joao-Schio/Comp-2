/*
 * ABI do scanner Flex do TP1. A ordem dos valores deve ser identica a
 * TokenType em external/comp1-2026/microc.flex (e no lex.yy.c fornecido).
 * Este arquivo pertence ao TP2; o submodulo permanece intacto.
 */
#ifndef COMP2_MICROC_TOKENS_H
#define COMP2_MICROC_TOKENS_H

typedef enum {
    UNDEF, ID, END_OF_FILE,
    INTEGERCONST, CHARCONST, STRINGCONST,
    PLUS, MINUS, MUL, DIV, MOD,
    EQ, NEQ, LT, GT, LEQ, GEQ, AND, OR, NOT,
    ASSIGN, SEMICOLON, COMMA, LPAREN, RPAREN,
    LBRACE, RBRACE, LBRACKET, RBRACKET,
    MAIN, IF, ELSE, FOR, RETURN, INT, CHAR, PRINT
} TokenType;

typedef struct {
    TokenType tipo;
    char lexema[100];
    int linha;
} Token;

#endif
