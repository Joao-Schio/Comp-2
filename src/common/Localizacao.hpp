#pragma once


#include <cstddef>


namespace compiladores::common
{
struct Posicao
{
    std::size_t offset{};
    std::size_t linha{1};
    std::size_t coluna{1};
};

struct Localizacao
{
    Posicao _inicio;
    Posicao _fim;
};
}