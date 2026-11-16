#include "src/analysis/Analise.hpp"
#include "src/ebnf/Parser.hpp"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <print>
#include <string>
#include <string_view>
#include <utility>

namespace
{
void imprimir_diagnostico(std::string_view arquivo, const compiladores::analysis::Diagnostico& d)
{
    const auto gravidade = d.gravidade == compiladores::analysis::Gravidade::Erro
        ? "erro" : "aviso";
    if (d.posicao.linha == 0)
    {
        std::println(stderr, "{}: {}: {}", arquivo, gravidade, d.mensagem);
    }
    else if (!d.incluir_coluna || d.posicao.coluna == 0)
    {
        std::println(stderr, "{}:{}: {}: {}", arquivo, d.posicao.linha,
                     gravidade, d.mensagem);
    }
    else
    {
        std::println(stderr, "{}:{}:{}: {}: {}", arquivo, d.posicao.linha,
                     d.posicao.coluna, gravidade, d.mensagem);
    }
}
} // namespace

int main(int argc, char* argv[])
{
    // Modos temporarios enquanto a geracao de ANSI C esta em desenvolvimento.
    if (argc != 3 || (std::string_view{argv[1]} != "--parse-only" &&
                      std::string_view{argv[1]} != "--sets" &&
                      std::string_view{argv[1]} != "--check"))
    {
        std::println(stderr,
            "Uso temporario: gerador [--parse-only|--check|--sets] gramatica.ebnf");
        return EXIT_FAILURE;
    }

    std::ifstream arquivo{argv[2], std::ios::binary};
    if (!arquivo)
    {
        std::println(stderr, "Nao foi possivel abrir: {}", argv[2]);
        return EXIT_FAILURE;
    }
    std::string fonte{std::istreambuf_iterator<char>{arquivo},
                      std::istreambuf_iterator<char>{}};
    if (arquivo.bad())
    {
        std::println(stderr, "Falha ao ler: {}", argv[2]);
        return EXIT_FAILURE;
    }

    auto gramatica = compiladores::ebnf::analisar_ebnf(std::move(fonte));
    if (!gramatica)
    {
        const auto& erro = gramatica.error();
        std::println(stderr, "{}:{}:{}: erro: {}", argv[2],
                     erro.posicao.linha, erro.posicao.coluna, erro.mensagem);
        return EXIT_FAILURE;
    }
    
    if (std::string_view{argv[1]} == "--parse-only")
    {
        std::println("EBNF analisada: {} tokens, {} skips, {} regras",
                     gramatica -> tokens.size(), gramatica -> skips.size(),
                     gramatica -> regras.size());
        return EXIT_SUCCESS;
    }

    auto analise = compiladores::analysis::analisar_gramatica(*gramatica);
    if (!analise)
    {
        for (const auto& diagnostico : analise.error())
        {
            imprimir_diagnostico(argv[2], diagnostico);
        }
        return EXIT_FAILURE;
    }
    for (const auto& aviso : analise -> avisos)
    {
        imprimir_diagnostico(argv[2], aviso);
    }
    if (std::string_view{argv[1]} == "--sets")
    {
        std::print("{}", compiladores::analysis::formatar_tabela_conjuntos(
                            *gramatica, *analise));
        (void)std::fflush(stdout);
    }

    const auto conflitos = compiladores::analysis::verificar_ll1(*gramatica, *analise);
    bool erro_ll1 = false;
    for (const auto& diagnostico : conflitos)
    {
        imprimir_diagnostico(argv[2], diagnostico);
        erro_ll1 |= diagnostico.gravidade == compiladores::analysis::Gravidade::Erro;
    }
    if (erro_ll1)
    {
        std::println(stderr,
            "{}: a gramatica nao e LL(1); nenhum codigo foi gerado", argv[2]);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
