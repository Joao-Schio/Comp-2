#include "src/codegen/Regex.hpp"

#include <algorithm>
#include <bitset>
#include <cstddef>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace compiladores::codegen
{
namespace
{
using Caracteres = std::bitset<256>;

struct Transicao
{
    Caracteres caracteres;
    int destino;
};

struct Estado
{
    std::vector<int> epsilon;
    std::vector<Transicao> transicoes;
};

struct Fragmento
{
    int inicio;
    int fim;
};

class CompiladorRegex
{
public:
    explicit CompiladorRegex(std::string_view padrao) : _padrao(padrao) {}

    [[nodiscard]] std::expected<Automato, std::string> compilar()
    {
        auto fragmento = alternativa();
        if (!fragmento)
        {
            return std::unexpected(_erro);
        }
        if (!fim())
        {
            return std::unexpected("caractere inesperado na regex na posicao " +
                std::to_string(_posicao + 1));
        }
        return determinizar(*fragmento);
    }

private:
    std::string_view _padrao;
    std::size_t _posicao{};
    std::string _erro;
    std::vector<Estado> _nfa;

    [[nodiscard]] bool fim() const noexcept { return _posicao >= _padrao.size(); }
    [[nodiscard]] char atual() const noexcept { return fim() ? '\0' : _padrao[_posicao]; }

    [[nodiscard]] bool consumir(char c) noexcept
    {
        if (atual() != c || fim()) return false;
        ++_posicao;
        return true;
    }

    [[nodiscard]] std::optional<Fragmento> falhar(std::string mensagem)
    {
        if (_erro.empty()) _erro = std::move(mensagem);
        return std::nullopt;
    }

    [[nodiscard]] int novo_estado()
    {
        const auto indice = static_cast<int>(_nfa.size());
        _nfa.emplace_back();
        return indice;
    }

    [[nodiscard]] Fragmento vazio()
    {
        const int inicio = novo_estado();
        const int fim_ = novo_estado();
        _nfa[inicio].epsilon.push_back(fim_);
        return {inicio, fim_};
    }

    [[nodiscard]] Fragmento caracteres(const Caracteres& conjunto)
    {
        const int inicio = novo_estado();
        const int fim_ = novo_estado();
        _nfa[inicio].transicoes.push_back({conjunto, fim_});
        return {inicio, fim_};
    }

    [[nodiscard]] Fragmento concatenar(Fragmento a, Fragmento b)
    {
        _nfa[a.fim].epsilon.push_back(b.inicio);
        return {a.inicio, b.fim};
    }

    [[nodiscard]] Fragmento alternar(Fragmento a, Fragmento b)
    {
        const int inicio = novo_estado();
        const int fim_ = novo_estado();
        _nfa[inicio].epsilon.push_back(a.inicio);
        _nfa[inicio].epsilon.push_back(b.inicio);
        _nfa[a.fim].epsilon.push_back(fim_);
        _nfa[b.fim].epsilon.push_back(fim_);
        return {inicio, fim_};
    }

    [[nodiscard]] Fragmento quantificar(Fragmento a, char operador)
    {
        const int inicio = novo_estado();
        const int fim_ = novo_estado();
        _nfa[inicio].epsilon.push_back(a.inicio);
        _nfa[a.fim].epsilon.push_back(fim_);
        if (operador == '*' || operador == '?') _nfa[inicio].epsilon.push_back(fim_);
        if (operador == '*' || operador == '+') _nfa[a.fim].epsilon.push_back(a.inicio);
        return {inicio, fim_};
    }

    [[nodiscard]] std::optional<Fragmento> alternativa()
    {
        auto esquerda = sequencia();
        if (!esquerda) return std::nullopt;
        while (consumir('|'))
        {
            auto direita = sequencia();
            if (!direita) return std::nullopt;
            esquerda = alternar(*esquerda, *direita);
        }
        return esquerda;
    }

    [[nodiscard]] std::optional<Fragmento> sequencia()
    {
        std::optional<Fragmento> resultado;
        while (!fim() && atual() != ')' && atual() != '|')
        {
            auto elemento = repeticao();
            if (!elemento) return std::nullopt;
            resultado = resultado ? concatenar(*resultado, *elemento) : *elemento;
        }
        return resultado ? resultado : std::optional<Fragmento>{vazio()};
    }

    [[nodiscard]] std::optional<Fragmento> repeticao()
    {
        auto elemento = atomo();
        if (!elemento) return std::nullopt;
        if (atual() == '*' || atual() == '+' || atual() == '?')
        {
            const char operador = _padrao[_posicao++];
            elemento = quantificar(*elemento, operador);
            if (atual() == '*' || atual() == '+' || atual() == '?')
                return falhar("quantificadores consecutivos nao sao suportados");
        }
        return elemento;
    }

    [[nodiscard]] std::optional<unsigned char> escapado()
    {
        if (fim())
        {
            _erro = "escape incompleto na regex";
            return std::nullopt;
        }
        const char c = _padrao[_posicao++];
        switch (c)
        {
        case 'n': return static_cast<unsigned char>('\n');
        case 'r': return static_cast<unsigned char>('\r');
        case 't': return static_cast<unsigned char>('\t');
        case '0':
            _erro = "byte NUL nao e suportado em regex de entrada textual";
            return std::nullopt;
        case '\\': case '/': case '|': case '(': case ')':
        case '[': case ']': case '{': case '}': case '*': case '+':
        case '?': case '-': case '.': case '^': case '$':
            return static_cast<unsigned char>(c);
        default:
            _erro = "escape nao suportado na regex: \\" + std::string(1, c);
            return std::nullopt;
        }
    }

    [[nodiscard]] std::optional<Fragmento> classe()
    {
        const bool negar = consumir('^');
        Caracteres conjunto;
        bool primeiro = true;
        while (!fim() && atual() != ']')
        {
            unsigned char primeiro_char;
            if (consumir('\\'))
            {
                auto e = escapado();
                if (!e) return std::nullopt;
                primeiro_char = *e;
            }
            else primeiro_char = static_cast<unsigned char>(_padrao[_posicao++]);

            if (atual() == '-' && _posicao + 1 < _padrao.size() &&
                _padrao[_posicao + 1] != ']')
            {
                ++_posicao;
                unsigned char ultimo;
                if (consumir('\\'))
                {
                    auto e = escapado();
                    if (!e) return std::nullopt;
                    ultimo = *e;
                }
                else ultimo = static_cast<unsigned char>(_padrao[_posicao++]);
                if (ultimo < primeiro_char) return falhar("intervalo invertido em classe de regex");
                for (unsigned int c = primeiro_char; c <= ultimo; ++c) conjunto.set(c);
            }
            else conjunto.set(primeiro_char);
            primeiro = false;
        }
        if (primeiro || !consumir(']')) return falhar("classe de caracteres nao terminada ou vazia");
        if (negar) conjunto.flip();
        conjunto.reset(0); // Fonte C e terminada por NUL.
        return caracteres(conjunto);
    }

    [[nodiscard]] std::optional<Fragmento> atomo()
    {
        if (consumir('('))
        {
            auto conteudo = alternativa();
            if (!conteudo) return std::nullopt;
            if (!consumir(')')) return falhar("grupo de regex nao terminado");
            return conteudo;
        }
        if (consumir('[')) return classe();
        if (consumir('.'))
        {
            Caracteres conjunto;
            conjunto.set();
            conjunto.reset(0);
            conjunto.reset(static_cast<unsigned char>('\n'));
            return caracteres(conjunto);
        }
        if (consumir('\\'))
        {
            auto c = escapado();
            if (!c) return std::nullopt;
            Caracteres conjunto;
            conjunto.set(*c);
            return caracteres(conjunto);
        }
        if (fim() || atual() == '*' || atual() == '+' || atual() == '?' ||
            atual() == ')' || atual() == '|' || atual() == '{' || atual() == '}' ||
            atual() == ']')
            return falhar("metacaractere invalido na regex na posicao " +
                          std::to_string(_posicao + 1));
        Caracteres conjunto;
        conjunto.set(static_cast<unsigned char>(_padrao[_posicao++]));
        return caracteres(conjunto);
    }

    [[nodiscard]] std::vector<int> fecho(std::vector<int> estados) const
    {
        std::vector<bool> visto(_nfa.size(), false);
        for (const int estado : estados) visto[static_cast<std::size_t>(estado)] = true;
        for (std::size_t i = 0; i < estados.size(); ++i)
        {
            for (int destino : _nfa[static_cast<std::size_t>(estados[i])].epsilon)
            {
                if (!visto[static_cast<std::size_t>(destino)])
                {
                    visto[static_cast<std::size_t>(destino)] = true;
                    estados.push_back(destino);
                }
            }
        }
        std::ranges::sort(estados);
        return estados;
    }

    [[nodiscard]] std::expected<Automato, std::string> determinizar(Fragmento fragmento) const
    {
        Automato dfa;
        std::vector<std::vector<int>> conjuntos;
        std::map<std::vector<int>, int> indices;
        auto inicial = fecho({fragmento.inicio});
        indices.emplace(inicial, 0);
        conjuntos.push_back(std::move(inicial));
        for (std::size_t i = 0; i < conjuntos.size(); ++i)
        {
            if (conjuntos.size() > 512)
                return std::unexpected("regex gera automato com mais de 512 estados");
            std::array<int, 256> linha;
            linha.fill(-1);
            for (unsigned int c = 1; c < 256; ++c)
            {
                std::vector<int> proximos;
                std::vector<bool> visto(_nfa.size(), false);
                for (int estado : conjuntos[i])
                {
                    for (const auto& t : _nfa[static_cast<std::size_t>(estado)].transicoes)
                    {
                        if (t.caracteres.test(c) && !visto[static_cast<std::size_t>(t.destino)])
                        {
                            proximos.push_back(t.destino);
                            visto[static_cast<std::size_t>(t.destino)] = true;
                        }
                    }
                }
                if (proximos.empty()) continue;
                proximos = fecho(std::move(proximos));
                const auto [it, inserted] = indices.emplace(proximos, static_cast<int>(conjuntos.size()));
                if (inserted) conjuntos.push_back(std::move(proximos));
                linha[c] = it -> second;
            }
            dfa.transicoes.push_back(std::move(linha));
            dfa.finais.push_back(std::ranges::binary_search(conjuntos[i], fragmento.fim));
        }
        if (dfa.finais.front())
            return std::unexpected("regex aceita a cadeia vazia, o que impediria o lexer de avancar");
        return dfa;
    }
};
}

std::expected<Automato, std::string> compilar_regex(std::string_view padrao)
{
    if (padrao.empty()) return std::unexpected("regex vazia");
    return CompiladorRegex{padrao}.compilar();
}
}
