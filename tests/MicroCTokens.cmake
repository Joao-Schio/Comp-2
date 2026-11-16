# Reproduz a saida textual do executavel TP1 e alimenta o parser gerado.
foreach(param PARSER LEXER ROOT OUTPUT)
    if(NOT DEFINED ${param})
        message(FATAL_ERROR "Parametro ausente: ${param}")
    endif()
endforeach()
file(MAKE_DIRECTORY "${OUTPUT}")

foreach(caso soma bubble else_pendente)
    set(entrada "${ROOT}/testes/programas/microc/${caso}.mc")
    set(tokens "${OUTPUT}/${caso}.tokens")
    execute_process(
        COMMAND "${LEXER}" "${entrada}"
        RESULT_VARIABLE codigo
        OUTPUT_FILE "${tokens}"
        ERROR_VARIABLE erro)
    if(NOT codigo STREQUAL "0" OR NOT erro STREQUAL "")
        message(FATAL_ERROR "Lexer TP1 falhou para ${caso}: ${erro}")
    endif()
    execute_process(
        COMMAND "${PARSER}" --tokens "${tokens}"
        RESULT_VARIABLE codigo
        OUTPUT_VARIABLE saida
        ERROR_VARIABLE erro)
    if(NOT codigo STREQUAL "0" OR NOT saida STREQUAL "entrada aceita\n" OR
       NOT erro STREQUAL "")
        message(FATAL_ERROR "Parser nao aceitou tokens TP1 de ${caso}: ${saida}${erro}")
    endif()
endforeach()

# O leitor de arquivos deve tratar tambem erros lexicos ja codificados.
file(WRITE "${OUTPUT}/undef.tokens"
    "Token: tipo = UNDEF lexema = 'String nao terminada' linha = 7\n")
execute_process(
    COMMAND "${PARSER}" --tokens "${OUTPUT}/undef.tokens"
    RESULT_VARIABLE codigo
    OUTPUT_VARIABLE saida
    ERROR_VARIABLE erro)
if(NOT codigo STREQUAL "1" OR NOT erro STREQUAL
    "linha 7: erro lexico: String nao terminada\n")
    message(FATAL_ERROR "UNDEF via --tokens nao tratado corretamente: ${saida}${erro}")
endif()
