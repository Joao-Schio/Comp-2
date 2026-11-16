#include "src/ebnf/Parser.hpp"

#include <iostream>
#include <string_view>
#include <variant>

namespace
{
using namespace compiladores::ebnf;

bool expressao_aritmetica()
{
    auto gramatica = analisar_ebnf(R"GRAM(
        (* comentario *)
        TOKENS
          NUM = /[0-9]+/ ;
          ID = /[a-z]+/ ;
          SKIP = /[ \t\r\n]+/ ;
        RULES
          expr = term { ( "+" | "-" ) term } ;
          term = factor { ( "*" | "/" ) factor } ;
          factor = NUM | ID | "(" expr ")" ;
    )GRAM");
    if (!gramatica || gramatica->tokens.size() != 2 ||
        gramatica->skips.size() != 1 || gramatica->regras.size() != 3 ||
        gramatica->regras.front().nome != "expr")
    {
        return false;
    }
    const auto* seq = std::get_if<Sequencia>(&gramatica->regras.front().expressao->no);
    return seq != nullptr && seq->elementos.size() == 2 &&
           std::holds_alternative<ReferenciaNaoTerminal>(seq->elementos[0]->no) &&
           std::holds_alternative<Repeticao>(seq->elementos[1]->no);
}

bool alternativa_vazia()
{
    auto gramatica = analisar_ebnf("RULES a = | 'x' | ;");
    if (!gramatica || gramatica->regras.size() != 1)
    {
        return false;
    }
    const auto* alternativas =
        std::get_if<Alternativa>(&gramatica->regras.front().expressao->no);
    return alternativas && alternativas->opcoes.size() == 3 &&
           std::holds_alternative<Vazio>(alternativas->opcoes[0]->no) &&
           std::holds_alternative<Vazio>(alternativas->opcoes[2]->no);
}

bool grupos_aninhados()
{
    auto gramatica = analisar_ebnf("RULES a = [ { ( 'x' | 'y' ) } ] ;");
    if (!gramatica)
    {
        return false;
    }
    const auto* opcional = std::get_if<Opcional>(&gramatica->regras[0].expressao->no);
    if (!opcional)
    {
        return false;
    }
    const auto* repeticao = std::get_if<Repeticao>(&opcional->conteudo->no);
    return repeticao != nullptr &&
           std::holds_alternative<Alternativa>(repeticao->conteudo->no);
}

bool tokens_externos()
{
    auto gramatica = analisar_ebnf("TOKENS EXTERN 'tokens.h'; RULES inicio = ID;");
    return gramatica && gramatica->tokens_externos == "tokens.h" &&
           gramatica->tokens.empty() && gramatica->regras.size() == 1;
}

bool erro_delimitador()
{
    auto gramatica = analisar_ebnf("RULES inicio = ( 'a' ;");
    return !gramatica && gramatica.error().mensagem.find("')'") != std::string::npos;
}

bool erro_lexer()
{
    auto gramatica = analisar_ebnf("RULES inicio = @;");
    return !gramatica && gramatica.error().posicao.coluna == 16 &&
           gramatica.error().mensagem.find("invalido") != std::string::npos;
}

bool erro_regra_ausente()
{
    auto gramatica = analisar_ebnf("TOKENS A=/a/; RULES");
    return !gramatica && gramatica.error().mensagem.find("ao menos uma regra") != std::string::npos;
}

bool erro_ponto_virgula()
{
    auto gramatica = analisar_ebnf("RULES inicio = 'a' ");
    return !gramatica && gramatica.error().mensagem.find("';'") != std::string::npos;
}
} // namespace

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Informe o teste\n";
        return 1;
    }
    const std::string_view nome{argv[1]};
    const bool sucesso =
        (nome == "expressao" && expressao_aritmetica()) ||
        (nome == "vazio" && alternativa_vazia()) ||
        (nome == "grupos" && grupos_aninhados()) ||
        (nome == "extern" && tokens_externos()) ||
        (nome == "delimitador" && erro_delimitador()) ||
        (nome == "lexico" && erro_lexer()) ||
        (nome == "sem_regra" && erro_regra_ausente()) ||
        (nome == "ponto_virgula" && erro_ponto_virgula());
    if (!sucesso)
    {
        std::cerr << "Falha: " << nome << '\n';
    }
    return sucesso ? 0 : 1;
}
