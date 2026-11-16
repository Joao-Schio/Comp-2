# Rodar via CTest: todas as rotas devem ser absolutas.
foreach(param GERADOR C89_COMPILER ROOT OUTPUT)
    if(NOT DEFINED ${param})
        message(FATAL_ERROR "Parametro obrigatorio ausente: ${param}")
    endif()
endforeach()
file(MAKE_DIRECTORY "${OUTPUT}")

function(executar_e_compilar gramatica nome)
    execute_process(
        COMMAND "${GERADOR}" "${gramatica}" -o "${OUTPUT}/${nome}" --tree
        RESULT_VARIABLE codigo
        OUTPUT_VARIABLE stdout
        ERROR_VARIABLE stderr)
    if(NOT codigo STREQUAL "0")
        message(FATAL_ERROR "Falha no gerador ${nome}: ${stdout}${stderr}")
    endif()
    execute_process(
        COMMAND "${C89_COMPILER}" -ansi -pedantic -Wall -Wextra -Werror
            -o "${OUTPUT}/${nome}.bin" "${OUTPUT}/${nome}.c"
        RESULT_VARIABLE codigo
        OUTPUT_VARIABLE stdout
        ERROR_VARIABLE stderr)
    if(NOT codigo STREQUAL "0")
        message(FATAL_ERROR "ANSI C invalido (${nome}): ${stdout}${stderr}")
    endif()
endfunction()

function(verificar_programa nome arquivo esperado)
    execute_process(
        COMMAND "${OUTPUT}/${nome}.bin" "${arquivo}"
        RESULT_VARIABLE codigo
        OUTPUT_VARIABLE stdout
        ERROR_VARIABLE stderr)
    set(obtido "${stdout}${stderr}status ${codigo}\n")
    if(NOT obtido STREQUAL esperado)
        message(FATAL_ERROR "Resultado incorreto para ${arquivo}:\n--- obtido ---\n${obtido}--- esperado ---\n${esperado}")
    endif()
endfunction()

executar_e_compilar("${ROOT}/testes/gramaticas/expr.ebnf" expr)
foreach(nome aceito_1 aceito_2 aceito_3 rejeitado_1 rejeitado_2 rejeitado_3 rejeitado_4)
    set(entrada "${ROOT}/testes/programas/expr/${nome}.txt")
    file(READ "${ROOT}/testes/programas/expr/${nome}.esperado" esperado)
    verificar_programa(expr "${entrada}" "${esperado}")
endforeach()

# Uma alternativa anulavel com FIRST nao vazio deve consumir quando couber.
executar_e_compilar("${ROOT}/tests/fixtures/nullable.ebnf" nullable)
foreach(caso a b vazio)
    if(caso STREQUAL "vazio")
        file(WRITE "${OUTPUT}/nullable-${caso}.txt" "")
    else()
        file(WRITE "${OUTPUT}/nullable-${caso}.txt" "${caso}")
    endif()
    verificar_programa(nullable "${OUTPUT}/nullable-${caso}.txt" "entrada aceita\nstatus 0\n")
endforeach()

# Literal com prioridade sobre ID de mesmo comprimento ('if' versus ID).
executar_e_compilar("${ROOT}/tests/fixtures/literals.ebnf" literals)
file(WRITE "${OUTPUT}/literals.txt" "if alpha-beta gamma\n")
# A entrada deliberadamente contem um ID extra, portanto deve ser rejeitada.
execute_process(COMMAND "${OUTPUT}/literals.bin" "${OUTPUT}/literals.txt"
                RESULT_VARIABLE codigo OUTPUT_VARIABLE stdout ERROR_VARIABLE stderr)
if(NOT codigo STREQUAL "1" OR NOT stderr MATCHES "esperado fim de arquivo")
    message(FATAL_ERROR "Tokenizacao ou verificacao de fim incorreta: ${stdout}${stderr}")
endif()
file(WRITE "${OUTPUT}/literals.txt" "if alpha-beta\n")
verificar_programa(literals "${OUTPUT}/literals.txt" "entrada aceita\nstatus 0\n")

# Arvore de derivacao: somente ao invocar --tree, sem afetar a saida normal.
execute_process(COMMAND "${OUTPUT}/literals.bin" --tree "${OUTPUT}/literals.txt"
                RESULT_VARIABLE codigo OUTPUT_VARIABLE stdout ERROR_VARIABLE stderr)
if(NOT codigo STREQUAL "0" OR NOT stdout MATCHES "inicio" OR
   NOT stdout MATCHES "entrada aceita")
    message(FATAL_ERROR "Arvore nao gerada: ${stdout}${stderr}")
endif()

# A opcao --tokens possui implementacao propria no analisador gerado.
file(WRITE "${OUTPUT}/tokenfile.txt" "Token: tipo = 0, lexema = 'x', linha = 1\n")
execute_process(COMMAND "${OUTPUT}/literals.bin" --tokens "${OUTPUT}/tokenfile.txt"
                RESULT_VARIABLE codigo OUTPUT_VARIABLE stdout ERROR_VARIABLE stderr)
if(NOT codigo STREQUAL "1" OR NOT stderr MATCHES "esperado")
    message(FATAL_ERROR "Leitura de tokens nao rejeitou arquivo sintaticamente invalido: ${stdout}${stderr}")
endif()

# Leitura de um arquivo de tokens valido (tipos conforme o cabecalho gerado).
file(WRITE "${OUTPUT}/tokens-validos.txt"
    "Token: tipo = 2, lexema = 'if', linha = 1\n"
    "Token: tipo = 1, lexema = 'alpha', linha = 1\n"
    "Token: tipo = 3, lexema = '-', linha = 1\n"
    "Token: tipo = 1, lexema = 'beta', linha = 1\n")
execute_process(COMMAND "${OUTPUT}/literals.bin" --tokens "${OUTPUT}/tokens-validos.txt"
                RESULT_VARIABLE codigo OUTPUT_VARIABLE stdout ERROR_VARIABLE stderr)
if(NOT codigo STREQUAL "0" OR NOT stdout STREQUAL "entrada aceita\n")
    message(FATAL_ERROR "Tokens validos nao aceitos: ${stdout}${stderr}")
endif()

# Uma gramatica minima (sem TOKENS nem correspondencias) tambem compila em C89.
file(WRITE "${OUTPUT}/vazia.ebnf" "RULES inicio = ;\n")
executar_e_compilar("${OUTPUT}/vazia.ebnf" vazia)
file(WRITE "${OUTPUT}/vazia.txt" "")
verificar_programa(vazia "${OUTPUT}/vazia.txt" "entrada aceita\nstatus 0\n")

# O --tokens tambem aceita tipos por nome e sem as virgulas do formato antigo.
file(WRITE "${OUTPUT}/tokens-nomes.txt"
    "Token: tipo = T_LIT_2 lexema = 'if' linha = 1\n"
    "Token: tipo = ID lexema = 'alpha' linha = 1\n"
    "Token: tipo = T_LIT_3 lexema = '-' linha = 1\n"
    "Token: tipo = T_N_ID lexema = 'beta' linha = 1\n")
execute_process(COMMAND "${OUTPUT}/literals.bin" --tokens "${OUTPUT}/tokens-nomes.txt"
                RESULT_VARIABLE codigo OUTPUT_VARIABLE stdout ERROR_VARIABLE stderr)
if(NOT codigo STREQUAL "0" OR NOT stdout STREQUAL "entrada aceita\n")
    message(FATAL_ERROR "Tokens com nomes nao aceitos: ${stdout}${stderr}")
endif()

foreach(nome unknown bad_line)
    if(nome STREQUAL "unknown")
        file(WRITE "${OUTPUT}/tokens-${nome}.txt"
            "Token: tipo = T_NAO_EXISTE lexema = 'x' linha = 1\n")
    else()
        file(WRITE "${OUTPUT}/tokens-${nome}.txt"
            "Token: tipo = 1, lexema = 'x', linha = nao_numero\n")
    endif()
    execute_process(COMMAND "${OUTPUT}/literals.bin" --tokens "${OUTPUT}/tokens-${nome}.txt"
                    RESULT_VARIABLE codigo OUTPUT_VARIABLE stdout ERROR_VARIABLE stderr)
    if(NOT codigo STREQUAL "1" OR NOT stderr MATCHES "arquivo de tokens: formato invalido")
        message(FATAL_ERROR "Arquivo malformado ${nome} deveria falhar: ${stdout}${stderr}")
    endif()
endforeach()

# --debug e uma opcao do gerador; nao precisa de -o.
execute_process(COMMAND "${GERADOR}" "${ROOT}/tests/fixtures/nullable.ebnf" --debug
                RESULT_VARIABLE codigo OUTPUT_VARIABLE stdout ERROR_VARIABLE stderr)
if(NOT codigo STREQUAL "0" OR NOT stdout STREQUAL "inicio = [ 'a' ] | 'b' ;\n")
    message(FATAL_ERROR "--debug nao imprimiu a AST como o esperado: ${stdout}${stderr}")
endif()
