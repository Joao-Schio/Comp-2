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

add_executable(teste_analise AnaliseTests.cpp)
configurar_alvo_cpp23(teste_analise)
target_link_libraries(teste_analise PRIVATE compiladores_core)

add_test(NAME Analise.Expr COMMAND teste_analise expr)
add_test(NAME Analise.Vazio COMMAND teste_analise vazio)
add_test(NAME Analise.FollowRepeticao COMMAND teste_analise follow_repeticao)
add_test(NAME Analise.NullableFollow COMMAND teste_analise nullable_follow)
add_test(NAME Analise.Inalcancavel COMMAND teste_analise inalcancavel)
add_test(NAME Analise.Extern COMMAND teste_analise extern)
add_test(NAME Analise.CicloValido COMMAND teste_analise ciclo_valido)
add_test(NAME Analise.TerminalAusente COMMAND teste_analise terminal_ausente)
add_test(NAME Analise.NaoTerminalAusente COMMAND teste_analise naoterminal_ausente)
add_test(NAME Analise.TokenDuplicado COMMAND teste_analise token_duplicado)
add_test(NAME Analise.RegraDuplicada COMMAND teste_analise regra_duplicada)
add_test(NAME Analise.Improdutivo COMMAND teste_analise improdutivo)
add_test(NAME Analise.RecursaoDireta COMMAND teste_analise recursao_direta)
add_test(NAME Analise.RecursaoIndireta COMMAND teste_analise recursao_indireta)

add_executable(teste_ll1 LL1Tests.cpp)
configurar_alvo_cpp23(teste_ll1)
target_link_libraries(teste_ll1 PRIVATE compiladores_core)

add_test(NAME LL1.FirstConflito COMMAND teste_ll1 first_conflito)
add_test(NAME LL1.AlternativaValida COMMAND teste_ll1 alternativa_valida)
add_test(NAME LL1.DuasAnulaveis COMMAND teste_ll1 duas_anulaveis)
add_test(NAME LL1.NullableFollow COMMAND teste_ll1 nullable_follow)
add_test(NAME LL1.OpcionalAnulavel COMMAND teste_ll1 opcional_anulavel)
add_test(NAME LL1.RepeticaoAnulavel COMMAND teste_ll1 repeticao_anulavel)
add_test(NAME LL1.OpcionalGuloso COMMAND teste_ll1 opcional_guloso)
add_test(NAME LL1.RepeticaoGulosa COMMAND teste_ll1 repeticao_gulosa)
add_test(NAME LL1.RepeticaoValida COMMAND teste_ll1 repeticao_valida)
add_test(NAME LL1.ElsePendente COMMAND teste_ll1 else_pendente)

# Determinizacao de regex em C++: um caso independente por chamada.
add_executable(teste_regex RegexTests.cpp)
configurar_alvo_cpp23(teste_regex)
target_link_libraries(teste_regex PRIVATE compiladores_core)
foreach(caso digits identifier whitespace group optional negated dot slash empty inverted unclosed unsupported)
    add_test(NAME Regex.${caso} COMMAND teste_regex ${caso})
endforeach()

# Executa o gerador, compila o ANSI C gerado com flags estritas e compara
# a saida com os casos de referencia e cenarios de regressao.
find_program(COMP2_C89_COMPILER NAMES gcc clang cc)
if(COMP2_C89_COMPILER)
    add_test(NAME Gerador.EndToEnd
        COMMAND ${CMAKE_COMMAND}
            "-DGERADOR=$<TARGET_FILE:gerador>"
            "-DC89_COMPILER=${COMP2_C89_COMPILER}"
            "-DROOT=${CMAKE_SOURCE_DIR}"
            "-DOUTPUT=${CMAKE_CURRENT_BINARY_DIR}/gerados"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/GeneratedEndToEnd.cmake")
else()
    message(STATUS "Nao ha compilador de ANSI C disponivel: teste end-to-end desabilitado")
endif()

if(TARGET parser_microc)
    add_test(NAME MicroC.EndToEnd
        COMMAND "${CMAKE_COMMAND}"
            "-DPARSER=$<TARGET_FILE:parser_microc>"
            "-DROOT=${CMAKE_SOURCE_DIR}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/MicroCIntegration.cmake")
endif()

# Reformatacao da AST usada por --debug (uma verificacao por caso).
add_executable(teste_formatacao FormatacaoTests.cpp)
configurar_alvo_cpp23(teste_formatacao)
target_link_libraries(teste_formatacao PRIVATE compiladores_core)
foreach(caso sequencia precedencia vazio)
    add_test(NAME Formatacao.${caso} COMMAND teste_formatacao ${caso})
endforeach()

# Testa o formato de saida real do scanner TP1 (nomes, sem virgulas).
if(TARGET parser_microc)
    add_executable(lexer_comp1_referencia "${CMAKE_SOURCE_DIR}/microc/lex.yy.c")
    set_target_properties(lexer_comp1_referencia PROPERTIES
        C_STANDARD 11 C_STANDARD_REQUIRED YES C_EXTENSIONS YES)
    add_test(NAME MicroC.ArquivoTokens
        COMMAND "${CMAKE_COMMAND}"
            "-DPARSER=$<TARGET_FILE:parser_microc>"
            "-DLEXER=$<TARGET_FILE:lexer_comp1_referencia>"
            "-DROOT=${CMAKE_SOURCE_DIR}"
            "-DOUTPUT=${CMAKE_CURRENT_BINARY_DIR}/tokens-microc"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/MicroCTokens.cmake")
endif()

# Compara mensagens, conjuntos, posicoes e codigos de saida com o TP2.
add_test(NAME Gerador.Referencias
    COMMAND "${CMAKE_COMMAND}"
        "-DGERADOR=$<TARGET_FILE:gerador>"
        "-DROOT=${CMAKE_SOURCE_DIR}"
        "-DOUTPUT=${CMAKE_CURRENT_BINARY_DIR}/referencias"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/ReferenceOutputs.cmake")
