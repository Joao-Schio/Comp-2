#pragma once

#include <string>
#include <expected>
#include <optional>
#include <format>
#include "src/ebnf/Token.hpp"

namespace compiladores::ebnf
{

class Scanner
{
    private:
        std::string _fonte;
        std::size_t _posicao{0};
        std::size_t _linha{1};
        std::size_t _coluna{1};

    public:
        explicit Scanner(std::string fonte)
            : _fonte(std::move(fonte))
        {
        }

        [[nodiscard]] char peek() const noexcept;
        char advance() noexcept;

        [[nodiscard]] bool eof() const noexcept;
        [[nodiscard]] common::Posicao posicao() const noexcept;
};


struct ErroLexico
{
    std::string mensagem;
    common::Posicao posicao;
    static ErroLexico caracter_invalido(
        char caracter,
        common::Posicao posicao)
    {
        return {
            .mensagem = std::format(
                "caractere invalido '{}'", caracter),
            .posicao = posicao};
    }
};

class Lexer
{
    private:
        Scanner _scanner;
    private:
        [[nodiscard]] Token criar_token(
            TokenType tipo,
            common::Posicao inicio) const;

        [[nodiscard]] std::optional<ErroLexico>
            pular_ignoraveis();

        [[nodiscard]] std::expected<Token, ErroLexico>
            ler_identificador(common::Posicao inicio);

        [[nodiscard]] std::expected<Token, ErroLexico>
            ler_literal(common::Posicao inicio);

        [[nodiscard]] std::expected<Token, ErroLexico>
            ler_regex(common::Posicao inicio);
    private:    
        [[nodiscard]] Token
            ler_simbolo(Simbolo simbolo, common::Posicao inicio);

    public:
        Lexer(Scanner scanner) : _scanner(std::move(scanner))
        {

        }

        [[nodiscard]]
            std::expected<Token, ErroLexico> get_prox_token();
};
}
