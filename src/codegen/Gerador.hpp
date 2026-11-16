#pragma once

#include "src/analysis/Analise.hpp"
#include "src/ebnf/Gramatica.hpp"

#include <expected>
#include <string>
#include <string_view>

namespace compiladores::codegen
{
struct ArquivosGerados
{
    std::string cabecalho;
    std::string fonte;
};

// Gera codigo ANSI C89 a partir de uma gramatica previamente validada como LL(1).
// O chamador e responsavel pela gravacao atomica dos arquivos e pelos diagnosticos.
[[nodiscard]] std::expected<ArquivosGerados, std::string>
gerar(const ebnf::Gramatica& gramatica,
     const analysis::Resultado& analise,
     std::string_view nome_cabecalho,
     bool suporte_arvore);
}
