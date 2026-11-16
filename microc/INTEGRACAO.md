# Scanner original do TP1 no Comp-2

O submodulo `external/comp1-2026/` permanece inalterado. A especificacao Flex do
TP1 define `TokenType`, `microc_yylval`, `linha_atual`, `yylex()` e um `main()`
proprio. Esses simbolos nao sao diretamente compativeis com a interface de
`microc/scanner.h` fornecida no esqueleto do TP2.

## Arquivos de integracao

- `lex.yy.c`: **copia exata** do artefato Flex do TP1 fornecido pelo aluno.
  O submodulo nao o rastreia; caso o TP1 mude, gere uma nova copia com `flex`
  a partir do `microc.flex` e substitua este arquivo.
- `tokens.h`: enum de tokens com a mesma ordem do TP1 e o tipo `Token` para
  o parser ANSI C.
- `scanner_comp1.c`: adaptador que chama `yylex`, transforma `microc_yylval`
  em `Token`, transfere erros lexicos e libera os lexemas alocados.
- `scanner.c`: esqueleto original do professor, preservado como referencia,
  **nao** incluido no target `microc_scanner`.

O codigo Flex declara um `main()` destinado apenas ao teste isolado do TP1.
Compilamos somente `lex.yy.c` com `-Dmain=comp1_lexer_main`, impedindo colisao
com o `main()` do parser gerado. O lexer legado e compilado em C GNU11 porque
usa `strdup()` e infraestrutura Flex; **somente o parser gerado** precisa ser
compativel com ANSI C89. Nao se deve compilar o `lex.yy.c` original com `-ansi
-Werror`.

## Gerar, compilar e executar

A partir da raiz de Comp-2, com um compilador C e C++23 compativel:

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
./build/microc/parser_microc testes/programas/microc/soma.mc
```

O CMake compila o gerador C++23, gera `build/microc/parser_microc.c/.h` usando
`--scanner microc`, compila o parser em ANSI C89 e o vincula ao adaptador e
scanner do TP1. Para gerar um parser manualmente:

```sh
./build/gerador testes/gramaticas/microc.ebnf -o build/microc/manual --scanner microc --tree
gcc -ansi -pedantic -Wall -Wextra -Werror -I microc -c build/microc/manual.c -o build/microc/manual.o
gcc -std=gnu11 -Dmain=comp1_lexer_main -c microc/lex.yy.c -o build/microc/lex.o
gcc -std=gnu11 -c microc/scanner_comp1.c -o build/microc/adapter.o
gcc build/microc/manual.o build/microc/adapter.o build/microc/lex.o -o build/microc/manual
```

As definicoes `TOKENS EXTERN` e `--scanner microc` devem ser usadas juntas.
O lexer permanece dependente dos tokens e das regras do TP1 original; a
semantica desses tokens nao e alterada pelo gerador.

## Fluxo de tokens do TP1

Para conferir a leitura `--tokens` com a saida do lexer original, sem passar
pelo adaptador, o target de testes compila tambem o executavel TP1 de
referencia:

```sh
./build/tests/lexer_comp1_referencia testes/programas/microc/soma.mc > build/soma.tokens
./build/microc/parser_microc --tokens build/soma.tokens
```

O arquivo do TP1 usa nomes de `TokenType` e separa os campos com espacos;
o modo de compatibilidade tambem preserva a leitura de linhas com IDs
numericos separados por virgulas, como no esqueleto do TP2.
Esse formato textual nao representa sem ambiguidade lexemas contendo
quebras de linha reais.
