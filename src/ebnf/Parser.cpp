#include "src/ebnf/Parser.hpp"

#include <memory>
#include <utility>

namespace compiladores::ebnf
{
namespace
{
[[nodiscard]] constexpr std::string_view nome_simbolo(Simbolo simbolo) noexcept
{
    switch (simbolo)
    {
    case Simbolo::Igual: return "'='";
    case Simbolo::PontoVirgula: return "';'";
    case Simbolo::Alternativa: return "'|'";
    case Simbolo::AbreChave: return "'{'";
    case Simbolo::FechaChave: return "'}'";
    case Simbolo::AbreColchete: return "'['";
    case Simbolo::FechaColchete: return "']'";
    case Simbolo::AbreParenteses: return "'('";
    case Simbolo::FechaParenteses: return "')'";
    }
    return "simbolo da EBNF";
}

[[nodiscard]] ExpressaoPtr criar_expressao(
    Expressao::No no, common::Posicao inicio, common::Posicao fim)
{
    return std::make_unique<Expressao>(
        Expressao{.no = std::move(no), .localizacao = {._inicio = inicio, ._fim = fim}});
}
} // namespace

std::expected<void, ErroSintatico> Parser::avancar()
{
    _ultimo_fim = _atual.localizacao._fim;
    auto proximo = _lexer.get_prox_token();
    if (!proximo)
    {
        return std::unexpected(ErroSintatico{
            .mensagem = std::move(proximo.error().mensagem),
            .posicao = proximo.error().posicao});
    }

    _atual = std::move(*proximo);
    return {};
}

bool Parser::atual_e(Simbolo simbolo) const noexcept
{
    const auto* encontrado = std::get_if<Simbolo>(&_atual.tipo);
    return encontrado != nullptr && *encontrado == simbolo;
}

bool Parser::atual_e(PalavraReservada palavra) const noexcept
{
    const auto* encontrada = std::get_if<PalavraReservada>(&_atual.tipo);
    return encontrada != nullptr && *encontrada == palavra;
}

bool Parser::inicio_fator() const noexcept
{
    if (std::holds_alternative<Terminal>(_atual.tipo) ||
        std::holds_alternative<NaoTerminal>(_atual.tipo) ||
        std::holds_alternative<Literal>(_atual.tipo))
    {
        return true;
    }

    return atual_e(Simbolo::AbreParenteses) ||
           atual_e(Simbolo::AbreColchete) ||
           atual_e(Simbolo::AbreChave);
}

ErroSintatico Parser::esperado(std::string_view descricao) const
{
    return ErroSintatico{
        .mensagem = "esperado " + std::string(descricao) + ", encontrado outro token",
        .posicao = _atual.localizacao._inicio};
}

std::expected<void, ErroSintatico> Parser::consumir(Simbolo simbolo)
{
    if (!atual_e(simbolo))
    {
        return std::unexpected(esperado(nome_simbolo(simbolo)));
    }
    return avancar();
}

std::expected<void, ErroSintatico> Parser::consumir(PalavraReservada palavra)
{
    if (!atual_e(palavra))
    {
        return std::unexpected(esperado(palavra == PalavraReservada::Tokens ? "TOKENS" :
                                        palavra == PalavraReservada::Rules ? "RULES" :
                                        palavra == PalavraReservada::Skip ? "SKIP" : "EXTERN"));
    }
    return avancar();
}

std::expected<void, ErroSintatico> Parser::ler_definicao_token(Gramatica& gramatica)
{
    const auto inicio = _atual.localizacao._inicio;
    const auto* terminal = std::get_if<Terminal>(&_atual.tipo);
    const bool skip = atual_e(PalavraReservada::Skip);
    if (terminal == nullptr && !skip)
    {
        return std::unexpected(esperado("nome de terminal ou SKIP"));
    }

    const std::string nome = skip ? "" : terminal -> lexema;
    if (auto resultado = avancar(); !resultado)
    {
        return std::unexpected(std::move(resultado.error()));
    }
    if (auto resultado = consumir(Simbolo::Igual); !resultado)
    {
        return std::unexpected(std::move(resultado.error()));
    }

    const auto* regex = std::get_if<ExpressaoRegular>(&_atual.tipo);
    if (regex == nullptr)
    {
        return std::unexpected(esperado("expressao regular /.../"));
    }
    std::string padrao = regex -> padrao;
    if (auto resultado = avancar(); !resultado)
    {
        return std::unexpected(std::move(resultado.error()));
    }
    if (!atual_e(Simbolo::PontoVirgula))
    {
        return std::unexpected(esperado("';'"));
    }
    const auto fim = _atual.localizacao._fim;
    if (auto resultado = avancar(); !resultado)
    {
        return std::unexpected(std::move(resultado.error()));
    }

    const common::Localizacao localizacao{._inicio = inicio, ._fim = fim};
    if (skip)
    {
        gramatica.skips.push_back({.regex = std::move(padrao), .localizacao = localizacao});
    }
    else
    {
        gramatica.tokens.push_back({
            .nome = nome, .regex = std::move(padrao), .localizacao = localizacao});
    }
    return {};
}

std::expected<void, ErroSintatico> Parser::ler_secao_tokens(Gramatica& gramatica)
{
    if (auto resultado = consumir(PalavraReservada::Tokens); !resultado)
    {
        return std::unexpected(std::move(resultado.error()));
    }

    if (atual_e(PalavraReservada::Extern))
    {
        if (auto resultado = avancar(); !resultado)
        {
            return std::unexpected(std::move(resultado.error()));
        }
        const auto* caminho = std::get_if<Literal>(&_atual.tipo);
        if (!caminho)
        {
            return std::unexpected(esperado("caminho de TOKENS EXTERN entre aspas"));
        }
        gramatica.tokens_externos = caminho -> valor;
        if (auto resultado = avancar(); !resultado)
        {
            return std::unexpected(std::move(resultado.error()));
        }
        if (!atual_e(Simbolo::PontoVirgula))
        {
            return std::unexpected(esperado("';'"));
        }
        return avancar();
    }

    while (std::holds_alternative<Terminal>(_atual.tipo) ||
           atual_e(PalavraReservada::Skip))
    {
        if (auto resultado = ler_definicao_token(gramatica); !resultado)
        {
            return std::unexpected(std::move(resultado.error()));
        }
    }
    return {};
}

std::expected<ExpressaoPtr, ErroSintatico> Parser::ler_fator()
{
    const auto inicio = _atual.localizacao._inicio;
    const auto localizacao = _atual.localizacao;
    if (const auto* terminal = std::get_if<Terminal>(&_atual.tipo))
    {
        std::string nome = terminal -> lexema;
        if (auto resultado = avancar(); !resultado)
        {
            return std::unexpected(std::move(resultado.error()));
        }
        return criar_expressao(ReferenciaTerminal{.nome = std::move(nome), .literal = false},
                               inicio, localizacao._fim);
    }
    if (const auto* literal = std::get_if<Literal>(&_atual.tipo))
    {
        std::string valor = literal -> valor;
        if (auto resultado = avancar(); !resultado)
        {
            return std::unexpected(std::move(resultado.error()));
        }
        return criar_expressao(ReferenciaTerminal{.nome = std::move(valor), .literal = true},
                               inicio, localizacao._fim);
    }
    if (const auto* nao_terminal = std::get_if<NaoTerminal>(&_atual.tipo))
    {
        std::string nome = nao_terminal -> lexema;
        if (auto resultado = avancar(); !resultado)
        {
            return std::unexpected(std::move(resultado.error()));
        }
        return criar_expressao(ReferenciaNaoTerminal{.nome = std::move(nome)},
                               inicio, localizacao._fim);
    }

    const bool parenteses = atual_e(Simbolo::AbreParenteses);
    const bool opcional = atual_e(Simbolo::AbreColchete);
    const bool repeticao = atual_e(Simbolo::AbreChave);
    if (!parenteses && !opcional && !repeticao)
    {
        return std::unexpected(esperado("fator da EBNF"));
    }

    if (auto resultado = avancar(); !resultado)
    {
        return std::unexpected(std::move(resultado.error()));
    }
    auto conteudo = ler_expressao();
    if (!conteudo)
    {
        return std::unexpected(std::move(conteudo.error()));
    }

    const Simbolo fechamento = parenteses ? Simbolo::FechaParenteses :
                               opcional ? Simbolo::FechaColchete : Simbolo::FechaChave;
    if (!atual_e(fechamento))
    {
        return std::unexpected(esperado(parenteses ? "')'" : opcional ? "']'" : "'}'"));
    }
    const auto fim = _atual.localizacao._fim;
    if (auto resultado = avancar(); !resultado)
    {
        return std::unexpected(std::move(resultado.error()));
    }

    if (parenteses)
    {
        return std::move(*conteudo);
    }
    if (opcional)
    {
        return criar_expressao(Opcional{.conteudo = std::move(*conteudo)}, inicio, fim);
    }
    return criar_expressao(Repeticao{.conteudo = std::move(*conteudo)}, inicio, fim);
}

std::expected<ExpressaoPtr, ErroSintatico> Parser::ler_sequencia()
{
    const auto inicio = _atual.localizacao._inicio;
    std::vector<ExpressaoPtr> elementos;
    while (inicio_fator())
    {
        auto fator = ler_fator();
        if (!fator)
        {
            return std::unexpected(std::move(fator.error()));
        }
        elementos.push_back(std::move(*fator));
    }

    if (elementos.empty())
    {
        return criar_expressao(Vazio{}, inicio, inicio);
    }
    if (elementos.size() == 1)
    {
        return std::move(elementos.front());
    }
    return criar_expressao(Sequencia{.elementos = std::move(elementos)},
                           inicio, _ultimo_fim);
}

std::expected<ExpressaoPtr, ErroSintatico> Parser::ler_expressao()
{
    const auto inicio = _atual.localizacao._inicio;
    auto primeira = ler_sequencia();
    if (!primeira)
    {
        return std::unexpected(std::move(primeira.error()));
    }
    if (!atual_e(Simbolo::Alternativa))
    {
        return std::move(*primeira);
    }

    std::vector<ExpressaoPtr> opcoes;
    opcoes.push_back(std::move(*primeira));
    while (atual_e(Simbolo::Alternativa))
    {
        if (auto resultado = avancar(); !resultado)
        {
            return std::unexpected(std::move(resultado.error()));
        }
        auto proxima = ler_sequencia();
        if (!proxima)
        {
            return std::unexpected(std::move(proxima.error()));
        }
        opcoes.push_back(std::move(*proxima));
    }
    return criar_expressao(Alternativa{.opcoes = std::move(opcoes)},
                           inicio, _ultimo_fim);
}

std::expected<void, ErroSintatico> Parser::ler_regra(Gramatica& gramatica)
{
    const auto inicio = _atual.localizacao._inicio;
    const auto* nome_token = std::get_if<NaoTerminal>(&_atual.tipo);
    if (!nome_token)
    {
        return std::unexpected(esperado("nome de nao terminal"));
    }
    std::string nome = nome_token -> lexema;
    if (auto resultado = avancar(); !resultado)
    {
        return std::unexpected(std::move(resultado.error()));
    }
    if (auto resultado = consumir(Simbolo::Igual); !resultado)
    {
        return std::unexpected(std::move(resultado.error()));
    }
    auto expressao = ler_expressao();
    if (!expressao)
    {
        return std::unexpected(std::move(expressao.error()));
    }
    if (!atual_e(Simbolo::PontoVirgula))
    {
        return std::unexpected(esperado("';'"));
    }
    const auto fim = _atual.localizacao._fim;
    if (auto resultado = avancar(); !resultado)
    {
        return std::unexpected(std::move(resultado.error()));
    }
    gramatica.regras.push_back({
        .nome = std::move(nome),
        .expressao = std::move(*expressao),
        .localizacao = {._inicio = inicio, ._fim = fim}});
    return {};
}

std::expected<Gramatica, ErroSintatico> Parser::analisar()
{
    if (auto resultado = avancar(); !resultado)
    {
        return std::unexpected(std::move(resultado.error()));
    }
    Gramatica gramatica{};
    if (atual_e(PalavraReservada::Tokens))
    {
        if (auto resultado = ler_secao_tokens(gramatica); !resultado)
        {
            return std::unexpected(std::move(resultado.error()));
        }
    }
    if (!atual_e(PalavraReservada::Rules))
    {
        return std::unexpected(esperado("RULES"));
    }
    if (auto resultado = avancar(); !resultado)
    {
        return std::unexpected(std::move(resultado.error()));
    }
    if (!std::holds_alternative<NaoTerminal>(_atual.tipo))
    {
        return std::unexpected(esperado("ao menos uma regra em RULES"));
    }
    while (!std::holds_alternative<Eof>(_atual.tipo))
    {
        if (auto resultado = ler_regra(gramatica); !resultado)
        {
            return std::unexpected(std::move(resultado.error()));
        }
    }
    return gramatica;
}

std::expected<Gramatica, ErroSintatico> analisar_ebnf(std::string fonte)
{
    return Parser{Lexer{Scanner{std::move(fonte)}}}.analisar();
}
}
