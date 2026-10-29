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