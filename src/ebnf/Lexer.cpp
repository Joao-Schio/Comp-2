#include "src/ebnf/Lexer.hpp"
namespace compiladores::ebnf
{
char Scanner::peek() const noexcept
{
    if (this -> eof())
    {
        return '\0';
    }
    else
    {
        return this -> _fonte[_posicao];
    }
}

bool Scanner::eof() const noexcept
{
    return this -> _posicao == this -> _fonte.size();
}

char Scanner::advance() noexcept
{
    while(!this -> eof())
    {
        auto caracter = this -> peek();
        this->_posicao++;
        if (caracter == '\n')
        {
            this -> _linha++;
            this -> _coluna = 1;
        }
        else
        {
            this -> _coluna++;    
        }
        if (!std::isspace(caracter))
        {
            return caracter;
        }
    }
    return '\0';
}

common::Posicao Scanner::posicao() const noexcept
{
    return 
    {
        .offset = _posicao,
        .linha = _linha,
        .coluna = _coluna
    };
}


Token Lexer::ler_simbolo(Simbolo simbolo, common::Posicao posicao)
{
    _scanner.advance();
    return Token{
        .tipo = simbolo,
        .localizacao =
            {
                ._inicio = posicao,
                ._fim = _scanner.posicao()}};
}

std::expected<Token, ErroLexico>
Lexer::get_prox_token()
{
    const auto inicio = _scanner.posicao();

    if (_scanner.eof())
    {
        return Token{
            .tipo = Eof{},
            .localizacao = {
                ._inicio = inicio,
                ._fim = inicio}};
    }

    const char caracter = _scanner.peek();
    
    auto is_simbolo = this -> ler_simbolo(caracter, inicio);
    if (is_simbolo.has_value())
    {
        return std::move(*is_simbolo);
    }

    
    return std::unexpected(
        ErroLexico::caracter_invalido(caracter, inicio));
}
}