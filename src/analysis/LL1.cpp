#include "src/analysis/Analise.hpp"

#include <format>
#include <type_traits>
#include <utility>

namespace compiladores::analysis
{
namespace
{
[[nodiscard]] Conjunto intersecao(const Conjunto& esquerdo,
                                  const Conjunto& direito,
                                  std::size_t quantidade)
{
    Conjunto resultado{quantidade};
    for (std::size_t indice = 0; indice < quantidade; ++indice)
    {
        if (esquerdo.contem(indice) && direito.contem(indice))
        {
            (void)resultado.adicionar(indice);
        }
    }
    return resultado;
}

void verificar_expressao(const ebnf::Expressao& expressao,
                        std::string_view regra,
                        const Resultado& analise,
                        std::vector<Diagnostico>& diagnosticos)
{
    const auto& info = analise.expressoes.at(&expressao);
    const auto quantidade = analise.terminais.size();
    auto reportar = [&](Gravidade gravidade, std::string mensagem)
    {
        diagnosticos.push_back({gravidade, expressao.localizacao._inicio,
            std::format("conflito LL(1) em {}: {}", regra, mensagem)});
    };

    std::visit([&](const auto& no)
    {
        using Tipo = std::decay_t<decltype(no)>;
        if constexpr (std::is_same_v<Tipo, ebnf::Alternativa>)
        {
            for (std::size_t i = 0; i < no.opcoes.size(); ++i)
            {
                const auto& esquerda = analise.expressoes.at(no.opcoes[i].get());
                for (std::size_t j = i + 1; j < no.opcoes.size(); ++j)
                {
                    const auto& direita = analise.expressoes.at(no.opcoes[j].get());
                    auto comum = intersecao(esquerda.first, direita.first, quantidade);
                    if (!comum.vazio())
                    {
                        reportar(Gravidade::Erro,
                            std::format("alternativas {} e {} comecam com {}",
                                i + 1, j + 1, analise.formatar_conjunto(comum)));
                    }
                    if (esquerda.anulavel && direita.anulavel)
                    {
                        reportar(Gravidade::Erro,
                            std::format("alternativas {} e {} sao anulaveis", i + 1, j + 1));
                    }
                }
                if (esquerda.anulavel)
                {
                    for (std::size_t j = 0; j < no.opcoes.size(); ++j)
                    {
                        if (j == i)
                        {
                            continue;
                        }
                        const auto& outra = analise.expressoes.at(no.opcoes[j].get());
                        auto comum = intersecao(outra.first, info.follow, quantidade);
                        if (!comum.vazio())
                        {
                            reportar(Gravidade::Erro,
                                std::format("alternativa {} anulavel conflita com FOLLOW "
                                            "da alternativa {} em {}",
                                            i + 1, j + 1, analise.formatar_conjunto(comum)));
                        }
                    }
                }
            }
            for (const auto& opcao : no.opcoes)
            {
                verificar_expressao(*opcao, regra, analise, diagnosticos);
            }
        }
        else if constexpr (std::is_same_v<Tipo, ebnf::Opcional> ||
                           std::is_same_v<Tipo, ebnf::Repeticao>)
        {
            const auto& filho = analise.expressoes.at(no.conteudo.get());
            const auto construcao = std::is_same_v<Tipo, ebnf::Opcional>
                ? "opcional" : "repeticao";
            if (filho.anulavel)
            {
                reportar(Gravidade::Erro,
                    std::format("conteudo de {} e anulavel", construcao));
            }
            auto comum = intersecao(filho.first, info.follow, quantidade);
            if (!comum.vazio())
            {
                reportar(Gravidade::Aviso,
                    std::format("{} tem escolha gulosa com {}",
                                construcao, analise.formatar_conjunto(comum)));
            }
            verificar_expressao(*no.conteudo, regra, analise, diagnosticos);
        }
        else if constexpr (std::is_same_v<Tipo, ebnf::Sequencia>)
        {
            for (const auto& filho : no.elementos)
            {
                verificar_expressao(*filho, regra, analise, diagnosticos);
            }
        }
    }, expressao.no);
}
} // namespace

std::vector<Diagnostico> verificar_ll1(const ebnf::Gramatica& gramatica,
                                      const Resultado& analise)
{
    std::vector<Diagnostico> diagnosticos;
    for (const auto& regra : gramatica.regras)
    {
        verificar_expressao(*regra.expressao, regra.nome, analise, diagnosticos);
    }
    return diagnosticos;
}
}
