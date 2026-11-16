aluno: [Nao vou deixar minha matricula publica, obrigado]
uso_ia: sim

# Compiladores I — Trabalho Prático 2

**Gerador de analisadores sintáticos descendentes recursivos, escrito em C++23 e com saída em ANSI C (C89).**

Este projeto recebe uma gramática em **EBNF**, constrói sua representação sintática, realiza verificações semânticas e de compatibilidade **LL(1)**, calcula os conjuntos **anulável, FIRST e FOLLOW** e gera um analisador sintático descendente recursivo em C. O analisador produzido pode utilizar tanto um scanner gerado a partir da seção `TOKENS` quanto o scanner de **Micro C** desenvolvido no TP1.

> **Sobre a implementação:** com a autorização do professor, o gerador foi reescrito em C++23 em vez de continuar o esqueleto em C. O esqueleto original permanece em [`src-old/`](src-old/) para referência. O antigo README foi preservado **sem alterações** em [`README.OLD.MD`](README.OLD.MD).
>
> As linhas `aluno:` e `uso_ia:` no início deste arquivo preservam a convenção de identificação do trabalho. A utilização de IA está documentada em [`USO_IA.md`](USO_IA.md).

## Requisitos

- **CMake 3.25 ou superior**.
- **Compilador C++23** e biblioteca padrão com suporte aos recursos usados no gerador, incluindo `std::expected` e `std::print`.
- **Compilador C** para o analisador gerado. A saída em C é compilada com o padrão **ANSI C89**; os testes de integração utilizam GCC, Clang ou `cc` quando disponíveis.
- Para a integração com Micro C, um compilador compatível com o código **GNU C11** do Flex original. O arquivo `microc/lex.yy.c` já está no repositório; instalar Flex não é necessário para a compilação normal, apenas para regenerá-lo.

O CMake contém configurações para GCC, Clang e MSVC. A integração do scanner Flex legado depende de recursos GNU/POSIX; para executar o fluxo completo de Micro C, prefira GCC ou Clang em ambiente compatível.

## Compilação e testes

Execute os comandos a partir da **raiz do repositório**.

### Debug

```bash
git clone --recurse-submodules https://github.com/Joao-Schio/Comp-2.git
cd Comp-2

cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Em um clone já existente, inicialize o submódulo com `git submodule update --init --recursive`.

Na configuração Debug, os alvos C++23 usam `-Wall -Wextra -Werror -pedantic` em GCC/Clang. Se `CMAKE_BUILD_TYPE` não for informado em um gerador CMake de configuração única, o projeto usa Debug por padrão.

### Release

```bash
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release --parallel
ctest --test-dir build-release --output-on-failure
```

O Release habilita `-O2` (ou `/O2` no MSVC). Os diretórios `build/` e `build-release/` são independentes. **Não há um alvo específico `make release`**: a configuração de compilação é selecionada pelo CMake. Em geradores de múltiplas configurações, selecione a configuração também com `cmake --build <diretorio> --config Release`.

Após o build, os executáveis principais são:

| Executável | Finalidade |
| --- | --- |
| `build/gerador` | Gerador de analisadores, escrito em C++23 |
| `build/microc/parser_microc` | Analisador Micro C produzido pelo gerador e compilado pelo CMake |

Para não compilar a integração Micro C, configure com `-DCOMP2_BUILD_MICROC=OFF`. O alvo Micro C só é criado quando a opção está habilitada e `microc/lex.yy.c` está disponível.

## Como usar o gerador

### 1. Gerar um analisador a partir de EBNF

O exemplo abaixo utiliza a gramática de expressões aritméticas fornecida para o trabalho:

```bash
./build/gerador testes/gramaticas/expr.ebnf -o build/exemplos/expr --sets

cc -ansi -pedantic -Wall -Wextra -Werror \
    build/exemplos/expr.c -o build/exemplos/expr

./build/exemplos/expr testes/programas/expr/aceito_1.txt
```

O argumento `-o` é um **prefixo**: são produzidos `<prefixo>.c` e `<prefixo>.h`. O código gerado contém o parser, o scanner próprio quando necessário e o programa principal. Uma entrada válida produz `entrada aceita`; entradas inválidas geram diagnósticos e um código de saída diferente de zero.

### 2. Analisar ou inspecionar a gramática sem gerar arquivos

```bash
./build/gerador --parse-only testes/gramaticas/expr.ebnf
./build/gerador --check testes/gramaticas/expr.ebnf
./build/gerador --sets testes/gramaticas/expr.ebnf
./build/gerador testes/gramaticas/expr.ebnf --debug
```

| Opção do gerador | Comportamento |
| --- | --- |
| `-o prefixo` | Gera `prefixo.c` e `prefixo.h` após as verificações |
| `--parse-only` | Analisa a sintaxe EBNF e informa a quantidade de tokens, skips e regras |
| `--check` | Executa a análise semântica e a verificação LL(1) sem gerar arquivos |
| `--sets` | Imprime anulabilidade e conjuntos FIRST/FOLLOW |
| `--debug` | Imprime as regras EBNF reconstruídas a partir da AST |
| `--tree` | Inclui no parser gerado o suporte à impressão da árvore de derivação |
| `--scanner microc` | Usa o scanner externo de Micro C; requer `TOKENS EXTERN` na gramática |

As opções `--sets` e `--debug` também podem acompanhar `-o`. O gerador não emite código quando encontra erros de validação que impedem a geração.

### 3. Executar o parser gerado com a árvore de derivação

```bash
./build/gerador testes/gramaticas/expr.ebnf \
    -o build/exemplos/expr-arvore --tree

cc -ansi -pedantic -Wall -Wextra -Werror \
    build/exemplos/expr-arvore.c -o build/exemplos/expr-arvore

./build/exemplos/expr-arvore --tree testes/programas/expr/aceito_1.txt
```

`--tree` no **gerador** habilita esse recurso no código produzido; `--tree` no **parser gerado** solicita a impressão durante a execução.

## Integração com o scanner de Micro C (TP1)

O submódulo [`external/comp1-2026/`](external/comp1-2026/) aponta para o repositório original do TP1 e permanece independente. Como o Flex original define seu próprio `main()` e sua enumeração de tokens, o TP2 usa um adaptador separado em `microc/scanner_comp1.c` e uma interface em `microc/tokens.h`. O `microc/lex.yy.c` fornecido pelo aluno é compilado separadamente do parser C89.

O build padrão executa automaticamente estas etapas: gera o parser a partir de `testes/gramaticas/microc.ebnf` com `--scanner microc`, compila o parser em ANSI C e o vincula ao scanner original.

```bash
cmake --build build --parallel
./build/microc/parser_microc testes/programas/microc/soma.mc
./build/microc/parser_microc testes/programas/microc/bubble.mc
```

Também é possível consumir um arquivo de tokens previamente produzido pelo executável original do TP1:

```bash
./build/tests/lexer_comp1_referencia \
    testes/programas/microc/soma.mc > build/soma.tokens

./build/microc/parser_microc --tokens build/soma.tokens
```

O parser gerado aceita arquivos `--tokens` com tipos **numéricos** (formato do esqueleto do TP2) ou **nomes de tokens** (formato textual do TP1), com mensagens para registros malformados. Esse formato de texto, orientado a linhas, não representa sem ambiguidades lexemas com quebras de linha reais.

Para detalhes sobre a ABI do TP1, os comandos de compilação manual e as limitações do Flex, consulte [`microc/INTEGRACAO.md`](microc/INTEGRACAO.md).

## Estrutura do repositório

```text
Comp-2/
├── CMakeLists.txt          # Build Debug/Release, alvos e CTest
├── src/
│   ├── main.cpp            # CLI e orquestração das etapas
│   ├── common/             # Posições e localização na entrada
│   ├── ebnf/               # Lexer, parser, AST e formatação EBNF
│   ├── analysis/           # Semântica, anulável, FIRST/FOLLOW e LL(1)
│   └── codegen/            # Emissão de C89, regex e leitura de tokens
├── microc/                 # Scanner Flex do TP1 e adaptador para C89
├── external/comp1-2026/    # Submódulo Git do trabalho original
├── tests/                  # Testes em C++23 e integração com CTest
├── testes/                 # Gramáticas, entradas e saídas de referência
├── src-old/                # Esqueleto original em C do professor
├── Trabalho-2.pdf          # Enunciado do trabalho
├── USO_IA.md               # Registro do uso de IA
└── README.OLD.MD           # README original, preservado integralmente
```

## Decisões de projeto

O gerador adota um estilo de **C++23 inspirado em princípios de Rust**, sem tentar reproduzir a linguagem Rust:

- **Tipos de domínio explícitos:** a AST é modelada com `std::variant` e tipos distintos para alternativas, sequências, opcionais, repetições, terminais e não terminais.
- **Erros como valores:** falhas esperadas são propagadas com `std::expected` e diagnósticos estruturados, evitando exceções como mecanismo normal de controle.
- **Ownership e ciclo de vida:** gerenciamento de recursos via RAII, `std::unique_ptr` e movimentação explícita quando apropriada.
- **Separação de responsabilidades:** parsing da EBNF, análise semântica, cálculo de conjuntos, verificações LL(1) e geração de código ficam em módulos separados.
- **Compatibilidade na saída:** os recursos de C++23 são usados **no gerador**, enquanto o analisador produzido continua sendo **ANSI C89**.

A justificativa arquitetural e os critérios de avaliação da reescrita estão detalhados em [`src/README.md`](src/README.md).

## Testes e critérios de entrega

```bash
ctest --test-dir build --output-on-failure
ctest --test-dir build -N
```

O CTest registra **55 cenários na configuração com as ferramentas e a integração Micro C disponíveis**, cobrindo lexer, parser EBNF, análise semântica, LL(1), regex, formatação, geração C89, Micro C e comparação de saídas com os arquivos de referência. Alguns testes dependem da disponibilidade de compilador C e dos alvos do scanner original.

Há **dois conjuntos de testes distintos**:

- [`tests/`](tests/README.md): testes automatizados do novo código C++23, incluindo cenários de ponta a ponta executados pelo CTest.
- [`testes/`](testes/): material de referência do trabalho, com gramáticas, programas aceitos/rejeitados e saídas esperadas do professor.

**Atenção à entrega:** os casos automatizados existentes não substituem a exigência descrita no enunciado de preparar **pelo menos cinco gramáticas** e **três programas aceitos e três rejeitados por gramática válida**. Esse conjunto adicional de artefatos de avaliação deve ser conferido antes da submissão final.

## Documentação e atribuições

- [`src/README.md`](src/README.md) — fundamentação da arquitetura em C++23 e decisões de implementação.
- [`tests/README.md`](tests/README.md) — organização e execução dos testes.
- [`microc/INTEGRACAO.md`](microc/INTEGRACAO.md) — integração entre o scanner do TP1 e o parser C89.
- [`USO_IA.md`](USO_IA.md) — histórico e atribuição do uso de IA generativa.
- [`README.OLD.MD`](README.OLD.MD) — texto original do README anterior à atualização, preservado como referência.

Este README atualizado foi redigido com assistência do ChatGPT (GPT-6), a pedido do aluno. Nenhuma alteração no comportamento do gerador ou do analisador é necessária para esta atualização de documentação.
