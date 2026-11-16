#pragma once

#include "src/ebnf/Gramatica.hpp"
#include "src/ebnf/Lexer.hpp"

#include <expected>
#include <string>
#include <string_view>
#include <utility>

namespace compiladores::ebnf
{
struct ErroSintatico
{
    std::string mensagem;
    common::Posicao posicao;
};

class Parser
{
public:
    explicit Parser(Lexer lexer) : _lexer(std::move(lexer)) {}

    [[nodiscard]] std::expected<Gramatica, ErroSintatico> analisar();

private:
    Lexer _lexer;
    Token _atual{.tipo = Eof{}, .localizacao = {}};
    common::Posicao _ultimo_fim{};

    [[nodiscard]] std::expected<void, ErroSintatico> avancar();
    [[nodiscard]] std::expected<void, ErroSintatico> consumir(Simbolo simbolo);
    [[nodiscard]] std::expected<void, ErroSintatico> consumir(PalavraReservada palavra);

    [[nodiscard]] bool atual_e(Simbolo simbolo) const noexcept;
    [[nodiscard]] bool atual_e(PalavraReservada palavra) const noexcept;
    [[nodiscard]] bool inicio_fator() const noexcept;
    [[nodiscard]] ErroSintatico esperado(std::string_view descricao) const;

    [[nodiscard]] std::expected<void, ErroSintatico> ler_secao_tokens(Gramatica& gramatica);
    [[nodiscard]] std::expected<void, ErroSintatico> ler_definicao_token(Gramatica& gramatica);
    [[nodiscard]] std::expected<void, ErroSintatico> ler_regra(Gramatica& gramatica);
    [[nodiscard]] std::expected<ExpressaoPtr, ErroSintatico> ler_expressao();
    [[nodiscard]] std::expected<ExpressaoPtr, ErroSintatico> ler_sequencia();
    [[nodiscard]] std::expected<ExpressaoPtr, ErroSintatico> ler_fator();
};

[[nodiscard]] std::expected<Gramatica, ErroSintatico>
analisar_ebnf(std::string fonte);
}
