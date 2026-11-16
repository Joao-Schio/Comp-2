#include "src/ebnf/Parser.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <string_view>
#include <utility>
#include <print>

int main(int argc, char* argv[])
{
    // Interface provisoria para validar a etapa de leitura EBNF.
    // O modo padrao do gerador sera implementado junto com a geracao ANSI C.
    if (argc != 3 || std::string_view{argv[1]} != "--parse-only")
    {
        std::println(stderr, "Uso temporario: gerador --parse-only gramatica.ebnf");
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
        std::println(stderr,"Falha ao ler: {}", argv[2]);
        return EXIT_FAILURE;
    }

    auto gramatica = compiladores::ebnf::analisar_ebnf(std::move(fonte));
    if (!gramatica)
    {
        const auto& erro = gramatica.error();
        std::println(stderr,"{}:{}:{}: erro: {}", argv[2], erro.posicao.linha,':'
                  , erro.posicao.coluna, erro.mensagem);
        return EXIT_FAILURE;
    }
    std::println(
        "EBNF analisada: {} tokens, {} skips, {} regras", 
        gramatica ->tokens.size(), 
        gramatica -> skips.size(), 
        gramatica -> regras.size()
    );
}
