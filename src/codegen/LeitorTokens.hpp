#pragma once

#include <string_view>

namespace compiladores::codegen
{
// Implementacao de entrada --tokens do parser gerado. Este codigo e C89;
// os tipos dos tokens sao fornecidos por tipo_token_por_nome(), emitida
// especificamente para cada gramatica pelo Gerador.
inline constexpr std::string_view leitor_tokens_ansi_c = R"COMP2(
static void erro_arquivo_tokens(int linha_arquivo)
{
    fprintf(stderr, "linha %d do arquivo de tokens: formato invalido\n", linha_arquivo);
    exit(EXIT_FAILURE);
}

static const char *pular_espacos_token(const char *texto)
{
    while (isspace((unsigned char)*texto)) ++texto;
    return texto;
}

/* Consome "campo =" e devolve o inicio do valor. */
static const char *campo_token(const char *texto, const char *campo)
{
    const size_t n = strlen(campo);
    texto = pular_espacos_token(texto);
    if (strncmp(texto, campo, n) != 0) return NULL;
    texto = pular_espacos_token(texto + n);
    if (*texto != '=') return NULL;
    return pular_espacos_token(texto + 1);
}

/* Aceita tanto identificadores do TP1 (ID, INT, ...) quanto inteiros. */
static int tipo_token_texto(const char *texto)
{
    char *fim;
    long numero;
    if (*texto == '-' || *texto == '+' || isdigit((unsigned char)*texto)) {
        errno = 0;
        numero = strtol(texto, &fim, 10);
        if (fim == texto || *fim != '\0' || errno == ERANGE ||
            numero < 0 || numero > INT_MAX) return -1;
        return (int)numero;
    }
    return tipo_token_por_nome(texto);
}

static Token token_de_arquivo(void)
{
    Token t;
    char linha_texto[4096];
    char tipo_texto[128];
    char *fim_lexema, *fim_numero;
    const char *p;
    size_t tamanho, n;
    long numero_linha;
    int tipo;
    static int linha_arquivo = 0;

    memset(&t, 0, sizeof t);
    while (fgets(linha_texto, sizeof linha_texto, arquivo_tokens) != NULL) {
        ++linha_arquivo;
        n = strlen(linha_texto);
        if (n == sizeof linha_texto - 1 && linha_texto[n - 1] != '\n')
            erro_arquivo_tokens(linha_arquivo);
        p = pular_espacos_token(linha_texto);
        if (*p == '\0') continue;
        if (strncmp(p, "Token:", 6) != 0) erro_arquivo_tokens(linha_arquivo);
        p = campo_token(p + 6, "tipo");
        if (p == NULL) erro_arquivo_tokens(linha_arquivo);
        tamanho = strcspn(p, " ,\t\r\n");
        if (tamanho == 0 || tamanho >= sizeof tipo_texto)
            erro_arquivo_tokens(linha_arquivo);
        memcpy(tipo_texto, p, tamanho);
        tipo_texto[tamanho] = '\0';
        tipo = tipo_token_texto(tipo_texto);
        if (tipo < 0) erro_arquivo_tokens(linha_arquivo);
        p = pular_espacos_token(p + tamanho);
        if (*p == ',') ++p;
        p = campo_token(p, "lexema");
        if (p == NULL || *p != '\'') erro_arquivo_tokens(linha_arquivo);
        ++p;
        fim_lexema = strrchr(p, '\'');
        if (fim_lexema == NULL) erro_arquivo_tokens(linha_arquivo);
        tamanho = (size_t)(fim_lexema - p);
        if (tamanho >= sizeof t.lexema) erro_arquivo_tokens(linha_arquivo);
        memcpy(t.lexema, p, tamanho);
        t.lexema[tamanho] = '\0';
        p = pular_espacos_token(fim_lexema + 1);
        if (*p == ',') ++p;
        p = campo_token(p, "linha");
        if (p == NULL) erro_arquivo_tokens(linha_arquivo);
        errno = 0;
        numero_linha = strtol(p, &fim_numero, 10);
        if (fim_numero == p || numero_linha <= 0 || numero_linha > INT_MAX ||
            errno == ERANGE || *pular_espacos_token(fim_numero) != '\0')
            erro_arquivo_tokens(linha_arquivo);
        t.tipo = tipo;
        t.linha = (int)numero_linha;
#if COMP2_TOKEN_POSSUI_COLUNA
        t.coluna = 1;
#endif
        return t;
    }
    if (ferror(arquivo_tokens)) erro_arquivo_tokens(linha_arquivo + 1);
    t.tipo = TOKEN_FIM;
    t.linha = linha_arquivo + 1;
#if COMP2_TOKEN_POSSUI_COLUNA
    t.coluna = 1;
#endif
    return t;
}

)COMP2";
} // namespace compiladores::codegen
