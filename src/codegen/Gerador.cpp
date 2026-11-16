#include "src/codegen/Gerador.hpp"
#include "src/codegen/Regex.hpp"
#include "src/codegen/LeitorTokens.hpp"

#include <algorithm>
#include <cstddef>
#include <format>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace compiladores::codegen
{
namespace
{
[[nodiscard]] std::string string_c(std::string_view texto)
{
    std::string resultado{"\""};
    for (unsigned char c : texto)
    {
        if (c == '"' || c == '\\')
        {
            resultado += '\\';
            resultado += static_cast<char>(c);
        }
        else if (c == '\n') resultado += "\\n";
        else if (c == '\r') resultado += "\\r";
        else if (c == '\t') resultado += "\\t";
        else if (c < 32 || c > 126)
        {
            resultado += std::format("\\{:03o}", static_cast<unsigned int>(c));
        }
        else resultado += static_cast<char>(c);
    }
    resultado += '"';
    return resultado;
}

void linha(std::string& saida, int nivel, std::string_view texto)
{
    saida.append(static_cast<std::size_t>(nivel) * 4, ' ');
    saida.append(texto);
    saida += '\n';
}

// DFA estatico, codificado em condicoes de intervalos. Nao depende de regex POSIX.
void emitir_automato(std::string& saida, const Automato& dfa, std::size_t numero)
{
    linha(saida, 0, std::format("static size_t casa_regex_{}(const char *s)", numero));
    linha(saida, 0, "{");
    linha(saida, 1, "size_t pos = 0, melhor = 0;");
    linha(saida, 1, "int estado = 0;");
    linha(saida, 1, "while (s[pos] != '\\0') {");
    linha(saida, 2, "const int c = (unsigned char)s[pos];");
    linha(saida, 2, "switch (estado) {");
    for (std::size_t i = 0; i < dfa.transicoes.size(); ++i)
    {
        linha(saida, 2, std::format("case {}:", i));
        const auto& transicoes = dfa.transicoes[i];
        bool primeiro = true;
        for (int c = 1; c < 256;)
        {
            const int destino = transicoes[static_cast<std::size_t>(c)];
            if (destino == -1) { ++c; continue; }
            int fim = c;
            while (fim + 1 < 256 && transicoes[static_cast<std::size_t>(fim + 1)] == destino)
                ++fim;
            const auto condicao = c == fim
                ? std::format("c == {}", c)
                : std::format("c >= {} && c <= {}", c, fim);
            linha(saida, 3, std::format("{}if ({}) estado = {};",
                primeiro ? "" : "else ", condicao, destino));
            primeiro = false;
            c = fim + 1;
        }
        linha(saida, 3, primeiro ? "estado = -1;" : "else estado = -1;");
        linha(saida, 3, "break;");
    }
    linha(saida, 2, "default: estado = -1; break;");
    linha(saida, 2, "}");
    linha(saida, 2, "if (estado < 0) break;");
    linha(saida, 2, "++pos;");
    std::string finais;
    for (std::size_t i = 0; i < dfa.finais.size(); ++i)
    {
        if (dfa.finais[i])
        {
            if (!finais.empty()) finais += " || ";
            finais += std::format("estado == {}", i);
        }
    }
    linha(saida, 2, std::format("if ({}) melhor = pos;", finais.empty() ? "0" : finais));
    linha(saida, 1, "}");
    linha(saida, 1, "return melhor;");
    linha(saida, 0, "}");
    saida += '\n';
}

class Emissor
{
public:
    Emissor(const ebnf::Gramatica& gramatica, const analysis::Resultado& analise,
            std::string_view cabecalho, bool arvore)
        : _gramatica(gramatica), _analise(analise), _nome_cabecalho(cabecalho),
          _arvore(arvore) {}

    [[nodiscard]] std::expected<ArquivosGerados, std::string> executar()
    {
        if (_gramatica.tokens_externos)
        {
            auto validacao = validar_tokens_microc();
            if (!validacao) return std::unexpected(std::move(validacao.error()));
        }
        auto lexico = preparar_lexico();
        if (!lexico) return std::unexpected(std::move(lexico.error()));
        gerar_cabecalho();
        gerar_fonte();
        return ArquivosGerados{std::move(_cabecalho), std::move(_fonte)};
    }

private:
    const ebnf::Gramatica& _gramatica;
    const analysis::Resultado& _analise;
    std::string _nome_cabecalho;
    bool _arvore;
    std::vector<Automato> _automatos;
    std::vector<std::vector<int>> _conjuntos;
    std::string _cabecalho;
    std::string _fonte;

    // A numeracao de tokens faz parte do contrato do scanner legado do TP1.
    // Nao alteramos o submodulo: a tabela fica isolada neste backend.
    [[nodiscard]] static std::optional<std::string_view> literal_microc(std::string_view literal)
    {
        static constexpr std::pair<std::string_view, std::string_view> mapa[] = {
            {"+", "PLUS"}, {"-", "MINUS"}, {"*", "MUL"}, {"/", "DIV"},
            {"%", "MOD"}, {"==", "EQ"}, {"!=", "NEQ"}, {"<", "LT"},
            {">", "GT"}, {"<=", "LEQ"}, {">=", "GEQ"},
            {"&&", "AND"}, {"||", "OR"}, {"!", "NOT"},
            {"=", "ASSIGN"}, {";", "SEMICOLON"}, {",", "COMMA"},
            {"(", "LPAREN"}, {")", "RPAREN"}, {"{", "LBRACE"},
            {"}", "RBRACE"}, {"[", "LBRACKET"}, {"]", "RBRACKET"},
            {"main", "MAIN"}, {"if", "IF"}, {"else", "ELSE"},
            {"for", "FOR"}, {"return", "RETURN"}, {"int", "INT"},
            {"char", "CHAR"}, {"print", "PRINT"}
        };
        for (const auto& [texto, nome] : mapa)
            if (texto == literal) return nome;
        return std::nullopt;
    }

    [[nodiscard]] std::expected<void, std::string> validar_tokens_microc() const
    {
        static constexpr std::string_view nomeados[] = {
            "UNDEF", "ID", "INTEGERCONST", "CHARCONST", "STRINGCONST",
            "PLUS", "MINUS", "MUL", "DIV", "MOD", "EQ", "NEQ",
            "LT", "GT", "LEQ", "GEQ", "AND", "OR", "NOT",
            "ASSIGN", "SEMICOLON", "COMMA", "LPAREN", "RPAREN",
            "LBRACE", "RBRACE", "LBRACKET", "RBRACKET",
            "MAIN", "IF", "ELSE", "FOR", "RETURN", "INT", "CHAR", "PRINT"
        };
        for (const auto& token : _analise.terminais)
        {
            if (token.tipo == analysis::TipoTerminal::Fim) continue;
            if (token.tipo == analysis::TipoTerminal::Literal)
            {
                if (!literal_microc(token.nome))
                    return std::unexpected(std::format(
                        "literal '{}' nao pertence ao vocabulario Micro C", token.nome));
            }
            else if (std::ranges::find(nomeados, token.nome) == std::end(nomeados))
            {
                return std::unexpected(std::format(
                    "terminal '{}' nao existe no scanner Micro C", token.nome));
            }
        }
        return {};
    }

    [[nodiscard]] std::expected<void, std::string> preparar_lexico()
    {
        if (_gramatica.tokens_externos) return {};
        for (std::size_t i = 0; i < _gramatica.skips.size(); ++i)
        {
            auto dfa = compilar_regex(_gramatica.skips[i].regex);
            if (!dfa) return std::unexpected(std::format("SKIP {}: {}", i + 1, dfa.error()));
            _automatos.push_back(std::move(*dfa));
        }
        for (const auto& token : _gramatica.tokens)
        {
            auto dfa = compilar_regex(token.regex);
            if (!dfa) return std::unexpected(std::format("TOKENS {}: {}", token.nome, dfa.error()));
            _automatos.push_back(std::move(*dfa));
        }
        return {};
    }

    [[nodiscard]] std::string nome_tipo(std::size_t indice) const
    {
        const auto& token = _analise.terminais[indice];
        if (_gramatica.tokens_externos)
        {
            if (token.tipo == analysis::TipoTerminal::Fim) return "END_OF_FILE";
            if (token.tipo == analysis::TipoTerminal::Literal)
                return std::string{*literal_microc(token.nome)};
            return token.nome;
        }
        if (token.tipo == analysis::TipoTerminal::Fim) return "T_FIM";
        if (token.tipo == analysis::TipoTerminal::Literal)
            return std::format("T_LIT_{}", indice);
        return "T_N_" + token.nome;
    }

    [[nodiscard]] std::optional<std::size_t> indice_terminal(
        std::string_view nome, bool literal) const
    {
        for (std::size_t i = 0; i < _analise.terminais.size(); ++i)
        {
            const auto& t = _analise.terminais[i];
            if (t.nome == nome && (t.tipo == analysis::TipoTerminal::Literal) == literal &&
                t.tipo != analysis::TipoTerminal::Fim) return i;
        }
        return std::nullopt;
    }

    [[nodiscard]] int registrar_conjunto(const analysis::Conjunto& conjunto)
    {
        std::vector<int> indices;
        for (std::size_t i = 0; i < _analise.terminais.size(); ++i)
            if (conjunto.contem(i)) indices.push_back(static_cast<int>(i));
        const auto it = std::ranges::find(_conjuntos, indices);
        if (it != _conjuntos.end()) return static_cast<int>(it - _conjuntos.begin());
        _conjuntos.push_back(std::move(indices));
        return static_cast<int>(_conjuntos.size() - 1);
    }

    void gerar_cabecalho()
    {
        _cabecalho = "/* Parser gerado automaticamente em ANSI C89. */\n"
            "#ifndef COMP2_PARSER_GERADO_H\n#define COMP2_PARSER_GERADO_H\n\n";
        if (_gramatica.tokens_externos)
        {
            _cabecalho += "#include \"scanner.h\" /* adaptador do scanner Micro C */\n"
                "#define TOKEN_FIM END_OF_FILE\n\n";
        }
        else
        {
            _cabecalho += "typedef enum {\n";
            for (std::size_t i = 0; i < _analise.terminais.size(); ++i)
            {
                linha(_cabecalho, 1, std::format("{} = {}{}", nome_tipo(i), i,
                    i + 1 == _analise.terminais.size() ? "" : ","));
            }
            _cabecalho += "} TipoToken;\n\ntypedef struct {\n"
                "    int tipo;\n    char lexema[100];\n    int linha;\n    int coluna;\n"
                "} Token;\n\n#define TOKEN_FIM T_FIM\n\n";
        }
        _cabecalho += "const char *nome_token(int tipo);\n";
        for (const auto& regra : _gramatica.regras)
            _cabecalho += std::format("void parse_{}(void);\n", regra.nome);
        _cabecalho += "\n#endif\n";
    }

    void gerar_procedimentos(std::string& corpo)
    {
        for (const auto& regra : _gramatica.regras)
        {
            linha(corpo, 0, std::format("void parse_{}(void)", regra.nome));
            linha(corpo, 0, "{");
            linha(corpo, 1, std::format("arvore_entra({});", string_c(regra.nome)));
            emitir_expressao(corpo, *regra.expressao, 1);
            linha(corpo, 1, "arvore_sai();");
            linha(corpo, 0, "}");
            corpo += '\n';
        }
    }

    void emitir_expressao(std::string& corpo, const ebnf::Expressao& expressao, int nivel)
    {
        const auto& info = _analise.expressoes.at(&expressao);
        std::visit([&](const auto& no)
        {
            using Tipo = std::decay_t<decltype(no)>;
            if constexpr (std::is_same_v<Tipo, ebnf::ReferenciaTerminal>)
            {
                const auto indice = indice_terminal(no.nome, no.literal);
                if (indice) linha(corpo, nivel, std::format("match({});", nome_tipo(*indice)));
            }
            else if constexpr (std::is_same_v<Tipo, ebnf::ReferenciaNaoTerminal>)
            {
                linha(corpo, nivel, std::format("parse_{}();", no.nome));
            }
            else if constexpr (std::is_same_v<Tipo, ebnf::Sequencia>)
            {
                for (const auto& filho : no.elementos) emitir_expressao(corpo, *filho, nivel);
            }
            else if constexpr (std::is_same_v<Tipo, ebnf::Opcional> ||
                               std::is_same_v<Tipo, ebnf::Repeticao>)
            {
                const auto& filho_info = _analise.expressoes.at(no.conteudo.get());
                const int idx = registrar_conjunto(filho_info.first);
                linha(corpo, nivel, std::format("{} (pertence(tok.tipo, conj_{})) {{",
                    std::is_same_v<Tipo, ebnf::Opcional> ? "if" : "while", idx));
                emitir_expressao(corpo, *no.conteudo, nivel + 1);
                linha(corpo, nivel, "}");
            }
            else if constexpr (std::is_same_v<Tipo, ebnf::Alternativa>)
            {
                bool primeiro = true;
                const ebnf::Expressao* anulavel = nullptr;
                for (const auto& opcao : no.opcoes)
                {
                    const auto& op_info = _analise.expressoes.at(opcao.get());
                    if (op_info.anulavel)
                        anulavel = opcao.get();
                    if (op_info.first.vazio()) continue;
                    const int idx = registrar_conjunto(op_info.first);
                    linha(corpo, nivel, std::format("{}if (pertence(tok.tipo, conj_{})) {{",
                        primeiro ? "" : "else ", idx));
                    emitir_expressao(corpo, *opcao, nivel + 1);
                    linha(corpo, nivel, "}");
                    primeiro = false;
                }
                if (anulavel)
                {
                    if (!primeiro) linha(corpo, nivel, "else {");
                    emitir_expressao(corpo, *anulavel, nivel + (primeiro ? 0 : 1));
                    if (!primeiro) linha(corpo, nivel, "}");
                }
                else
                {
                    const int idx = registrar_conjunto(info.first);
                    if (!primeiro) linha(corpo, nivel, "else {");
                    linha(corpo, nivel + (primeiro ? 0 : 1),
                        std::format("erro_sintatico(conj_{});", idx));
                    if (!primeiro) linha(corpo, nivel, "}");
                }
            }
        }, expressao.no);
    }

    void gerar_fonte()
    {
        std::string corpo;
        gerar_procedimentos(corpo); // Antes do cabecalho para registrar os conjuntos.
        _fonte = "/* Analisador descendente recursivo gerado em ANSI C89. */\n"
            "#include <stdio.h>\n#include <stdlib.h>\n#include <string.h>\n"
            "#include <ctype.h>\n#include <errno.h>\n#include <limits.h>\n"
            "#include " + string_c(_nome_cabecalho) + "\n\n";
        _fonte += std::format("#define SUPORTE_ARVORE {}\n", _arvore ? 1 : 0);
        _fonte += std::format("#define COMP2_TOKEN_POSSUI_COLUNA {}\n\n",
            _gramatica.tokens_externos ? 0 : 1);
        for (std::size_t i = 0; i < _conjuntos.size(); ++i)
        {
            _fonte += std::format("static const int conj_{}[] = {{", i);
            for (int indice : _conjuntos[i])
                _fonte += std::format(" {},", nome_tipo(static_cast<std::size_t>(indice)));
            _fonte += " -1 };\n";
        }
        _fonte += '\n';
        _fonte += R"C(
static Token tok;
static int imprimir_arvore = 0;
static int nivel_arvore = 0;
static FILE *arquivo_tokens = NULL;
)C";
        if (!_gramatica.tokens_externos)
        {
            _fonte += R"C(static char *fonte = NULL;
static size_t pos = 0;
static int linha = 1, coluna = 1;

static char *ler_tudo(FILE *f)
{
    size_t cap = 4096, n = 0, lidos;
    char *buf = (char *)malloc(cap), *novo;
    if (buf == NULL) { fprintf(stderr, "memoria insuficiente\n"); exit(EXIT_FAILURE); }
    while ((lidos = fread(buf + n, 1, cap - n - 1, f)) > 0) {
        n += lidos;
        if (cap - n - 1 == 0) {
            cap *= 2;
            novo = (char *)realloc(buf, cap);
            if (novo == NULL) { free(buf); fprintf(stderr, "memoria insuficiente\n"); exit(EXIT_FAILURE); }
            buf = novo;
        }
    }
    if (ferror(f)) { free(buf); fprintf(stderr, "erro ao ler a entrada\n"); exit(EXIT_FAILURE); }
    buf[n] = '\0';
    return buf;
}

static void consumir_entrada(size_t tamanho)
{
    while (tamanho-- > 0) {
        if (fonte[pos++] == '\n') { ++linha; coluna = 1; }
        else ++coluna;
    }
}

)C";
            for (std::size_t i = 0; i < _automatos.size(); ++i)
                emitir_automato(_fonte, _automatos[i], i);
            gerar_lexico();
        }
        gerar_primitivas();
        _fonte += "\n" + corpo;
        gerar_main();
    }

    void gerar_lexico()
    {
        _fonte += "static const char *const texto_literais[] = {\n";
        for (const auto& terminal : _analise.terminais)
            if (terminal.tipo == analysis::TipoTerminal::Literal)
                linha(_fonte, 1, string_c(terminal.nome) + ",");
        linha(_fonte, 1, "NULL");
        _fonte += "};\nstatic const int tipo_literais[] = {\n";
        for (std::size_t i = 0; i < _analise.terminais.size(); ++i)
            if (_analise.terminais[i].tipo == analysis::TipoTerminal::Literal)
                linha(_fonte, 1, nome_tipo(i) + ",");
        linha(_fonte, 1, "-1");
        _fonte += "};\n\n";
        _fonte += R"C(static Token proximo_token_gerado(void)
{
    Token t;
    size_t n, melhor, skip;
    int tipo, i;
    do {
        skip = 0;
)C";
        for (std::size_t i = 0; i < _gramatica.skips.size(); ++i)
            linha(_fonte, 2, std::format("n = casa_regex_{}(fonte + pos); if (n > skip) skip = n;", i));
        _fonte += R"C(        consumir_entrada(skip);
    } while (skip > 0);
    memset(&t, 0, sizeof t);
    t.linha = linha;
    t.coluna = coluna;
    if (fonte[pos] == '\0') { t.tipo = T_FIM; return t; }
    melhor = 0;
    tipo = -1;
    for (i = 0; texto_literais[i] != NULL; ++i) {
        n = strlen(texto_literais[i]);
        if (n > melhor && strncmp(fonte + pos, texto_literais[i], n) == 0) {
            melhor = n;
            tipo = tipo_literais[i];
        }
    }
)C";
        for (std::size_t i = 0; i < _gramatica.tokens.size(); ++i)
        {
            const auto idx = indice_terminal(_gramatica.tokens[i].nome, false);
            if (idx)
                linha(_fonte, 1, std::format("n = casa_regex_{}(fonte + pos); if (n > melhor) {{ melhor = n; tipo = {}; }}",
                    _gramatica.skips.size() + i, nome_tipo(*idx)));
        }
        _fonte += R"C(    if (melhor == 0) {
        fprintf(stderr, "linha %d, coluna %d: erro lexico: caractere invalido '%c'\n",
                linha, coluna, fonte[pos]);
        exit(EXIT_FAILURE);
    }
    n = melhor < sizeof t.lexema ? melhor : sizeof t.lexema - 1;
    memcpy(t.lexema, fonte + pos, n);
    t.lexema[n] = '\0';
    t.tipo = tipo;
    consumir_entrada(melhor);
    return t;
}

)C";
    }

    void gerar_leitor_tokens()
    {
        _fonte += "static int tipo_token_por_nome(const char *nome)\n{\n";
        if (_gramatica.tokens_externos)
        {
            // O fluxo de tokens original do TP1 imprime nomes do enum,
            // inclusive alguns que podem nao aparecer na gramatica.
            static constexpr std::string_view nomes[] = {
                "UNDEF", "ID", "END_OF_FILE", "INTEGERCONST", "CHARCONST",
                "STRINGCONST", "PLUS", "MINUS", "MUL", "DIV", "MOD",
                "EQ", "NEQ", "LT", "GT", "LEQ", "GEQ", "AND", "OR", "NOT",
                "ASSIGN", "SEMICOLON", "COMMA", "LPAREN", "RPAREN",
                "LBRACE", "RBRACE", "LBRACKET", "RBRACKET",
                "MAIN", "IF", "ELSE", "FOR", "RETURN", "INT", "CHAR", "PRINT"
            };
            for (const auto nome : nomes)
                linha(_fonte, 1, std::format("if (strcmp(nome, {}) == 0) return {};",
                    string_c(nome), nome));
        }
        else
        {
            for (std::size_t i = 0; i < _analise.terminais.size(); ++i)
            {
                const auto& terminal = _analise.terminais[i];
                const auto nome = nome_tipo(i);
                linha(_fonte, 1, std::format("if (strcmp(nome, {}) == 0) return {};",
                    string_c(nome), nome));
                if (terminal.tipo == analysis::TipoTerminal::Nomeado)
                    linha(_fonte, 1, std::format("if (strcmp(nome, {}) == 0) return {};",
                        string_c(terminal.nome), nome));
            }
        }
        linha(_fonte, 1, "return -1;");
        _fonte += "}\n\n";
        _fonte += leitor_tokens_ansi_c;
    }

    void gerar_primitivas()
    {
        _fonte += "const char *nome_token(int tipo)\n{\n    switch (tipo) {\n";
        for (std::size_t i = 0; i < _analise.terminais.size(); ++i)
        {
            const auto& t = _analise.terminais[i];
            const auto nome = t.tipo == analysis::TipoTerminal::Fim ? "fim de arquivo" :
                t.tipo == analysis::TipoTerminal::Literal ? "'" + t.nome + "'" : t.nome;
            linha(_fonte, 1, std::format("case {}: return {};", nome_tipo(i), string_c(nome)));
        }
        _fonte += R"C(    default: return "token desconhecido";
    }
}

int pertence(int tipo, const int *conjunto)
{
    while (*conjunto != -1) {
        if (tipo == *conjunto) return 1;
        ++conjunto;
    }
    return 0;
}

static void escrever_posicao(void)
{
)C";
        _fonte += _gramatica.tokens_externos
            ? "    fprintf(stderr, \"linha %d: \", tok.linha);\n"
            : "    fprintf(stderr, \"linha %d, coluna %d: \", tok.linha, tok.coluna);\n";
        _fonte += R"C(}

static void erro_sintatico(const int *esperados)
{
    int i;
    escrever_posicao();
    if (tok.tipo == TOKEN_FIM) fprintf(stderr, "encontrado fim de arquivo, ");
    else fprintf(stderr, "encontrado '%s', ", tok.lexema);
    if (esperados[0] != -1 && esperados[1] == -1) {
        fprintf(stderr, "esperado %s\n", nome_token(esperados[0]));
    } else {
        fprintf(stderr, "esperado um de {");
        for (i = 0; esperados[i] != -1; ++i)
            fprintf(stderr, "%s%s", i ? ", " : "", nome_token(esperados[i]));
        fprintf(stderr, "}\n");
    }
    exit(EXIT_FAILURE);
}

static void arvore_entra(const char *nome)
{
    if (SUPORTE_ARVORE && imprimir_arvore) {
        int i;
        for (i = 0; i < nivel_arvore; ++i) fputs("  ", stdout);
        printf("%s\n", nome);
    }
    ++nivel_arvore;
}

static void arvore_sai(void)
{
    --nivel_arvore;
}

)C";
        gerar_leitor_tokens();
        _fonte += R"C(static void avancar(void)
{
    if (arquivo_tokens != NULL) tok = token_de_arquivo();
    else {
)C";
        if (_gramatica.tokens_externos)
        {
            _fonte += "        tok = proximo_token();\n";
        }
        else _fonte += "        tok = proximo_token_gerado();\n";
        _fonte += R"C(    }
)C";
        if (_gramatica.tokens_externos)
        {
            // O UNDEF deve ser tratado inclusive quando vem de --tokens.
            _fonte += R"C(    if (tok.tipo == UNDEF) {
        fprintf(stderr, "linha %d: erro lexico: %s\n", tok.linha, tok.lexema);
        exit(EXIT_FAILURE);
    }
)C";
        }
        _fonte += R"C(}

void match(int esperado)
{
    int lista[2];
    if ((int)tok.tipo != esperado) {
        lista[0] = esperado;
        lista[1] = -1;
        erro_sintatico(lista);
    }
    if (SUPORTE_ARVORE && imprimir_arvore) {
        int i;
        for (i = 0; i < nivel_arvore; ++i) fputs("  ", stdout);
        if (nome_token(tok.tipo)[0] == '\'') printf("%s\n", nome_token(tok.tipo));
        else printf("%s '%s'\n", nome_token(tok.tipo), tok.lexema);
    }
    avancar();
}

)C";
    }

    void gerar_main()
    {
        _fonte += R"C(int main(int argc, char **argv)
{
    const char *arquivo = NULL, *tokens = NULL;
    FILE *entrada = stdin;
    int i, fim[2];
    for (i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--tree") == 0) imprimir_arvore = 1;
        else if (strcmp(argv[i], "--tokens") == 0 && i + 1 < argc) tokens = argv[++i];
        else if (argv[i][0] == '-' && argv[i][1] != '\0') {
            fprintf(stderr, "uso: %s [--tree] [--tokens arquivo.tok | programa]\n", argv[0]);
            return 2;
        } else arquivo = argv[i];
    }
    if (arquivo != NULL && tokens != NULL) {
        fprintf(stderr, "use um arquivo de programa ou --tokens, nao ambos\n");
        return 2;
    }
    if (imprimir_arvore && !SUPORTE_ARVORE)
        fprintf(stderr, "aviso: gere o analisador com --tree para imprimir a arvore\n");
    if (tokens != NULL) {
        arquivo_tokens = fopen(tokens, "r");
        if (arquivo_tokens == NULL) { fprintf(stderr, "nao foi possivel abrir %s\n", tokens); return 2; }
    } else {
        if (arquivo != NULL && (entrada = fopen(arquivo, "rb")) == NULL) {
            fprintf(stderr, "nao foi possivel abrir %s\n", arquivo);
            return 2;
        }
        )C";
        if (_gramatica.tokens_externos)
            _fonte += "scanner_iniciar(entrada);\n";
        else
            _fonte += "fonte = ler_tudo(entrada);\n";
        _fonte += R"C(    }
    avancar();
)C";
        _fonte += std::format("    parse_{}();\n", _gramatica.regras.front().nome);
        _fonte += R"C(    if (tok.tipo != TOKEN_FIM) {
        fim[0] = TOKEN_FIM;
        fim[1] = -1;
        erro_sintatico(fim);
    }
    printf("entrada aceita\n");
    if (arquivo_tokens != NULL) fclose(arquivo_tokens);
    else {
)C";
        if (_gramatica.tokens_externos)
            _fonte += "        scanner_finalizar();\n";
        else
            _fonte += "        free(fonte);\n";
        _fonte += R"C(        if (entrada != stdin) fclose(entrada);
    }
    return 0;
}
)C";
    }
};
}

std::expected<ArquivosGerados, std::string>
gerar(const ebnf::Gramatica& gramatica,
     const analysis::Resultado& analise,
     std::string_view nome_cabecalho,
     bool suporte_arvore)
{
    if (gramatica.regras.empty()) return std::unexpected("gramatica sem regras");
    return Emissor{gramatica, analise, nome_cabecalho, suporte_arvore}.executar();
}
}
