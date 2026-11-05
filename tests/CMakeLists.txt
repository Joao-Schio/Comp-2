add_executable(teste_lexer
    LexerTests.cpp
)

configurar_alvo_cpp23(teste_lexer)
target_link_libraries(teste_lexer PRIVATE compiladores_core)

add_test(
    NAME Lexer.Smoke
    COMMAND teste_lexer
)
