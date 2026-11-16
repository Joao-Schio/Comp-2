cmake_minimum_required(VERSION 3.25)

project(Comp2 LANGUAGES CXX)

# Em geradores de configuracao unica (Ninja, Unix Makefiles), Debug e o padrao.
# Geradores multi-config escolhem a configuracao no momento do build.
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Configuracao da compilacao" FORCE)
endif()

# Aplica as mesmas regras de compilacao ao gerador, a biblioteca e aos testes.
function(configurar_alvo_cpp23 alvo)
    set_target_properties(${alvo} PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
    )

    if(MSVC)
        target_compile_options(${alvo} PRIVATE
            "$<$<CONFIG:Debug>:/W4>"
            "$<$<CONFIG:Debug>:/WX>"
            "$<$<CONFIG:Debug>:/permissive->"
            "$<$<CONFIG:Release>:/O2>"
        )
    else()
        target_compile_options(${alvo} PRIVATE
            "$<$<CONFIG:Debug>:-Wall>"
            "$<$<CONFIG:Debug>:-Wextra>"
            "$<$<CONFIG:Debug>:-Werror>"
            "$<$<CONFIG:Debug>:-pedantic>"
            "$<$<CONFIG:Release>:-O2>"
        )
    endif()
endfunction()

# Nucleo compartilhado entre o executavel e os testes.
add_library(compiladores_core STATIC
    src/ebnf/Lexer.cpp
    src/ebnf/Parser.cpp
    src/ebnf/Formatacao.cpp
    src/analysis/Analise.cpp
    src/analysis/LL1.cpp
    src/codegen/Regex.cpp
    src/codegen/Gerador.cpp
)
configurar_alvo_cpp23(compiladores_core)

# Os includes existentes usam "src/ebnf/..." e "common/...".
# PUBLIC propaga os caminhos para os executaveis que ligam a biblioteca.
target_include_directories(compiladores_core PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

add_executable(gerador src/main.cpp)
configurar_alvo_cpp23(gerador)
target_link_libraries(gerador PRIVATE compiladores_core)

include(CTest)

# Integracao do scanner original do TP1; a copia de lex.yy.c e fornecida
# separadamente do submodulo, cujo .gitignore nao registra arquivos gerados.
# O parser gerado continua sendo C89, enquanto o Flex legado usa C GNU11.
option(COMP2_BUILD_MICROC "Compilar scanner do TP1 e parser Micro C" ON)
if(COMP2_BUILD_MICROC AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/microc/lex.yy.c")
    enable_language(C)
    add_library(microc_scanner STATIC
        microc/lex.yy.c
        microc/scanner_comp1.c
    )
    set_target_properties(microc_scanner PROPERTIES C_STANDARD 11 C_EXTENSIONS ON)
    set_source_files_properties(microc/lex.yy.c PROPERTIES
        COMPILE_DEFINITIONS "main=comp1_lexer_main")

    set(MICROC_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/microc/parser_microc")
    add_custom_command(
        OUTPUT "${MICROC_PREFIX}.c" "${MICROC_PREFIX}.h"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/microc"
        COMMAND "$<TARGET_FILE:gerador>"
            "${CMAKE_CURRENT_SOURCE_DIR}/testes/gramaticas/microc.ebnf"
            -o "${MICROC_PREFIX}" --scanner microc
        DEPENDS gerador "${CMAKE_CURRENT_SOURCE_DIR}/testes/gramaticas/microc.ebnf"
        VERBATIM
    )
    add_executable(parser_microc "${MICROC_PREFIX}.c")
    target_include_directories(parser_microc PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/microc")
    target_link_libraries(parser_microc PRIVATE microc_scanner)
    set_target_properties(parser_microc PROPERTIES
        C_STANDARD 90 C_STANDARD_REQUIRED YES C_EXTENSIONS NO
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/microc"
    )
    if(NOT MSVC)
        target_compile_options(parser_microc PRIVATE -ansi -pedantic -Wall -Wextra -Werror)
    endif()
elseif(COMP2_BUILD_MICROC)
    message(STATUS "microc/lex.yy.c ausente: parser Micro C nao sera compilado")
endif()

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
