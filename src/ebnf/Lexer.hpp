#pragma once

#include <string>
#include <sstream>
#include <fstream>
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

class Lexer
{
    private:
        Scanner _scanner;
    public:
        Lexer(Scanner scanner) : _scanner(std::move(scanner))
        {

        }

        compiladores::ebnf::Token get_prox_token();
};
}
