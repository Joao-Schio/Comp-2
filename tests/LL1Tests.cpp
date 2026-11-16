#include "src/analysis/Analise.hpp"
#include "src/ebnf/Parser.hpp"

#include <print>
#include <cstdio>
#include <string>
#include <string_view>
#include <vector>

namespace
{
namespace ebnf = compiladores::ebnf;
namespace analysis = compiladores::analysis;

bool verificar(std::string_view fonte, std::string_view trecho,
              analysis::Gravidade gravidade, std::size_t quantidade_esperada)
{
    auto gramatica = ebnf::analisar_ebnf(std::string{fonte});
    if (!gramatica)
    {
        return false;
    }
    auto resultado = analysis::analisar_gramatica(*gramatica);
    if (!resultado)
    {
        return false;
    }
    const auto diagnosticos = analysis::verificar_ll1(*gramatica, *resultado);
    if (diagnosticos.size() != quantidade_esperada)
    {
        std::println(stderr, "Esperava {} diagnosticos, recebeu {}",
                     quantidade_esperada, diagnosticos.size());
        return false;
    }
    if (quantidade_esperada == 0)
    {
        return true;
    }
    for (const auto& diagnostico : diagnosticos)
    {
        if (diagnostico.gravidade == gravidade &&
            diagnostico.mensagem.find(trecho) != std::string::npos)
        {
            return true;
        }
    }
    return false;
}

bool executar(std::string_view nome)
{
    if (nome == "first_conflito")
        return verificar("TOKENS ID=/[a-z]+/; RULES inicio=ID 'a' | ID 'b';",
                        "alternativas 1 e 2 comecam com {ID}", analysis::Gravidade::Erro, 1);
    if (nome == "alternativa_valida")
        return verificar("RULES inicio = 'a' | 'b';", "", analysis::Gravidade::Erro, 0);
    if (nome == "duas_anulaveis")
        return verificar("RULES inicio = | ;", "alternativas 1 e 2 sao anulaveis",
                        analysis::Gravidade::Erro, 1);
    if (nome == "nullable_follow")
        return verificar("RULES inicio = a 'x'; a = 'x' | ;", "anulavel conflita com FOLLOW",
                        analysis::Gravidade::Erro, 1);
    if (nome == "opcional_anulavel")
        return verificar("RULES inicio = [ ] ;", "conteudo de opcional e anulavel",
                        analysis::Gravidade::Erro, 1);
    if (nome == "repeticao_anulavel")
        return verificar("RULES inicio = { } ;", "conteudo de repeticao e anulavel",
                        analysis::Gravidade::Erro, 1);
    if (nome == "opcional_guloso")
        return verificar("RULES inicio = [ 'a' ] 'a' ;", "escolha gulosa",
                        analysis::Gravidade::Aviso, 1);
    if (nome == "repeticao_gulosa")
        return verificar("RULES inicio = { 'a' } 'a' ;", "escolha gulosa",
                        analysis::Gravidade::Aviso, 1);
    if (nome == "repeticao_valida")
        return verificar("RULES inicio = { 'a' } 'b' ;", "",
                        analysis::Gravidade::Aviso, 0);
    if (nome == "else_pendente")
        return verificar("RULES inicio = 'if' inicio [ 'else' inicio ] | 'other' ;",
                        "escolha gulosa", analysis::Gravidade::Aviso, 1);
    return false;
}
}

int main(int argc, char* argv[])
{
    if (argc != 2 || !executar(argv[1]))
    {
        std::println(stderr, "Falha no teste LL(1): {}",
                     argc == 2 ? argv[1] : "argumentos invalidos");
        return 1;
    }
    return 0;
}
