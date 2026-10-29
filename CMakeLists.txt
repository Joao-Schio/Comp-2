cmake_minimum_required(VERSION 3.25)

project(Comp2 LANGUAGES CXX)

# Em geradores de configuracao unica (Ninja, Unix Makefiles), Debug e o padrao.
# Visual Studio e outros geradores multi-config escolhem a configuracao no build.
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Configuracao da compilacao" FORCE)
endif()

add_executable(gerador
    src/main.cpp
)

# Conforme os modulos forem implementados, adicionar os respectivos .cpp
# explicitamente na lista de fontes acima.
set_target_properties(gerador PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

target_include_directories(gerador PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

# GCC/Clang usam flags no estilo Unix; MSVC (e clang-cl) usa /W4 e /WX.
# As flags sao especificas por configuracao, inclusive em geradores multi-config.
if(MSVC)
    target_compile_options(gerador PRIVATE
        "$<$<CONFIG:Debug>:/W4>"
        "$<$<CONFIG:Debug>:/WX>"
        "$<$<CONFIG:Debug>:/permissive->"
        "$<$<CONFIG:Release>:/O2>"
    )
else()
    target_compile_options(gerador PRIVATE
        "$<$<CONFIG:Debug>:-Wall>"
        "$<$<CONFIG:Debug>:-Wextra>"
        "$<$<CONFIG:Debug>:-Werror>"
        "$<$<CONFIG:Debug>:-pedantic>"
        "$<$<CONFIG:Release>:-O2>"
    )
endif()

# Prepara o projeto para testes com CTest sem tentar executar os testes
# originais do gerador enquanto a reimplementacao nao estiver funcional.
include(CTest)
if(BUILD_TESTING AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
    add_subdirectory(tests)
endif()