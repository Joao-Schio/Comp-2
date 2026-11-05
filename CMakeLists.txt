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

# Nucleo compartilhado entre o executavel e os testes; adicionar futuros .cpp
# do parser e das analises aqui, sem incluir main.cpp na biblioteca.
add_library(compiladores_core STATIC
    src/ebnf/Lexer.cpp
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
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
