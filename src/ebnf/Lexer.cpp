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
}