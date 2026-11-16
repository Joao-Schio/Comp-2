#include "src/analysis/Analise.hpp"
#include "src/ebnf/Parser.hpp"

#include <cstdio>
#include <print>
#include <string>
#include <string_view>

namespace
{
namespace ebnf = compiladores::ebnf;
namespace analysis = compiladores::analysis;

bool contem_erro(std::string_view fonte, std::string_view trecho)
{
    auto gramatica = ebnf::analisar_ebnf(std::string{fonte});
    if (!gramatica)
    {
        return false;
    }
    auto resultado = analysis::analisar_gramatica(*gramatica);
    if (resultado)
    {
        return false;
    }
    for (const auto& diagnostico : resultado.error())
    {
        if (diagnostico.gravidade == analysis::Gravidade::Erro &&
            diagnostico.mensagem.find(trecho) != std::string::npos)
        {
            return true;
        }
    }
    return false;
}

bool teste_expr()
{
    auto gramatica = ebnf::analisar_ebnf(R"GRAM((* expressoes *)
TOKENS
  NUM  = /[0-9]+/ ;
  ID   = /[a-zA-Z_][a-zA-Z0-9_]*/ ;
  SKIP = /[ \t\r\n]+/ ;
RULES
  expr   = term { ( "+" | "-" ) term } ;
  term   = factor { ( "*" | "/" ) factor } ;
  factor = NUM | ID | "(" expr ")" ;
)GRAM");
    if (!gramatica)
    {
        return false;
    }
    auto resultado = analysis::analisar_gramatica(*gramatica);
    if (!resultado)
    {
        return false;
    }
    constexpr std::string_view esperado =
        "NAO TERMINAL  ANULAVEL  FIRST / FOLLOW\n"
        "expr          nao       FIRST  = {NUM, ID, '('}\n"
        "                        FOLLOW = {$, ')'}\n"
        "term          nao       FIRST  = {NUM, ID, '('}\n"
        "                        FOLLOW = {$, '+', '-', ')'}\n"
        "factor        nao       FIRST  = {NUM, ID, '('}\n"
        "                        FOLLOW = {$, '+', '-', '*', '/', ')'}\n";
    const auto obtido = analysis::formatar_tabela_conjuntos(*gramatica, *resultado);
    if (obtido != esperado)
    {
        std::println(stderr, "Tabela obtida:\n{}", obtido);
        return false;
    }
    return true;
}

bool teste_vazio()
{
    auto gramatica = ebnf::analisar_ebnf("RULES inicio = ;");
    if (!gramatica)
    {
        return false;
    }
    auto resultado = analysis::analisar_gramatica(*gramatica);
    if (!resultado)
    {
        return false;
    }
    const auto& inicio = resultado -> nao_terminais.at("inicio");
    return inicio.anulavel &&
           resultado -> formatar_conjunto(inicio.first) == "{}" &&
           resultado -> formatar_conjunto(inicio.follow) == "{$}";
}

bool teste_follow_repeticao()
{
    auto gramatica = ebnf::analisar_ebnf("RULES inicio = { 'a' } 'b' ;");
    if (!gramatica)
    {
        return false;
    }
    auto resultado = analysis::analisar_gramatica(*gramatica);
    if (!resultado)
    {
        return false;
    }
    const auto& inicio = resultado -> nao_terminais.at("inicio");
    const auto* sequencia =
        std::get_if<ebnf::Sequencia>(&gramatica -> regras.front().expressao -> no);
    if (!sequencia)
    {
        return false;
    }
    const auto* repeticao = std::get_if<ebnf::Repeticao>(
        &sequencia -> elementos.front() -> no);
    if (!repeticao)
    {
        return false;
    }
    const auto& filho = resultado -> expressoes.at(repeticao -> conteudo.get());
    const auto& repeticao_info =
        resultado -> expressoes.at(sequencia -> elementos.front().get());
    return resultado -> formatar_conjunto(inicio.first) == "{'a', 'b'}" &&
           resultado -> formatar_conjunto(repeticao_info.follow) == "{'b'}" &&
           resultado -> formatar_conjunto(filho.follow) == "{'a', 'b'}";
}

bool teste_nullable_e_follow_indireto()
{
    auto gramatica = ebnf::analisar_ebnf(
        "RULES inicio = a 'z'; a = b; b = 'x' | ;");
    if (!gramatica)
    {
        return false;
    }
    auto resultado = analysis::analisar_gramatica(*gramatica);
    if (!resultado)
    {
        return false;
    }
    const auto& inicio = resultado -> nao_terminais.at("inicio");
    const auto& a = resultado -> nao_terminais.at("a");
    const auto& b = resultado -> nao_terminais.at("b");
    return !inicio.anulavel && a.anulavel && b.anulavel &&
           resultado -> formatar_conjunto(inicio.first) == "{'z', 'x'}" &&
           resultado -> formatar_conjunto(a.follow) == "{'z'}" &&
           resultado -> formatar_conjunto(b.follow) == "{'z'}";
}

bool teste_inalcancavel()
{
    auto gramatica = ebnf::analisar_ebnf("RULES inicio = 'a'; inutil = 'b';");
    if (!gramatica)
    {
        return false;
    }
    auto resultado = analysis::analisar_gramatica(*gramatica);
    return resultado && resultado -> avisos.size() == 1 &&
           resultado -> avisos.front().mensagem.find("inalcancavel") != std::string::npos;
}

bool teste_tokens_externos()
{
    auto gramatica = ebnf::analisar_ebnf(
        "TOKENS EXTERN 'tokens.h'; RULES inicio = ID;");
    return gramatica && analysis::analisar_gramatica(*gramatica).has_value();
}

bool teste_nullable_ciclo_sem_recursao()
{
    auto gramatica = ebnf::analisar_ebnf(
        "RULES inicio = [ a ]; a = 'a' inicio | ;");
    if (!gramatica)
    {
        return false;
    }
    auto resultado = analysis::analisar_gramatica(*gramatica);
    return resultado && resultado -> nao_terminais.at("inicio").anulavel &&
           resultado -> nao_terminais.at("a").anulavel;
}

bool executar(std::string_view nome)
{
    if (nome == "expr") return teste_expr();
    if (nome == "vazio") return teste_vazio();
    if (nome == "follow_repeticao") return teste_follow_repeticao();
    if (nome == "nullable_follow") return teste_nullable_e_follow_indireto();
    if (nome == "inalcancavel") return teste_inalcancavel();
    if (nome == "extern") return teste_tokens_externos();
    if (nome == "ciclo_valido") return teste_nullable_ciclo_sem_recursao();
    if (nome == "terminal_ausente")
        return contem_erro("RULES inicio = NUM;", "terminal NUM nao declarado");
    if (nome == "naoterminal_ausente")
        return contem_erro("RULES inicio = outro;", "nao terminal outro usado mas nao definido");
    if (nome == "token_duplicado")
        return contem_erro("TOKENS NUM=/[0-9]/; NUM=/[0-9]+/; RULES inicio=NUM;",
                          "terminal NUM declarado mais de uma vez");
    if (nome == "regra_duplicada")
        return contem_erro("RULES inicio='a'; inicio='b';",
                          "nao terminal inicio definido mais de uma vez");
    if (nome == "improdutivo")
        return contem_erro("RULES inicio = a; a = a;", "nao terminal inicio improdutivo");
    if (nome == "recursao_direta")
        return contem_erro("RULES inicio = inicio 'a' | 'b';", "recursao a esquerda direta");
    if (nome == "recursao_indireta")
        return contem_erro("RULES inicio = [ 'x' ] a | 'b'; a = inicio | 'z';",
                          "recursao a esquerda indireta");
    return false;
}
}

int main(int argc, char* argv[])
{
    if (argc != 2 || !executar(argv[1]))
    {
        std::println(stderr, "Falha no teste de analise: {}",
                     argc == 2 ? argv[1] : "argumentos invalidos");
        return 1;
    }
    return 0;
}
