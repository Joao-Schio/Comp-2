#include "src/ebnf/Formatacao.hpp"
#include "src/ebnf/Parser.hpp"

#include <print>
#include <string>
#include <string_view>

namespace
{
[[nodiscard]] bool comparar(std::string_view fonte, std::string_view esperado)
{
    auto gramatica = compiladores::ebnf::analisar_ebnf(std::string{fonte});
    if (!gramatica) return false;
    const auto obtido = compiladores::ebnf::formatar_gramatica(*gramatica);
    if (obtido != esperado)
    {
        std::println(stderr, "Obtido:\n{}\nEsperado:\n{}", obtido, esperado);
        return false;
    }
    return true;
}
}

int main(int argc, char** argv)
{
    if (argc != 2) return 1;
    const std::string_view caso{argv[1]};
    if (caso == "sequencia")
        return comparar("RULES inicio = 'a' 'b';",
                       "inicio = 'a' 'b' ;\n") ? 0 : 1;
    if (caso == "precedencia")
        return comparar("RULES inicio = ('a' | 'b') 'c' | ['d'] { 'e' };",
                       "inicio = ( 'a' | 'b' ) 'c' | [ 'd' ] { 'e' } ;\n") ? 0 : 1;
    if (caso == "vazio")
        return comparar("RULES inicio = | 'x' | ;",
                       "inicio =  | 'x' |  ;\n") ? 0 : 1;
    return 1;
}
