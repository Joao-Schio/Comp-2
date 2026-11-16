#pragma once

#include "common/Localizacao.hpp"

#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace compiladores::ebnf
{
struct Expressao;
using ExpressaoPtr = std::unique_ptr<Expressao>;

struct Vazio {};

struct ReferenciaTerminal
{
    std::string nome;
    bool literal;
};

struct ReferenciaNaoTerminal
{
    std::string nome;
};

struct Sequencia
{
    std::vector<ExpressaoPtr> elementos;
};

struct Alternativa
{
    std::vector<ExpressaoPtr> opcoes;
};

struct Opcional
{
    ExpressaoPtr conteudo;
};

struct Repeticao
{
    ExpressaoPtr conteudo;
};

struct Expressao
{
    using No = std::variant<Vazio, ReferenciaTerminal, ReferenciaNaoTerminal,
                            Sequencia, Alternativa, Opcional, Repeticao>;

    No no;
    common::Localizacao localizacao;
};

struct DefinicaoToken
{
    std::string nome;
    std::string regex;
    common::Localizacao localizacao;
};

struct DefinicaoSkip
{
    std::string regex;
    common::Localizacao localizacao;
};

struct Regra
{
    std::string nome;
    ExpressaoPtr expressao;
    common::Localizacao localizacao;
};

struct Gramatica
{
    std::optional<std::string> tokens_externos;
    std::vector<DefinicaoToken> tokens;
    std::vector<DefinicaoSkip> skips;
    std::vector<Regra> regras;
};
}
