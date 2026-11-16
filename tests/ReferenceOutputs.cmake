# Confere a saida integral de todos os casos de referencia do professor.
foreach(param GERADOR ROOT OUTPUT)
    if(NOT DEFINED ${param})
        message(FATAL_ERROR "Parametro ausente: ${param}")
    endif()
endforeach()
file(MAKE_DIRECTORY "${OUTPUT}")

foreach(caso IN ITEMS expr recursao_esquerda conflito_ll1 microc)
    set(gramatica "testes/gramaticas/${caso}.ebnf")
    set(opcoes)
    if(caso STREQUAL "microc")
        list(APPEND opcoes --scanner microc)
    endif()
    execute_process(
        COMMAND "${GERADOR}" "${gramatica}" -o "${OUTPUT}/${caso}"
            --sets ${opcoes}
        WORKING_DIRECTORY "${ROOT}"
        RESULT_VARIABLE status
        OUTPUT_VARIABLE saida
        ERROR_VARIABLE erros)
    string(CONCAT obtido "${saida}" "${erros}" "status ${status}\n")
    file(READ "${ROOT}/testes/esperado/${caso}.gerador" esperado)
    if(NOT obtido STREQUAL esperado)
        message(FATAL_ERROR "Referencia ${caso} divergiu\nObtido:\n${obtido}\nEsperado:\n${esperado}")
    endif()
endforeach()
