/*
 * Adaptador do scanner original do TP1 para o parser ANSI C gerado.
 * O lex.yy.c do TP1 permanece inalterado, compilado com o seu main
 * renomeado apenas na linha de comando: -Dmain=comp1_lexer_main.
 */
#include "scanner.h"

#include <stdlib.h>
#include <string.h>

/* Simbolos exportados pelo lex.yy.c original, sem prefixo. */
extern int yylex(void);
extern int yylex_destroy(void);
extern FILE *yyin;
extern char *yytext;
extern int yyleng;
extern int linha_atual;

typedef struct {
    char *symbol;
    char *error_msg;
} FlexValor;
extern FlexValor microc_yylval;

static void copiar_lexema(char *destino, const char *origem, size_t capacidade)
{
    if (origem == NULL || capacidade == 0) return;
    strncpy(destino, origem, capacidade - 1);
    destino[capacidade - 1] = '\0';
}

void scanner_iniciar(FILE *entrada)
{
    yyin = entrada;
    linha_atual = 1;
}

Token proximo_token(void)
{
    Token token;
    int codigo;
    const char *texto;
    memset(&token, 0, sizeof token);
    codigo = yylex();
    token.tipo = (TokenType)codigo;
    token.linha = linha_atual;

    if (codigo == END_OF_FILE)
        return token;

    if (codigo == UNDEF)
    {
        texto = microc_yylval.error_msg;
        /* O scanner incrementa a linha ao consumir a quebra que encerra
         * uma string/char invalida; o diagnostico aponta a linha original. */
        if (yyleng > 0 && yytext[yyleng - 1] == '\n' && token.linha > 1)
            --token.linha;
        if (texto != NULL && strlen(texto) == 1)
        {
            /* O erro generico do TP1 devolve o caractere desconhecido. */
            char mensagem[sizeof token.lexema];
            mensagem[0] = '\0';
            strcat(mensagem, "caractere invalido '");
            strncat(mensagem, texto, 1);
            strcat(mensagem, "'");
            copiar_lexema(token.lexema, mensagem, sizeof token.lexema);
            free(microc_yylval.error_msg);
            microc_yylval.error_msg = NULL;
        }
        else copiar_lexema(token.lexema, texto, sizeof token.lexema);
        return token;
    }

    if (codigo == ID || codigo == INTEGERCONST ||
        codigo == CHARCONST || codigo == STRINGCONST)
    {
        texto = microc_yylval.symbol;
        copiar_lexema(token.lexema, texto, sizeof token.lexema);
        free(microc_yylval.symbol);
        microc_yylval.symbol = NULL;
    }
    else copiar_lexema(token.lexema, yytext, sizeof token.lexema);

    return token;
}

void scanner_finalizar(void)
{
    (void)yylex_destroy();
}
