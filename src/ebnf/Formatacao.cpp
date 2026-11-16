#include "src/ebnf/Formatacao.hpp"

#include <string>
#include <string_view>
#include <type_traits>
#include <variant>

namespace compiladores::ebnf
{
namespace
{
[[nodiscard]] int precedencia(const Expressao& expressao)
{
    if (std::holds_alternative<Alternativa>(expressao.no)) return 1;
    if (std::holds_alternative<Sequencia>(expressao.no)) return 2;
    return 3;
}

void escrever_literal(std::string& destino, std::string_view texto)
{
    destino += '\'';
    for (char caracter : texto)
    {
        switch (caracter)
        {
        case '\'': destino += "\\'"; break;
        case '\\': destino += "\\\\"; break;
        case '\n': destino += "\\n"; break;
        case '\r': destino += "\\r"; break;
        case '\t': destino += "\\t"; break;
        default: destino += caracter; break;
        }
    }
    destino += '\'';
}

void escrever_expressao(std::string& destino, const Expressao& expressao, int minimo)
{
    const bool parenteses = precedencia(expressao) < minimo;
    if (parenteses) destino += "( ";

    std::visit([&](const auto& no)
    {
        using Tipo = std::decay_t<decltype(no)>;
        if constexpr (std::is_same_v<Tipo, ReferenciaTerminal>)
        {
            if (no.literal) escrever_literal(destino, no.nome);
            else destino += no.nome;
        }
        else if constexpr (std::is_same_v<Tipo, ReferenciaNaoTerminal>)
        {
            destino += no.nome;
        }
        else if constexpr (std::is_same_v<Tipo, Sequencia>)
        {
            bool primeiro = true;
            for (const auto& filho : no.elementos)
            {
                if (!primeiro) destino += ' ';
                escrever_expressao(destino, *filho, 2);
                primeiro = false;
            }
        }
        else if constexpr (std::is_same_v<Tipo, Alternativa>)
        {
            bool primeiro = true;
            for (const auto& filho : no.opcoes)
            {
                if (!primeiro) destino += " | ";
                escrever_expressao(destino, *filho, 1);
                primeiro = false;
            }
        }
        else if constexpr (std::is_same_v<Tipo, Opcional>)
        {
            destino += "[ ";
            escrever_expressao(destino, *no.conteudo, 0);
            destino += " ]";
        }
        else if constexpr (std::is_same_v<Tipo, Repeticao>)
        {
            destino += "{ ";
            escrever_expressao(destino, *no.conteudo, 0);
            destino += " }";
        }
        // Vazio nao emite texto, assim como E_VAZIO do esqueleto C.
    }, expressao.no);

    if (parenteses) destino += " )";
}
} // namespace

std::string formatar_gramatica(const Gramatica& gramatica)
{
    std::string resultado;
    for (const auto& regra : gramatica.regras)
    {
        resultado += regra.nome;
        resultado += " = ";
        escrever_expressao(resultado, *regra.expressao, 0);
        resultado += " ;\n";
    }
    return resultado;
}
} // namespace compiladores::ebnf
