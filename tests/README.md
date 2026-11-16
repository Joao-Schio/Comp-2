# Testes automatizados em C++23

Este diretório contém os testes da **nova implementação do gerador**, escritos em C++23 e registrados no CTest. A finalidade é verificar cada etapa isoladamente, com falhas reproduzíveis e sem bibliotecas externas de teste.

> **Autoria e uso de IA:** o conteúdo dos arquivos deste diretório (`CMakeLists.txt`, `LexerTests.cpp`, `ParserTests.cpp`, `AnaliseTests.cpp`, `LL1Tests.cpp` e este `README.md`) foi produzido pelo ChatGPT (GPT-6) a pedido do aluno, que revisa e decide sobre a integração das alterações. Consulte [`USO_IA.md`](../USO_IA.md) para o histórico. Esta declaração **não** se estende ao diretório `testes/`, que contém os casos e resultados de referência do trabalho.

## Compilar e executar

A partir da raiz do repositório:

```sh
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

Para listar os casos registrados:

```sh
ctest --test-dir build -N
```

Para executar somente uma etapa, utilize o filtro `-R`:

```sh
ctest --test-dir build -R '^Lexer\.' --output-on-failure
ctest --test-dir build -R '^Parser\.' --output-on-failure
ctest --test-dir build -R '^Analise\.' --output-on-failure
ctest --test-dir build -R '^LL1\.' --output-on-failure
```

Os testes são compilados com as opções de C++23 do projeto e, na configuração Debug para GCC/Clang, com `-Wall -Wextra -Werror -pedantic`. O compilador e a biblioteca padrão precisam suportar os recursos de C++23 utilizados pelo código.

## Organização

| Arquivo | Executável | Casos CTest | Responsabilidade |
| --- | --- | ---: | --- |
| `LexerTests.cpp` | `teste_lexer` | 4 | Reconhecimento de tokens, escapes, comentários e erros léxicos |
| `ParserTests.cpp` | `teste_parser` | 8 | Construção da árvore EBNF, agrupamentos, alternativas vazias, tokens externos e erros sintáticos |
| `AnaliseTests.cpp` | `teste_analise` | 14 | Verificações semânticas, anulabilidade, FIRST/FOLLOW, referências, alcançabilidade e recursão à esquerda |
| `LL1Tests.cpp` | `teste_ll1` | 10 | Conflitos LL(1), alternativas anuláveis e avisos de escolha gulosa |
| `CMakeLists.txt` | — | **36 no total** | Compila os quatro executáveis e registra cada caso separadamente no CTest |

Cada executável recebe **um identificador de caso** como argumento. Por exemplo, depois da compilação:

```sh
./build/tests/teste_analise expr
./build/tests/teste_ll1 first_conflito
```

O CTest normalmente é preferível porque executa e identifica cada cenário de forma independente. Um teste devolve código de saída zero quando passa e código diferente de zero quando falha.

## Diferença entre `tests/` e `testes/`

- **`tests/` (este diretório):** testes unitários e de componentes da implementação em C++23, integrados ao CTest.
- **`testes/` (na raiz):** gramáticas, programas e arquivos de saída esperada fornecidos para validar o **comportamento completo** do gerador e dos parsers produzidos. O script `testes/executar.sh` compara saídas textuais, diagnósticos e códigos de saída. Ele **não é executado automaticamente** pelos casos CTest aqui definidos.

Por enquanto, os testes deste diretório cobrem lexer, parser EBNF, análise semântica, conjuntos e verificação LL(1). Eles não comprovam, por si só, a geração e execução de um analisador ANSI C. Essa parte deve ser validada com os casos de integração quando a geração estiver implementada.

## Como acrescentar um caso

1. Adicione uma função de teste no arquivo `*Tests.cpp` da etapa correspondente; a função deve indicar sucesso ou falha e conferir o comportamento observável.
2. Registre um identificador novo no despachante do executável (o `main` do arquivo).
3. Acrescente uma chamada `add_test(NAME ... COMMAND ...)` em `tests/CMakeLists.txt`, passando o identificador novo.
4. Recompile e execute `ctest --test-dir build --output-on-failure`.

Prefira casos pequenos que isolem uma propriedade e incluam entradas inválidas além das válidas. Para diagnósticos relevantes, confira também posição, gravidade e mensagem. Para tabelas de FIRST/FOLLOW, compare a formatação quando ela faz parte do contrato do trabalho.

A documentação deste diretório deve ser atualizada quando novos módulos e casos forem adicionados.
