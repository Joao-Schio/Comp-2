
#pragma once

#include <string>
#include <variant>

#include "common/Localizacao.hpp"

namespace compiladores::ebnf
{
struct Eof
{
};
struct Terminal
{
    std::string lexema;
};

struct NaoTerminal
{
    std::string lexema;
};

struct Literal
{
    std::string valor;
};


struct ExpressaoRegular
{
    std::string padrao;
};


enum class Simbolo
{
    Igual,
    PontoVirgula,
    Alternativa,
    AbreChave,
    FechaChave,
    AbreColchete,
    FechaColchete,
    AbreParenteses,
    FechaParenteses
};


enum class PalavraReservada
{
    Tokens,
    Rules,
    Skip,
    Extern
};


using TokenType = std::variant<
    Eof,
    Terminal,
    NaoTerminal,
    Literal,
    ExpressaoRegular,
    Simbolo,
    PalavraReservada>;


struct Token
{
    TokenType tipo;
    common::Localizacao localizacao;
};
}
