#pragma once

#include "src/ebnf/Gramatica.hpp"

#include <cstddef>
#include <expected>
#include <string>
#include <unordered_map>
#include <vector>

namespace compiladores::analysis
{
enum class Gravidade
{
    Aviso,
    Erro
};

struct Diagnostico
{
    Gravidade gravidade;
    common::Posicao posicao;
    std::string mensagem;
    bool incluir_coluna{true};
};

enum class TipoTerminal
{
    Fim,
    Nomeado,
    Literal
};

struct SimboloTerminal
{
    TipoTerminal tipo;
    std::string nome;
};

// Os indices correspondem ao catalogo de terminais do Resultado.
class Conjunto
{
public:
    explicit Conjunto(std::size_t quantidade = 0) : _bits(quantidade, false) {}

    [[nodiscard]] bool contem(std::size_t indice) const;
    [[nodiscard]] bool adicionar(std::size_t indice);
    [[nodiscard]] bool unir(const Conjunto& outro);
    [[nodiscard]] bool vazio() const noexcept;

private:
    std::vector<bool> _bits;
};

struct Informacao
{
    bool anulavel{};
    Conjunto first;
    Conjunto follow;
};

// Os ponteiros para expressoes sao emprestados: a Gramatica analisada
// precisa permanecer viva enquanto este Resultado for utilizado.
struct Resultado
{
    std::vector<SimboloTerminal> terminais;
    std::unordered_map<std::string, Informacao> nao_terminais;
    std::unordered_map<const ebnf::Expressao*, Informacao> expressoes;
    std::vector<Diagnostico> avisos;

    [[nodiscard]] std::string formatar_conjunto(const Conjunto& conjunto) const;
};

[[nodiscard]] std::expected<Resultado, std::vector<Diagnostico>>
analisar_gramatica(const ebnf::Gramatica& gramatica);

[[nodiscard]] std::vector<Diagnostico> verificar_ll1(
    const ebnf::Gramatica& gramatica, const Resultado& analise);

[[nodiscard]] std::string formatar_tabela_conjuntos(
    const ebnf::Gramatica& gramatica,
    const Resultado& analise);
}
