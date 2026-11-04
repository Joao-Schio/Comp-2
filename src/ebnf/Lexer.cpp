#include "src/ebnf/Lexer.hpp"
namespace compiladores::ebnf
{
char Scanner::peek() const noexcept
{
    // a versao com offset veio depois
    // e eu nao queria quebrar as dependencias ja existentes
    return this -> peek(0);
}
char Scanner::peek(std::size_t offset) const noexcept
{
    if (offset >= _fonte.size() - _posicao)
    {
        return '\0';
    }

    return _fonte[_posicao + offset];
}

bool Scanner::eof() const noexcept
{
    return this -> _posicao == this -> _fonte.size();
}

char Scanner::advance() noexcept
{
    if (this -> eof())
    {
        return '\0';
    }
    auto saida = this -> peek();
    _posicao++;
    if (saida == '\n')
    {
        _coluna = 1;
        _linha++;
    }
    else
    {
        _coluna++;
    }
    return saida;
}

char Scanner::advance(std::size_t offset) noexcept
{
    char ultimo = '\0';

    for (std::size_t i = 0; i < offset && !this->eof(); i++)
    {
        ultimo = this->advance();
    }

    return ultimo;
}

common::Posicao Scanner::posicao() const noexcept
{
    return 
    {
        .offset = _posicao,
        .linha = _linha,
        .coluna = _coluna
    };
}

std::optional<ErroLexico> Lexer::pular_ignoraveis()
{
    while (!_scanner.eof())
    {
        const char c = _scanner.peek();

        if (std::isspace(static_cast<unsigned char>(c)))
        {
            _scanner.advance();
            continue;
        }

        if (c == '(' && _scanner.peek(1) == '*')
        {
            _scanner.advance(2);

            if (auto erro = pular_comentarios())
            {
                return erro;
            }

            continue;
        }

        break;
    }

    return std::nullopt;
}

std::optional<ErroLexico> Lexer::pular_comentarios()
{
    while(!_scanner.eof())
    {
        auto caractere = _scanner.advance();
        if (caractere == '*' && _scanner.peek() == ')')
        {
            _scanner.advance();
            return std::nullopt;
        }
    }
    return ErroLexico::eof_inesperado(_scanner.posicao());
}

std::expected<Token, ErroLexico>
Lexer::ler_identificador(common::Posicao inicio)
{
    std::string identificador{};
    while(!_scanner.eof())
    {
        const auto c = static_cast<unsigned char>(_scanner.peek());
        if (!std::isalnum(c) && c != '_')
        {
            break;
        }
        identificador.push_back(_scanner.advance());
    }
    
    auto comeca_minuscula = std::islower(identificador.at(0));
    if (!comeca_minuscula)
    {
        const bool contem_minuscula = std::ranges::any_of(
            identificador,
            [](unsigned char c)
            {
                return std::islower(static_cast<unsigned char>(c)) != 0;
            });

        if (contem_minuscula)
        {
            return std::unexpected(
                ErroLexico
                {
                    .mensagem = std::format(
                        "identificador '{}' invalido",
                        identificador),
                    .posicao = inicio
                });
        }
    }

    auto palavra_reservada = this -> ler_palavra_reservada(identificador, inicio);
    if (palavra_reservada.has_value())
    {
        return std::move(*palavra_reservada);
    }
    if (comeca_minuscula)
    {
        return Token
        {
            .tipo = NaoTerminal
            {
                .lexema = std::move(identificador)
            },
            .localizacao =
            {
                ._inicio = inicio,
                ._fim = _scanner.posicao()
            }
        };
    }
    return Token
    {
        .tipo = Terminal
        {
            .lexema = std::move(identificador)
        },
        .localizacao = 
        {
            ._inicio = inicio, 
            ._fim = _scanner.posicao()
        }
    };
}

std::expected<Token, ErroLexico>
Lexer::ler_literal(common::Posicao posicao)
{
    
    
}



std::expected<Token, ErroLexico>
Lexer::get_prox_token()
{
    if (auto erro = this -> pular_ignoraveis())
    {
        return std::unexpected(std::move(*erro));
    }

    const auto inicio = _scanner.posicao();

    if (_scanner.eof())
    {
        return Token{
            .tipo = Eof{},
            .localizacao = {
                ._inicio = inicio,
                ._fim = inicio}};
    }

    const char caracter = _scanner.peek();

    auto simbolo = this -> ler_simbolo(caracter, inicio);
    if (simbolo)
    {
        return std::move(*simbolo);
    }

    if (std::isalpha(caracter) || caracter == '_')
    {
        return this -> ler_identificador(inicio);
    }


    return std::unexpected(
        ErroLexico::caracter_invalido(caracter, inicio));
}
}