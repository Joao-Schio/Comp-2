## USO_IA.md

**Autoria deste registro:** o conteúdo textual deste `USO_IA.md` é redigido e organizado pelo ChatGPT (GPT-6), por solicitação do aluno, que define os objetivos, revisa as propostas e decide sobre a incorporação das mudanças. A atribuição da redação deste documento à IA não implica atribuir à IA todo o código ou todas as decisões do projeto.

**Autoria dos testes em `tests/`:** os arquivos de testes e a infraestrutura CTest desse diretório, incluindo os testes de lexer, parser, análise, LL(1), regex, geração de ANSI C, Micro C e formatação, foram produzidos pelo ChatGPT (GPT-6) a pedido do aluno. Isso não inclui os materiais de referência fornecidos no diretório `testes/`.


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

Ferramenta:
    ChatGPT (GPT-6)
    Trecho: CMakeLists.txt, tests/CMakeLists.txt, tests/LexerTests.cpp e USO_IA.md
    Finalidade: Reorganizar a configuracao CMake para compilar o nucleo em C++23 como uma biblioteca estatica compartilhada entre o executavel gerador e testes, integrar CTest sem bibliotecas externas e manter as flags de Debug/Release para GCC, Clang e MSVC. Incluir testes basicos do lexer, cobrindo regex com escape, literal, comentario, palavra reservada, posicao e erro de regex vazia.
    O que fiz: Configuracao CMake e testes iniciais gerados pela IA e propostos em pull request para revisao antes da integracao. O codigo de producao do lexer nao foi modificado. A execucao dos testes deve ser verificada no ambiente do projeto.

Ferramenta:
    ChatGPT (GPT-6)
    Trecho: tests/CMakeLists.txt, tests/LexerTests.cpp e USO_IA.md
    Finalidade: Fazer o CTest registrar e reportar separadamente os quatro cenarios de teste do lexer, mantendo um unico executavel de teste e sem adicionar dependencias externas.
    O que fiz: A IA adaptou o main de LexerTests.cpp para aceitar o nome do caso por argumento e registrou as quatro invocacoes individuais no CMake. As alteracoes foram propostas em pull request para revisao; o codigo de producao nao foi modificado.

Ferramenta:
    ChatGPT (GPT-6)
    Trecho: tests/README.md e USO_IA.md
    Finalidade: Documentar os testes C++23 integrados ao CTest, explicar como executá-los, distinguir tests/ dos testes de referência em testes/ e declarar explicitamente a autoria integral da documentação e dos testes produzidos pela IA.
    O que fiz: Redigi integralmente tests/README.md, acrescentei as declarações de autoria a USO_IA.md e preservei os registros históricos existentes. Alterações de documentação propostas em pull request para revisão do aluno; não alterei os testes nem o código de produção.

Ferramenta:
    ChatGPT (GPT-6)
    Trecho: src/analysis/Analise.hpp, src/analysis/Analise.cpp, src/analysis/LL1.cpp e tests/AnaliseTests.cpp, tests/LL1Tests.cpp
    Finalidade: Implementar validacao semantica, anulabilidade, FIRST/FOLLOW, recursao a esquerda e verificacao LL(1) com testes automatizados.
    O que fiz: Gerou implementacao e testes em patch, revisados pelo aluno antes de integrar ao repositorio.

Ferramenta:
    ChatGPT (GPT-6)
    Trecho: src/codegen/, src/main.cpp, tests/GeneratedEndToEnd.cmake e tests/RegexTests.cpp
    Finalidade: Gerar parsers descendentes recursivos ANSI C89, compilar expressoes regulares em automatos, integrar a CLI e testar programas gerados.
    O que fiz: Produziu codigo e testes para revisao local; o aluno decidiu sobre a integracao.

Ferramenta:
    ChatGPT (GPT-6)
    Trecho: microc/scanner_comp1.c, microc/tokens.h, microc/INTEGRACAO.md, CMakeLists.txt e tests/MicroCIntegration.cmake
    Finalidade: Integrar o scanner Flex do TP1 ao parser gerado, sem modificar o submodulo externo, e automatizar os testes de Micro C.
    O que fiz: Criou o adaptador, configuracao de compilacao, documentacao e testes. O arquivo microc/lex.yy.c foi fornecido pelo aluno e nao foi redigido pela IA.

Ferramenta:
    ChatGPT (GPT-6)
    Trecho: src/ebnf/Formatacao.hpp, src/ebnf/Formatacao.cpp, src/codegen/LeitorTokens.hpp, src/codegen/Gerador.cpp, src/main.cpp, tests/ e microc/INTEGRACAO.md
    Finalidade: Implementar a opcao --debug segundo o esqueleto C e tornar a leitura --tokens compativel tanto com o formato numerico do TP2 quanto com os nomes impressos pelo scanner do TP1, incluindo validacao de entradas malformadas, ordenacao compativel de diagnosticos/FIRST/FOLLOW e testes comparados aos arquivos de referencia do professor.
    O que fiz: Implementacao, testes e documentacao produzidos pelo ChatGPT para revisao e aplicacao pelo aluno, sem commit ou pull request no repositorio remoto.

Ferramenta:
    ChatGPT (GPT-6)
    Trecho: README.md, README.OLD.MD e USO_IA.md
    Finalidade: Substituir o README principal desatualizado por uma documentação fiel à implementação atual em C++23, aos comandos CMake Debug/Release, à geração ANSI C89, ao scanner Micro C, ao submódulo TP1 e aos testes automatizados; preservar integralmente o README anterior em README.OLD.MD.
    O que fiz: Copiei sem nenhuma alteração o README.md anterior para README.OLD.MD, redigi integralmente o novo README.md e acrescentei este registro de uso de IA. As alterações são exclusivamente documentais e foram propostas em pull request para revisão pelo aluno.
