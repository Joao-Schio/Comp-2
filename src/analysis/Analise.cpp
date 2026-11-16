#include "src/analysis/Analise.hpp"

#include <algorithm>
#include <cassert>
#include <format>
#include <functional>
#include <iterator>
#include <string_view>
#include <type_traits>
#include <unordered_set>
#include <utility>

namespace compiladores::analysis
{
namespace
{
using ebnf::Expressao;
using ebnf::Gramatica;

template<class T>
inline constexpr bool sempre_falso = false;

[[nodiscard]] Informacao info_vazia(std::size_t quantidade)
{
    return Informacao{.anulavel = false, .first = Conjunto{quantidade},
                      .follow = Conjunto{quantidade}};
}

template<class F>
void visitar_referencias(const Expressao& expressao, F& funcao)
{
    std::visit([&](const auto& no)
    {
        using Tipo = std::decay_t<decltype(no)>;
        if constexpr (std::is_same_v<Tipo, ebnf::ReferenciaTerminal> ||
                      std::is_same_v<Tipo, ebnf::ReferenciaNaoTerminal>)
        {
            funcao(no, expressao.localizacao._inicio);
        }
        else if constexpr (std::is_same_v<Tipo, ebnf::Sequencia>)
        {
            for (const auto& filho : no.elementos)
            {
                visitar_referencias(*filho, funcao);
            }
        }
        else if constexpr (std::is_same_v<Tipo, ebnf::Alternativa>)
        {
            for (const auto& filho : no.opcoes)
            {
                visitar_referencias(*filho, funcao);
            }
        }
        else if constexpr (std::is_same_v<Tipo, ebnf::Opcional> ||
                           std::is_same_v<Tipo, ebnf::Repeticao>)
        {
            visitar_referencias(*no.conteudo, funcao);
        }
    }, expressao.no);
}

template<class F>
void visitar_expressoes(const Expressao& expressao, F& funcao)
{
    funcao(expressao);
    std::visit([&](const auto& no)
    {
        using Tipo = std::decay_t<decltype(no)>;
        if constexpr (std::is_same_v<Tipo, ebnf::Sequencia>)
        {
            for (const auto& filho : no.elementos)
            {
                visitar_expressoes(*filho, funcao);
            }
        }
        else if constexpr (std::is_same_v<Tipo, ebnf::Alternativa>)
        {
            for (const auto& filho : no.opcoes)
            {
                visitar_expressoes(*filho, funcao);
            }
        }
        else if constexpr (std::is_same_v<Tipo, ebnf::Opcional> ||
                           std::is_same_v<Tipo, ebnf::Repeticao>)
        {
            visitar_expressoes(*no.conteudo, funcao);
        }
    }, expressao.no);
}

[[nodiscard]] bool produtiva(
    const Expressao& expressao,
    const std::unordered_map<std::string, bool>& produtivos)
{
    return std::visit([&](const auto& no) -> bool
    {
        using Tipo = std::decay_t<decltype(no)>;
        if constexpr (std::is_same_v<Tipo, ebnf::Vazio> ||
                      std::is_same_v<Tipo, ebnf::ReferenciaTerminal> ||
                      std::is_same_v<Tipo, ebnf::Opcional> ||
                      std::is_same_v<Tipo, ebnf::Repeticao>)
        {
            return true;
        }
        else if constexpr (std::is_same_v<Tipo, ebnf::ReferenciaNaoTerminal>)
        {
            const auto encontrado = produtivos.find(no.nome);
            return encontrado != produtivos.end() && encontrado -> second;
        }
        else if constexpr (std::is_same_v<Tipo, ebnf::Sequencia>)
        {
            return std::ranges::all_of(no.elementos, [&](const auto& filho)
            {
                return produtiva(*filho, produtivos);
            });
        }
        else if constexpr (std::is_same_v<Tipo, ebnf::Alternativa>)
        {
            return std::ranges::any_of(no.opcoes, [&](const auto& filho)
            {
                return produtiva(*filho, produtivos);
            });
        }
        else
        {
            static_assert(sempre_falso<Tipo>);
        }
    }, expressao.no);
}

[[nodiscard]] std::vector<Diagnostico> validar_basico(const Gramatica& gramatica)
{
    std::vector<Diagnostico> diagnosticos;
    std::unordered_set<std::string> tokens;
    std::unordered_set<std::string> regras;

    for (const auto& token : gramatica.tokens)
    {
        if (!tokens.insert(token.nome).second)
        {
            diagnosticos.push_back({Gravidade::Erro, token.localizacao._inicio,
                std::format("terminal {} declarado mais de uma vez", token.nome)});
        }
    }
    for (const auto& regra : gramatica.regras)
    {
        if (!regras.insert(regra.nome).second)
        {
            diagnosticos.push_back({Gravidade::Erro, regra.localizacao._inicio,
                std::format("nao terminal {} definido mais de uma vez", regra.nome)});
        }
    }
    if (gramatica.regras.empty())
    {
        diagnosticos.push_back({Gravidade::Erro, {}, "a gramatica nao tem nenhuma regra"});
    }

    std::unordered_set<std::string> nao_definidos;
    std::unordered_set<std::string> nao_declarados;
    for (const auto& regra : gramatica.regras)
    {
        auto verificar = [&](const auto& referencia, common::Posicao posicao)
        {
            using Tipo = std::decay_t<decltype(referencia)>;
            if constexpr (std::is_same_v<Tipo, ebnf::ReferenciaNaoTerminal>)
            {
                if (!regras.contains(referencia.nome) &&
                    nao_definidos.insert(referencia.nome).second)
                {
                    diagnosticos.push_back({Gravidade::Erro, posicao,
                        std::format("nao terminal {} usado mas nao definido", referencia.nome)});
                }
            }
            else if constexpr (std::is_same_v<Tipo, ebnf::ReferenciaTerminal>)
            {
                if (!referencia.literal && !gramatica.tokens_externos &&
                    !tokens.contains(referencia.nome) &&
                    nao_declarados.insert(referencia.nome).second)
                {
                    diagnosticos.push_back({Gravidade::Erro, posicao,
                        std::format("terminal {} nao declarado em TOKENS", referencia.nome)});
                }
            }
        };
        visitar_referencias(*regra.expressao, verificar);
    }
    if (std::ranges::any_of(diagnosticos, [](const Diagnostico& d)
        { return d.gravidade == Gravidade::Erro; }))
    {
        return diagnosticos;
    }

    std::unordered_set<std::string> acessiveis{gramatica.regras.front().nome};
    bool mudou = true;
    while (mudou)
    {
        mudou = false;
        for (const auto& regra : gramatica.regras)
        {
            if (!acessiveis.contains(regra.nome))
            {
                continue;
            }
            auto adicionar = [&](const auto& referencia, common::Posicao)
            {
                using Tipo = std::decay_t<decltype(referencia)>;
                if constexpr (std::is_same_v<Tipo, ebnf::ReferenciaNaoTerminal>)
                {
                    mudou |= acessiveis.insert(referencia.nome).second;
                }
            };
            visitar_referencias(*regra.expressao, adicionar);
        }
    }
    for (const auto& regra : gramatica.regras)
    {
        if (!acessiveis.contains(regra.nome))
        {
            diagnosticos.push_back({Gravidade::Aviso, regra.localizacao._inicio,
                std::format("nao terminal {} inalcancavel", regra.nome), false});
        }
    }

    std::unordered_map<std::string, bool> produtivos;
    for (const auto& regra : gramatica.regras)
    {
        produtivos.emplace(regra.nome, false);
    }
    mudou = true;
    while (mudou)
    {
        mudou = false;
        for (const auto& regra : gramatica.regras)
        {
            if (!produtivos.at(regra.nome) && produtiva(*regra.expressao, produtivos))
            {
                produtivos[regra.nome] = true;
                mudou = true;
            }
        }
    }
    for (const auto& regra : gramatica.regras)
    {
        if (!produtivos.at(regra.nome))
        {
            diagnosticos.push_back({Gravidade::Erro, regra.localizacao._inicio,
                std::format("nao terminal {} improdutivo", regra.nome), false});
        }
    }
    return diagnosticos;
}

[[nodiscard]] std::size_t indice_terminal(
    const Resultado& resultado, TipoTerminal tipo, std::string_view nome)
{
    for (std::size_t i = 0; i < resultado.terminais.size(); ++i)
    {
        if (resultado.terminais[i].tipo == tipo && resultado.terminais[i].nome == nome)
        {
            return i;
        }
    }
    // Apenas chamado para simbolos previamente registrados na gramatica.
    return resultado.terminais.size();
}

void registrar_terminal(Resultado& resultado, TipoTerminal tipo, const std::string& nome)
{
    if (indice_terminal(resultado, tipo, nome) == resultado.terminais.size())
    {
        resultado.terminais.push_back({tipo, nome});
    }
}

void iniciar_atributos(const Gramatica& gramatica, Resultado& resultado)
{
    resultado.terminais.push_back({TipoTerminal::Fim, "$"});
    for (const auto& token : gramatica.tokens)
    {
        registrar_terminal(resultado, TipoTerminal::Nomeado, token.nome);
    }
    for (const auto& regra : gramatica.regras)
    {
        auto registrar = [&](const auto& referencia, common::Posicao)
        {
            using Tipo = std::decay_t<decltype(referencia)>;
            if constexpr (std::is_same_v<Tipo, ebnf::ReferenciaTerminal>)
            {
                registrar_terminal(resultado,
                    referencia.literal ? TipoTerminal::Literal : TipoTerminal::Nomeado,
                    referencia.nome);
            }
        };
        visitar_referencias(*regra.expressao, registrar);
    }

    const auto quantidade = resultado.terminais.size();
    for (const auto& regra : gramatica.regras)
    {
        resultado.nao_terminais.emplace(regra.nome, info_vazia(quantidade));
        auto iniciar = [&](const Expressao& expressao)
        {
            resultado.expressoes.emplace(&expressao, info_vazia(quantidade));
        };
        visitar_expressoes(*regra.expressao, iniciar);
    }
}

bool calcular_anulavel(const Expressao& expressao, Resultado& resultado, bool& mudou)
{
    const bool valor = std::visit([&](const auto& no) -> bool
    {
        using Tipo = std::decay_t<decltype(no)>;
        if constexpr (std::is_same_v<Tipo, ebnf::Vazio> ||
                      std::is_same_v<Tipo, ebnf::Opcional> ||
                      std::is_same_v<Tipo, ebnf::Repeticao>)
        {
            if constexpr (!std::is_same_v<Tipo, ebnf::Vazio>)
            {
                (void)calcular_anulavel(*no.conteudo, resultado, mudou);
            }
            return true;
        }
        else if constexpr (std::is_same_v<Tipo, ebnf::ReferenciaTerminal>)
        {
            return false;
        }
        else if constexpr (std::is_same_v<Tipo, ebnf::ReferenciaNaoTerminal>)
        {
            return resultado.nao_terminais.at(no.nome).anulavel;
        }
        else if constexpr (std::is_same_v<Tipo, ebnf::Sequencia>)
        {
            bool todos = true;
            for (const auto& filho : no.elementos)
            {
                const bool anulavel = calcular_anulavel(*filho, resultado, mudou);
                todos = todos && anulavel;
            }
            return todos;
        }
        else if constexpr (std::is_same_v<Tipo, ebnf::Alternativa>)
        {
            bool algum = false;
            for (const auto& filho : no.opcoes)
            {
                const bool anulavel = calcular_anulavel(*filho, resultado, mudou);
                algum = algum || anulavel;
            }
            return algum;
        }
        else
        {
            static_assert(sempre_falso<Tipo>);
        }
    }, expressao.no);

    auto& info = resultado.expressoes.at(&expressao);
    if (valor && !info.anulavel)
    {
        info.anulavel = true;
        mudou = true;
    }
    return info.anulavel;
}

void calcular_first(const Expressao& expressao, Resultado& resultado, bool& mudou)
{
    auto& info = resultado.expressoes.at(&expressao);
    std::visit([&](const auto& no)
    {
        using Tipo = std::decay_t<decltype(no)>;
        if constexpr (std::is_same_v<Tipo, ebnf::ReferenciaTerminal>)
        {
            const auto indice = indice_terminal(resultado,
                no.literal ? TipoTerminal::Literal : TipoTerminal::Nomeado, no.nome);
            mudou |= info.first.adicionar(indice);
        }
        else if constexpr (std::is_same_v<Tipo, ebnf::ReferenciaNaoTerminal>)
        {
            mudou |= info.first.unir(resultado.nao_terminais.at(no.nome).first);
        }
        else if constexpr (std::is_same_v<Tipo, ebnf::Sequencia>)
        {
            bool prefixo_anulavel = true;
            for (const auto& filho : no.elementos)
            {
                calcular_first(*filho, resultado, mudou);
                const auto& filho_info = resultado.expressoes.at(filho.get());
                if (prefixo_anulavel)
                {
                    mudou |= info.first.unir(filho_info.first);
                }
                prefixo_anulavel = prefixo_anulavel && filho_info.anulavel;
            }
        }
        else if constexpr (std::is_same_v<Tipo, ebnf::Alternativa>)
        {
            for (const auto& filho : no.opcoes)
            {
                calcular_first(*filho, resultado, mudou);
                mudou |= info.first.unir(resultado.expressoes.at(filho.get()).first);
            }
        }
        else if constexpr (std::is_same_v<Tipo, ebnf::Opcional> ||
                           std::is_same_v<Tipo, ebnf::Repeticao>)
        {
            calcular_first(*no.conteudo, resultado, mudou);
            mudou |= info.first.unir(resultado.expressoes.at(no.conteudo.get()).first);
        }
    }, expressao.no);
}

void propagar_follow(const Expressao& expressao, Resultado& resultado, bool& mudou)
{
    const auto& info = resultado.expressoes.at(&expressao);
    std::visit([&](const auto& no)
    {
        using Tipo = std::decay_t<decltype(no)>;
        if constexpr (std::is_same_v<Tipo, ebnf::ReferenciaNaoTerminal>)
        {
            mudou |= resultado.nao_terminais.at(no.nome).follow.unir(info.follow);
        }
        else if constexpr (std::is_same_v<Tipo, ebnf::Sequencia>)
        {
            for (std::size_t i = 0; i < no.elementos.size(); ++i)
            {
                Conjunto depois{resultado.terminais.size()};
                bool sufixo_anulavel = true;
                for (std::size_t j = i + 1; j < no.elementos.size(); ++j)
                {
                    const auto& seguinte = resultado.expressoes.at(no.elementos[j].get());
                    (void)depois.unir(seguinte.first);
                    if (!seguinte.anulavel)
                    {
                        sufixo_anulavel = false;
                        break;
                    }
                }
                if (sufixo_anulavel)
                {
                    (void)depois.unir(info.follow);
                }
                auto& filho = resultado.expressoes.at(no.elementos[i].get());
                mudou |= filho.follow.unir(depois);
                propagar_follow(*no.elementos[i], resultado, mudou);
            }
        }
        else if constexpr (std::is_same_v<Tipo, ebnf::Alternativa>)
        {
            for (const auto& filho : no.opcoes)
            {
                mudou |= resultado.expressoes.at(filho.get()).follow.unir(info.follow);
                propagar_follow(*filho, resultado, mudou);
            }
        }
        else if constexpr (std::is_same_v<Tipo, ebnf::Opcional> ||
                           std::is_same_v<Tipo, ebnf::Repeticao>)
        {
            auto& filho = resultado.expressoes.at(no.conteudo.get());
            mudou |= filho.follow.unir(info.follow);
            if constexpr (std::is_same_v<Tipo, ebnf::Repeticao>)
            {
                mudou |= filho.follow.unir(filho.first);
            }
            propagar_follow(*no.conteudo, resultado, mudou);
        }
    }, expressao.no);
}

void referencias_iniciais(const Expressao& expressao,
                         const Resultado& resultado,
                         std::unordered_set<std::string>& referencias)
{
    std::visit([&](const auto& no)
    {
        using Tipo = std::decay_t<decltype(no)>;
        if constexpr (std::is_same_v<Tipo, ebnf::ReferenciaNaoTerminal>)
        {
            referencias.insert(no.nome);
        }
        else if constexpr (std::is_same_v<Tipo, ebnf::Sequencia>)
        {
            for (const auto& filho : no.elementos)
            {
                referencias_iniciais(*filho, resultado, referencias);
                if (!resultado.expressoes.at(filho.get()).anulavel)
                {
                    break;
                }
            }
        }
        else if constexpr (std::is_same_v<Tipo, ebnf::Alternativa>)
        {
            for (const auto& filho : no.opcoes)
            {
                referencias_iniciais(*filho, resultado, referencias);
            }
        }
        else if constexpr (std::is_same_v<Tipo, ebnf::Opcional> ||
                           std::is_same_v<Tipo, ebnf::Repeticao>)
        {
            referencias_iniciais(*no.conteudo, resultado, referencias);
        }
    }, expressao.no);
}

// Reproduz a ordem em que o parser C do professor registra os simbolos:
// o primeiro uso de um nao terminal pode ocorrer antes de sua definicao.
[[nodiscard]] std::vector<const ebnf::Regra*> ordenar_regras(const Gramatica& gramatica)
{
    std::vector<std::string> nomes;
    std::unordered_set<std::string> conhecidos;
    for (const auto& regra : gramatica.regras)
    {
        if (conhecidos.insert(regra.nome).second) nomes.push_back(regra.nome);
        auto registrar = [&](const auto& referencia, common::Posicao)
        {
            using Tipo = std::decay_t<decltype(referencia)>;
            if constexpr (std::is_same_v<Tipo, ebnf::ReferenciaNaoTerminal>)
            {
                if (conhecidos.insert(referencia.nome).second)
                    nomes.push_back(referencia.nome);
            }
        };
        visitar_referencias(*regra.expressao, registrar);
    }

    std::vector<const ebnf::Regra*> resultado;
    for (const auto& nome : nomes)
    {
        const auto it = std::ranges::find_if(gramatica.regras,
            [&](const ebnf::Regra& regra) { return regra.nome == nome; });
        if (it != gramatica.regras.end()) resultado.push_back(&*it);
    }
    return resultado;
}

[[nodiscard]] std::vector<Diagnostico> detectar_recursao_esquerda(
    const Gramatica& gramatica, const Resultado& resultado)
{
    std::unordered_map<std::string, std::unordered_set<std::string>> grafo;
    for (const auto& regra : gramatica.regras)
    {
        referencias_iniciais(*regra.expressao, resultado, grafo[regra.nome]);
    }

    std::vector<Diagnostico> erros;
    for (const auto* regra_ptr : ordenar_regras(gramatica))
    {
        const auto& regra = *regra_ptr;
        std::unordered_set<std::string> visitados;
        std::function<bool(const std::string&)> alcanca_origem =
            [&](const std::string& atual) -> bool
        {
            for (const auto& destino : grafo.at(atual))
            {
                if (destino == regra.nome)
                {
                    return true;
                }
                if (visitados.insert(destino).second && alcanca_origem(destino))
                {
                    return true;
                }
            }
            return false;
        };
        if (alcanca_origem(regra.nome))
        {
            const auto tipo = grafo.at(regra.nome).contains(regra.nome)
                ? "direta" : "indireta";
            erros.push_back({Gravidade::Erro, regra.localizacao._inicio,
                std::format("nao terminal {} tem recursao a esquerda {}", regra.nome, tipo),
                false});
        }
    }
    return erros;
}
} // namespace

bool Conjunto::contem(std::size_t indice) const
{
    return indice < _bits.size() && _bits[indice];
}

bool Conjunto::adicionar(std::size_t indice)
{
    assert(indice < _bits.size());
    if (indice >= _bits.size() || _bits[indice])
    {
        return false;
    }
    _bits[indice] = true;
    return true;
}

bool Conjunto::unir(const Conjunto& outro)
{
    assert(_bits.size() == outro._bits.size());
    bool mudou = false;
    const auto quantidade = std::min(_bits.size(), outro._bits.size());
    for (std::size_t i = 0; i < quantidade; ++i)
    {
        if (outro._bits[i] && !_bits[i])
        {
            _bits[i] = true;
            mudou = true;
        }
    }
    return mudou;
}

bool Conjunto::vazio() const noexcept
{
    return std::ranges::none_of(_bits, [](bool elemento) { return elemento; });
}

std::string Resultado::formatar_conjunto(const Conjunto& conjunto) const
{
    std::string saida{"{"};
    bool primeiro = true;
    for (std::size_t i = 0; i < terminais.size(); ++i)
    {
        if (!conjunto.contem(i))
        {
            continue;
        }
        if (!primeiro)
        {
            saida += ", ";
        }
        primeiro = false;
        const auto& terminal = terminais[i];
        if (terminal.tipo == TipoTerminal::Literal)
        {
            saida += "'" + terminal.nome + "'";
        }
        else
        {
            saida += terminal.nome;
        }
    }
    saida += "}";
    return saida;
}

std::expected<Resultado, std::vector<Diagnostico>> analisar_gramatica(
    const Gramatica& gramatica)
{
    auto diagnosticos = validar_basico(gramatica);
    if (std::ranges::any_of(diagnosticos, [](const Diagnostico& d)
        { return d.gravidade == Gravidade::Erro; }))
    {
        return std::unexpected(std::move(diagnosticos));
    }

    Resultado resultado{};
    resultado.avisos = std::move(diagnosticos);
    iniciar_atributos(gramatica, resultado);

    bool mudou = true;
    while (mudou)
    {
        mudou = false;
        for (const auto& regra : gramatica.regras)
        {
            const bool anulavel = calcular_anulavel(*regra.expressao, resultado, mudou);
            if (anulavel && !resultado.nao_terminais.at(regra.nome).anulavel)
            {
                resultado.nao_terminais.at(regra.nome).anulavel = true;
                mudou = true;
            }
        }
    }

    auto recursao = detectar_recursao_esquerda(gramatica, resultado);
    if (!recursao.empty())
    {
        resultado.avisos.insert(resultado.avisos.end(),
            std::make_move_iterator(recursao.begin()),
            std::make_move_iterator(recursao.end()));
        return std::unexpected(std::move(resultado.avisos));
    }

    mudou = true;
    while (mudou)
    {
        mudou = false;
        for (const auto& regra : gramatica.regras)
        {
            calcular_first(*regra.expressao, resultado, mudou);
            mudou |= resultado.nao_terminais.at(regra.nome).first.unir(
                resultado.expressoes.at(regra.expressao.get()).first);
        }
    }

    (void)resultado.nao_terminais.at(gramatica.regras.front().nome).follow.adicionar(0);
    mudou = true;
    while (mudou)
    {
        mudou = false;
        for (const auto& regra : gramatica.regras)
        {
            mudou |= resultado.expressoes.at(regra.expressao.get()).follow.unir(
                resultado.nao_terminais.at(regra.nome).follow);
            propagar_follow(*regra.expressao, resultado, mudou);
        }
    }
    return resultado;
}

std::string formatar_tabela_conjuntos(
    const Gramatica& gramatica, const Resultado& analise)
{
    std::size_t largura = std::string_view{"NAO TERMINAL"}.size();
    const auto regras = ordenar_regras(gramatica);
    for (const auto* regra : regras)
    {
        largura = std::max(largura, regra -> nome.size());
    }

    std::string saida = std::format("{:<{}}  ANULAVEL  FIRST / FOLLOW\n",
                                    "NAO TERMINAL", largura);
    for (const auto* regra : regras)
    {
        const auto& info = analise.nao_terminais.at(regra -> nome);
        saida += std::format("{:<{}}  {:<8}  FIRST  = {}\n",
            regra -> nome, largura, info.anulavel ? "sim" : "nao",
            analise.formatar_conjunto(info.first));
        saida += std::format("{:<{}}            FOLLOW = {}\n",
            "", largura, analise.formatar_conjunto(info.follow));
    }
    return saida;
}
}
