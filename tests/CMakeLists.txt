add_executable(teste_lexer
    LexerTests.cpp
)

configurar_alvo_cpp23(teste_lexer)
target_link_libraries(teste_lexer PRIVATE compiladores_core)

# Cada caso usa o mesmo executavel, mas aparece separadamente no CTest.
add_test(NAME Lexer.RegexEscapada COMMAND teste_lexer regex_escapada)
add_test(NAME Lexer.Literal COMMAND teste_lexer literal)
add_test(NAME Lexer.Comentario COMMAND teste_lexer comentario)
add_test(NAME Lexer.RegexVazia COMMAND teste_lexer regex_vazia)

add_executable(teste_parser ParserTests.cpp)
configurar_alvo_cpp23(teste_parser)
target_link_libraries(teste_parser PRIVATE compiladores_core)

add_test(NAME Parser.Expressao COMMAND teste_parser expressao)
add_test(NAME Parser.AlternativaVazia COMMAND teste_parser vazio)
add_test(NAME Parser.Grupos COMMAND teste_parser grupos)
add_test(NAME Parser.TokensExternos COMMAND teste_parser extern)
add_test(NAME Parser.DelimitadorInvalido COMMAND teste_parser delimitador)
add_test(NAME Parser.ErroLexico COMMAND teste_parser lexico)
add_test(NAME Parser.SemRegra COMMAND teste_parser sem_regra)
add_test(NAME Parser.SemPontoVirgula COMMAND teste_parser ponto_virgula)
