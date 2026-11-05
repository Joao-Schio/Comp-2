#include "src/ebnf/Lexer.hpp"

#include <cstdlib>
#include <iostream>
#include <string_view>
#include <variant>

namespace
{
using compiladores::ebnf::Eof;
using compiladores::ebnf::ExpressaoRegular;
using compiladores::ebnf::Lexer;
using compiladores::ebnf::Literal;
using compiladores::ebnf::PalavraReservada;
using compiladores::ebnf::Scanner;
using compiladores::ebnf::Simbolo;

bool testar_regex_escapada()
{
    Lexer lexer{Scanner{R"(/a\/b/;)"}};

    const auto token = lexer.get_prox_token();
    const auto* regex = token
        ? std::get_if<ExpressaoRegular>(&token->tipo)
        : nullptr;

    if (!regex || regex->padrao != R"(a\/b)" ||
        token->localizacao._inicio.offset != 0 ||
        token->localizacao._fim.offset != 6)
    {
        std::cerr << "Regex escapada ou localizacao incorreta\n";
        return false;
    }

    const auto proximo = lexer.get_prox_token();
    const auto* simbolo = proximo
        ? std::get_if<Simbolo>(&proximo->tipo)
        : nullptr;

    if (!simbolo || *simbolo != Simbolo::PontoVirgula)
    {
        std::cerr << "Delimitador final da regex nao consumido\n";
        return false;
    }

    const auto fim = lexer.get_prox_token();
    if (!fim || !std::holds_alternative<Eof>(fim->tipo))
    {
        std::cerr << "EOF nao reconhecido depois da regex\n";
        return false;
    }

    return true;
}

bool testar_literal()
{
    Lexer lexer{Scanner{R"("if")"}};

    const auto token = lexer.get_prox_token();
    const auto* literal = token
        ? std::get_if<Literal>(&token->tipo)
        : nullptr;

    if (!literal || literal->valor != "if")
    {
        std::cerr << "Literal nao reconhecido\n";
        return false;
    }

    return true;
}

bool testar_comentario_e_palavra_reservada()
{
    Lexer lexer{Scanner{"(* comentario *)  TOKENS"}};

    const auto token = lexer.get_prox_token();
    const auto* palavra = token
        ? std::get_if<PalavraReservada>(&token->tipo)
        : nullptr;

    if (!palavra || *palavra != PalavraReservada::Tokens)
    {
        std::cerr << "Comentario ou palavra reservada nao reconhecido\n";
        return false;
    }

    return true;
}

bool testar_regex_vazia()
{
    Lexer lexer{Scanner{"//"}};

    const auto resultado = lexer.get_prox_token();
    if (resultado || resultado.error().mensagem != "regex vazia")
    {
        std::cerr << "Regex vazia nao produz o erro esperado\n";
        return false;
    }

    return true;
}
} // namespace

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Informe o teste a executar\n";
        return EXIT_FAILURE;
    }

    const std::string_view teste{argv[1]};
    bool passou = false;

    if (teste == "regex_escapada")
    {
        passou = testar_regex_escapada();
    }
    else if (teste == "literal")
    {
        passou = testar_literal();
    }
    else if (teste == "comentario")
    {
        passou = testar_comentario_e_palavra_reservada();
    }
    else if (teste == "regex_vazia")
    {
        passou = testar_regex_vazia();
    }
    else
    {
        std::cerr << "Teste desconhecido: " << teste << '\n';
        return EXIT_FAILURE;
    }

    return passou ? EXIT_SUCCESS : EXIT_FAILURE;
}
