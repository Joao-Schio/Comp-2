## USO_IA.md

Registre aqui cada uso relevante de ferramentas de IA generativa, como no TP1. Se nenhuma foi usada, deixe apenas a frase:

Nenhuma ferramenta de IA foi utilizada no desenvolvimento deste trabalho.

Modelo de entrada:

    Ferramenta: <ferramenta e modelo>
    Trecho:     <arquivo ou parte do trabalho>
    Finalidade: <o que foi pedido, de forma especifica>
    O que fiz:  <usado integralmente, adaptado ou apenas consultado>

Ferramenta:
    ChatGPT
    Trecho: CMakeLists.txt
    Finalidade: Makefile's não são portáveis para todos os compiladores de C/C++ portanto pedi para o ChatGPT gerar um CMakeList.txt para que, independentemente do compilador usado pelo usuário final, dentre os três mais usados (GCC, CLANG e MSVC) o trabalho ainda assim possa ser compilado.
    O que fiz: Dado o meu repositório no github (https://github.com/Joao-Schio/Comp-2) Faça um CMakelist que inclua os compiladores GCC, CLANG e MSVC, utilizando as flags -Wall -Werror -Wextra -pedantic para compilar em modo debug e para modo release coloque a flag -O2, certifique-se que o CMakeLists.txt final seja compatível com todos os compiladores ditos acima.


Ferramenta:
    ChatGPT (GPT-6)
    Trecho: .gitignore
    Finalidade: Solicitei a geração de um arquivo .gitignore para a nova estrutura do projeto em C++23, utilizando CMake e visando compatibilidade com GCC, Clang e MSVC. O objetivo é ignorar diretórios de compilação, binários, arquivos temporários e configurações locais, sem excluir código-fonte, testes ou arquivos necessários à entrega.
    O que fiz: [Preencher após revisar e aplicar: utilizado integralmente, adaptado ou apenas consultado.]

Ferramenta:
    ChatGPT (GPT-6)
    Trecho: CMakeLists.txt
    Finalidade: Atualizar a configuração CMake para incluir src/ebnf/Lexer.cpp na compilação do executável gerador e permitir os caminhos de include já utilizados pelo código C++23, sem alterar a implementação do lexer.
    O que fiz: Modificações geradas pela IA e propostas em pull request para revisão antes da integração. A entrada anterior sobre a criação original do CMakeLists.txt foi preservada.

Ferramenta:
    ChatGPT (GPT-6)
    Trecho: src/README.md e USO_IA.md
    Finalidade: Revisar e ampliar a documentação da nova implementação em C++23 para facilitar a leitura e a correção do trabalho pelo professor ou por ferramentas de IA. Explicar a inspiração em Rust (erros como valores com std::expected, ownership, std::move, std::variant e prevenção de comportamento indefinido), a separação entre gerador C++23 e parser ANSI C e os critérios para avaliar a reescrita sem exigir correspondência direta com o esqueleto em C.
    O que fiz: Texto de documentação redigido pela IA e proposto em pull request para revisão. Nenhuma alteração no código-fonte ou no comportamento do programa.
