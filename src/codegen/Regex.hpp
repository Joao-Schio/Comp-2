#pragma once

#include <array>
#include <expected>
#include <string>
#include <string_view>
#include <vector>

namespace compiladores::codegen
{
struct Automato
{
    std::vector<std::array<int, 256>> transicoes;
    std::vector<bool> finais;
};

[[nodiscard]] std::expected<Automato, std::string>
compilar_regex(std::string_view padrao);
}
