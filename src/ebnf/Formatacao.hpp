#pragma once

#include "src/ebnf/Gramatica.hpp"

#include <string>

namespace compiladores::ebnf
{
// Reconstitui as producoes da AST em EBNF, sem modificar a gramatica.
// Mesma precedencia e disposicao basica de gram_imprimir do esqueleto C.
[[nodiscard]] std::string formatar_gramatica(const Gramatica& gramatica);
}
