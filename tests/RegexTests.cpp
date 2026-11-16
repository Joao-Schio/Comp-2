#include "src/codegen/Regex.hpp"

#include <cstdio>
#include <print>
#include <string_view>
#include <utility>

namespace
{
[[nodiscard]] std::size_t casar(const compiladores::codegen::Automato& dfa,
                                std::string_view texto)
{
    std::size_t melhor = 0;
    int estado = 0;
    for (std::size_t i = 0; i < texto.size(); ++i)
    {
        const auto byte = static_cast<unsigned char>(texto[i]);
        estado = dfa.transicoes[static_cast<std::size_t>(estado)][byte];
        if (estado < 0) break;
        if (dfa.finais[static_cast<std::size_t>(estado)]) melhor = i + 1;
    }
    return melhor;
}

[[nodiscard]] bool testar(std::string_view padrao, std::string_view texto,
                          std::size_t esperado)
{
    auto dfa = compiladores::codegen::compilar_regex(padrao);
    if (!dfa)
    {
        std::println(stderr, "Regex {} recusada: {}", padrao, dfa.error());
        return false;
    }
    const auto encontrado = casar(*dfa, texto);
    if (encontrado != esperado)
    {
        std::println(stderr, "Regex {} contra {}: esperava {}, obteve {}",
                     padrao, texto, esperado, encontrado);
        return false;
    }
    return true;
}

[[nodiscard]] bool invalida(std::string_view padrao)
{
    return !compiladores::codegen::compilar_regex(padrao);
}
}

int main(int argc, char* argv[])
{
    if (argc != 2) return 2;
    const std::string_view nome{argv[1]};
    const bool passou =
        (nome == "digits" && testar("[0-9]+", "123x", 3)) ||
        (nome == "identifier" && testar("[a-zA-Z_][a-zA-Z0-9_]*", "_a7-", 3)) ||
        (nome == "whitespace" && testar(R"([ \t\r\n]+)", " \t\nx", 3)) ||
        (nome == "group" && testar("(ab|cd)+e?", "abcdabex", 7)) ||
        (nome == "optional" && testar("a?b", "b!", 1)) ||
        (nome == "negated" && testar("[^x]+", "abxy", 2)) ||
        (nome == "dot" && testar("a.b", "a#bx", 3) && testar("a.b", "a\nb", 0)) ||
        (nome == "slash" && testar(R"(a\/b)", "a/bx", 3)) ||
        (nome == "empty" && invalida("a*")) ||
        (nome == "inverted" && invalida("[z-a]")) ||
        (nome == "unclosed" && invalida("abc(")) ||
        (nome == "unsupported" && invalida("a{2}"));
    if (!passou) std::println(stderr, "Falha no teste regex: {}", nome);
    return passou ? 0 : 1;
}
