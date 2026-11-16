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
