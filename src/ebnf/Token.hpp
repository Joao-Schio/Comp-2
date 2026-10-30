#pragma once

#include <string>
#include <string_view>
#include <variant>
#include <memory>
#include "src/common/Localizacao.hpp"

namespace compiladores::ebnf
{

struct Eof
{
    std::string_view lexama{"EOF"};
};

struct Terminal
{
    std::string lexema;    
};

struct NaoTerminal
{
    std::string lexema;
};

struct ExpressaoRegular
{
    std::string lexema;
};

enum class PalavraReservada 
{
    Igual,
    PontoVirgula,
    AbreChave,
    FechaChave,
    AbreColchete,
    FechaColchete,
    AbreParenteses,
    FechaParenteses
};

using TokenType = std::variant<Eof, 
                        Terminal,
                        NaoTerminal,
                        ExpressaoRegular, 
                        PalavraReservada>;

using compiladores::common::Localizacao;

class Token
{
    private:
        TokenType _tipo;
        Localizacao _localizacao;
            
};
}