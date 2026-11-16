#include "src/analysis/Analise.hpp"
#include "src/codegen/Gerador.hpp"
#include "src/ebnf/Parser.hpp"

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <print>
#include <string>
#include <string_view>
#include <utility>

namespace
{
struct Opcoes
{
    std::string arquivo;
    std::string prefixo;
    bool sets{};
    bool check{};
    bool parse_only{};
    bool tree{};
    bool scanner_microc{};
};

void uso()
{
    std::println(stderr,
        "uso: gerador gramatica.ebnf -o prefixo [--sets] [--tree] [--scanner microc]\n"
        "     gerador --check|--sets|--parse-only gramatica.ebnf");
}

[[nodiscard]] bool ler_argumentos(int argc, char* argv[], Opcoes& opcoes)
{
    for (int i = 1; i < argc; ++i)
    {
        const std::string_view argumento{argv[i]};
        if (argumento == "-o" && i + 1 < argc)
            opcoes.prefixo = argv[++i];
        else if (argumento == "--sets") opcoes.sets = true;
        else if (argumento == "--check") opcoes.check = true;
        else if (argumento == "--parse-only") opcoes.parse_only = true;
        else if (argumento == "--tree") opcoes.tree = true;
        else if (argumento == "--scanner" && i + 1 < argc)
        {
            if (std::string_view{argv[++i]} != "microc") return false;
            opcoes.scanner_microc = true;
        }
        else if (argumento.starts_with('-')) return false;
        else if (opcoes.arquivo.empty()) opcoes.arquivo = argumento;
        else return false;
    }
    return !opcoes.arquivo.empty() &&
        (opcoes.parse_only || opcoes.check || opcoes.sets || !opcoes.prefixo.empty()) &&
        (!opcoes.tree || !opcoes.prefixo.empty());
}

void imprimir_diagnostico(std::string_view arquivo, const compiladores::analysis::Diagnostico& d)
{
    const auto gravidade = d.gravidade == compiladores::analysis::Gravidade::Erro
        ? "erro" : "aviso";
    if (d.posicao.linha == 0)
        std::println(stderr, "{}: {}: {}", arquivo, gravidade, d.mensagem);
    else if (!d.incluir_coluna || d.posicao.coluna == 0)
        std::println(stderr, "{}:{}: {}: {}", arquivo, d.posicao.linha,
                     gravidade, d.mensagem);
    else
        std::println(stderr, "{}:{}:{}: {}: {}", arquivo, d.posicao.linha,
                     d.posicao.coluna, gravidade, d.mensagem);
}

[[nodiscard]] bool salvar(const std::filesystem::path& caminho, const std::string& conteudo)
{
    std::ofstream arquivo{caminho, std::ios::binary | std::ios::trunc};
    if (!arquivo) return false;
    arquivo.write(conteudo.data(), static_cast<std::streamsize>(conteudo.size()));
    arquivo.close();
    return arquivo.good();
}
}

int main(int argc, char* argv[])
{
    Opcoes opcoes;
    if (!ler_argumentos(argc, argv, opcoes))
    {
        uso();
        return EXIT_FAILURE;
    }

    std::ifstream arquivo{opcoes.arquivo, std::ios::binary};
    if (!arquivo)
    {
        std::println(stderr, "gerador: nao foi possivel abrir {}", opcoes.arquivo);
        return EXIT_FAILURE;
    }
    std::string fonte{std::istreambuf_iterator<char>{arquivo},
                      std::istreambuf_iterator<char>{}};
    if (arquivo.bad())
    {
        std::println(stderr, "gerador: falha ao ler {}", opcoes.arquivo);
        return EXIT_FAILURE;
    }

    auto gramatica = compiladores::ebnf::analisar_ebnf(std::move(fonte));
    if (!gramatica)
    {
        const auto& erro = gramatica.error();
        std::println(stderr, "{}:{}:{}: erro: {}", opcoes.arquivo,
                     erro.posicao.linha, erro.posicao.coluna, erro.mensagem);
        return EXIT_FAILURE;
    }
    if (opcoes.parse_only)
    {
        std::println("EBNF analisada: {} tokens, {} skips, {} regras",
                     gramatica -> tokens.size(), gramatica -> skips.size(),
                     gramatica -> regras.size());
        return EXIT_SUCCESS;
    }

    if (opcoes.scanner_microc != gramatica -> tokens_externos.has_value())
    {
        std::println(stderr, "{}: erro: {}", opcoes.arquivo,
            opcoes.scanner_microc
                ? "--scanner microc exige TOKENS EXTERN"
                : "TOKENS EXTERN exige --scanner microc");
        return EXIT_FAILURE;
    }

    auto analise = compiladores::analysis::analisar_gramatica(*gramatica);
    if (!analise)
    {
        for (const auto& diagnostico : analise.error())
            imprimir_diagnostico(opcoes.arquivo, diagnostico);
        return EXIT_FAILURE;
    }
    for (const auto& aviso : analise -> avisos)
        imprimir_diagnostico(opcoes.arquivo, aviso);
    if (opcoes.sets)
    {
        std::print("{}", compiladores::analysis::formatar_tabela_conjuntos(
                            *gramatica, *analise));
        (void)std::fflush(stdout);
    }

    const auto conflitos = compiladores::analysis::verificar_ll1(*gramatica, *analise);
    bool erro_ll1 = false;
    for (const auto& diagnostico : conflitos)
    {
        imprimir_diagnostico(opcoes.arquivo, diagnostico);
        erro_ll1 |= diagnostico.gravidade == compiladores::analysis::Gravidade::Erro;
    }
    if (erro_ll1)
    {
        std::println(stderr, "{}: a gramatica nao e LL(1); nenhum codigo foi gerado",
                     opcoes.arquivo);
        return EXIT_FAILURE;
    }
    if (opcoes.prefixo.empty()) return EXIT_SUCCESS;

    const std::filesystem::path prefixo{opcoes.prefixo};
    auto gerados = compiladores::codegen::gerar(
        *gramatica, *analise, prefixo.filename().string() + ".h", opcoes.tree);
    if (!gerados)
    {
        std::println(stderr, "{}: erro: {}", opcoes.arquivo, gerados.error());
        return EXIT_FAILURE;
    }

    const auto parent = prefixo.parent_path();
    if (!parent.empty())
    {
        std::error_code ec;
        std::filesystem::create_directories(parent, ec);
        if (ec)
        {
            std::println(stderr, "gerador: nao foi possivel criar diretorio {}: {}",
                         parent.string(), ec.message());
            return EXIT_FAILURE;
        }
    }
    if (!salvar(prefixo.string() + ".h", gerados -> cabecalho) ||
        !salvar(prefixo.string() + ".c", gerados -> fonte))
    {
        std::println(stderr, "gerador: erro ao gravar {}.c/.h", prefixo.string());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
