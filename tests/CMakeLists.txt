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
