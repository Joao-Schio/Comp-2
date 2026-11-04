# `src/` — Nova implementação em C++23

> **Uma decisão ousada, mas deliberada:** em vez de converter gradualmente o código-base fornecido em C, vamos desenvolver uma nova implementação do gerador em C++23, mantendo o esqueleto original como referência.

## Por que começar de novo?

O código fornecido pelo professor é o ponto de partida conceitual deste trabalho. Ele descreve uma organização funcional para um gerador de analisadores sintáticos descendentes recursivos: leitura de gramáticas EBNF, verificações semânticas, cálculo de conjuntos, análise LL(1) e geração de código.

No entanto, adaptar cada estrutura e função diretamente de C para C++ nos levaria a preservar decisões arquiteturais tomadas para outro modelo de linguagem: alocação e liberação manuais, estruturas com campos de validade condicional, estado global e interfaces pouco expressivas.

A escolha de recomeçar é **ousada porque aumenta o trabalho inicial**. Em contrapartida, permite construir uma base coerente desde o primeiro módulo, sem misturar dois estilos de programação nem acumular camadas de compatibilidade dentro da implementação principal.

O objetivo não é descartar o conhecimento incorporado ao esqueleto, mas **preservar seus requisitos e repensar sua implementação**. A consistência do projeto, a clareza do fluxo de dados e a facilidade de manutenção justificam esse investimento.

## Organização e limites

- **`src/`**: implementação nova do gerador, em C++23.
- **`src-old/`**: destino planejado para uma cópia preservada do esqueleto original em C, usado como referência, sem refatorações.
- **`microc/`**: integração com o analisador léxico de Micro C do primeiro trabalho, por meio de uma interface compatível.
- **`testes/`**: casos e resultados esperados fornecidos para verificar a compatibilidade comportamental. Testes adicionais poderão ser organizados separadamente.

A reorganização dessas pastas faz parte da migração e ainda não deve ser interpretada como concluída.

O projeto [comp1-2026](https://github.com/Joao-Schio/comp1-2026) **não será modernizado nem modificado** por esta reescrita. Qualquer adaptação necessária para integrá-lo deverá acontecer neste repositório.

**O gerador será escrito em C++23, mas o analisador sintático por ele produzido continuará sendo código ANSI C**, respeitando o contrato do trabalho e a integração com o scanner existente.

## Princípios arquiteturais

### 1. C++ moderno, não C com classes

Usaremos a biblioteca padrão e os recursos de C++23 onde trouxerem benefícios reais: `std::variant`, `std::expected`, `std::optional`, `std::unique_ptr`, `std::vector`, `std::string`, `std::string_view` e outros, conforme a necessidade.

A inspiração em Rust está na disciplina de ownership, na explicitação de falhas e na modelagem de estados válidos — **não na tentativa de reproduzir Rust sintaticamente em C++**.

### 2. Ownership explícito e RAII

Recursos devem ter responsáveis claros e ser liberados automaticamente. Preferimos valores e referências para acesso não proprietário; ponteiros proprietários serão representados por tipos de ownership, quando necessários. `std::string_view` e referências exigem atenção explícita ao tempo de vida dos objetos referenciados.

Não adotaremos a regra artificial de tornar todos os tipos não copiáveis: cópias simples e semanticamente corretas continuam sendo válidas.

### 3. Erros como parte das interfaces

Falhas esperadas, como entradas inválidas, gramáticas malformadas e problemas de leitura, devem ser comunicadas explicitamente, preferencialmente com `std::expected<T, Error>` ou diagnósticos estruturados. Não usaremos exceções como fluxo normal de controle.

Erros e avisos devem informar contexto, posição no arquivo e causa, permitindo testes precisos e mensagens úteis para quem usa o gerador.

### 4. Tipos que representem o domínio

A gramática e suas expressões devem ser modeladas com tipos que restrinjam estados inválidos. Terminais, não terminais, sequências, alternativas, repetições e opções têm semânticas distintas e não precisam ser representados por uma única estrutura com vários campos opcionais implícitos.

As etapas de análise devem ter entradas e resultados claros. Evitaremos armazenar estado derivado na árvore sintática quando puder ser produzido e administrado separadamente, sem criar complexidade desnecessária.

### 5. Módulos pequenos e responsabilidades claras

A organização conceitual será:

1. **Leitura da EBNF**: análise léxica e sintática da gramática de entrada.
2. **Modelo da gramática**: representação e identificação dos símbolos.
3. **Validação**: referências, produtividade, alcançabilidade e recursão à esquerda.
4. **Análise**: anulabilidade e conjuntos FIRST/FOLLOW.
5. **Verificação LL(1)**: identificação e diagnóstico de conflitos, preservando os comportamentos especiais exigidos pelo trabalho.
6. **Geração**: emissão do analisador descendente recursivo em ANSI C.

Essas são responsabilidades, **não uma exigência de uma classe ou diretório para cada item**. Não criaremos abstrações, interfaces ou hierarquias de herança sem uma necessidade demonstrável.

### 6. Compatibilidade verificável

Reescrever não significa alterar arbitrariamente o comportamento. A sintaxe EBNF aceita, as opções de linha de comando, os critérios de validação, os diagnósticos relevantes e a saída gerada devem continuar atendendo ao enunciado.

Usaremos os testes e resultados esperados como referência, além de testes unitários e de integração para os novos componentes. Quando necessário, compilaremos o C gerado com as opções estritas previstas no trabalho.

A implementação original contém partes ainda não concluídas; portanto, **o código antigo não será tratado como fonte infalível de comportamento**. O enunciado e os casos de teste prevalecem.

### 7. Qualidade sem excesso de engenharia

Pretendemos usar CMake, avisos rigorosos de compilação, testes automatizados e sanitizers quando disponíveis. Ao mesmo tempo, evitaremos dependências externas e generalizações prematuras: este é um gerador de parsers, não um framework de compiladores.


## Guia de leitura para correção e revisão

Esta implementação **não é uma tradução linha a linha do esqueleto em C**. Para avaliá-la, considere primeiro o contrato do trabalho (entrada EBNF, diagnósticos, validações, saída ANSI C e testes) e depois localize a responsabilidade equivalente no novo desenho. A ausência de uma função ou estrutura com o mesmo nome do código original não implica ausência de funcionalidade.

### O que significa escrever C++23 de maneira inspirada em Rust?

A expressão *Rust-ic* descreve uma preferência de projeto, **não uma alegação de que C++ oferece as garantias do borrow checker de Rust**. Queremos tornar explícitos ownership, falhas e estados válidos, usando recursos idiomáticos de C++23:

- **Erros como valores:** `std::expected<T, ErroLexico>` representa uma operação que pode produzir um valor ou falhar. `std::unexpected` transporta o erro; `std::expected<void, E>` representa sucesso sem valor, como `Result<(), E>` em Rust. `std::optional<T>` é reservado a ausências normais, não a falhas que exigem diagnóstico. Exceções não são o mecanismo previsto para erros esperados da gramática.
- **Ownership e movimentação:** objetos como `Scanner` possuem seus dados; `std::string` mantém lexemas e literais com gerenciamento automático. `std::move` explicita a transferência de um valor quando não precisamos mais da origem, sem prometer que toda movimentação elimina alocações. Evitamos `new`/`delete` manuais e cópias desnecessárias, sem proibir cópias úteis.
- **Tipos que expressem alternativas:** `TokenType` é um `std::variant` de tipos específicos (`Terminal`, `NaoTerminal`, `Literal`, `ExpressaoRegular`, `Simbolo`, `PalavraReservada`, `Eof`). Isso evita hierarquias polimórficas e campos cuja validade dependa de flags externas.
- **Evitar comportamento indefinido (UB):** o código deve verificar limites e EOF antes de consumir caracteres, manter invariantes de posição, evitar referências ou `std::string_view` pendentes e converter `char` para `unsigned char` ao chamar funções de classificação de `<cctype>`. C++ não torna esses cuidados automáticos: testes, revisão e sanitizers continuam necessários.
- **Interfaces pequenas e explícitas:** `[[nodiscard]]` sinaliza resultados que não devem ser ignorados; funções auxiliares devem ter responsabilidades delimitadas. `noexcept` só deve ser usado quando a operação realmente não pode lançar exceções (por exemplo, funções que não alocam).

### Como ler o lexer em desenvolvimento

Em `ebnf/Lexer.hpp` e `ebnf/Lexer.cpp`, `Scanner` administra a fonte, o cursor e as posições. `Lexer` converte essa sequência em `Token` ou `ErroLexico`. A localização do token registra o início e o fim da leitura.

A leitura de literais ilustra a decomposição pretendida: `casa()` confere o delimitador sem consumir; `ler_literal()` coordena a operação; `ler_conteudo_literal()` percorre o conteúdo; `ler_escape()` interpreta escapes. Essa separação busca legibilidade e facilita testes e diagnósticos.

**Importante:** esses exemplos descrevem a arquitetura e o código em evolução, não afirmam que todos os caminhos do lexer, o parser, as análises ou a geração já estejam concluídos. A implementação deve ser julgada também pelo comportamento observável e pelos testes, não apenas pelas intenções aqui documentadas.

### Critérios práticos para quem avaliar

1. Confira os requisitos do enunciado e os casos em `testes/`, sem presumir equivalência nominal com `src-old/`.
2. Distinga o executável **gerador em C++23** do **parser gerado em ANSI C**: são contratos de compilação diferentes.
3. Verifique os caminhos de sucesso e erro, inclusive posições, EOF, literais malformados e referências inválidas.
4. Considere ownership, validade de referências e ausência de UB; não confunda `std::move` com uma garantia de segurança de memória.
5. Identifique funcionalidades ainda não implementadas como pendências reais, sem tomar este documento como evidência de que estejam prontas.

## Estratégia de desenvolvimento

A implementação será incremental: primeiro a infraestrutura mínima e os tipos fundamentais; depois leitura da EBNF, representação da gramática, validações, conjuntos e LL(1); por fim, geração de código e integração completa com Micro C.

Cada etapa deve ter um propósito claro e testes correspondentes. Não haverá uma tradução mecânica arquivo a arquivo de `src-old/` para `src/`: a organização nova deve acompanhar o desenho novo.

## Estado atual

**Este documento registra as decisões e intenções da reescrita.** A existência deste README não indica que a migração, as interfaces ou os módulos descritos já estejam implementados. As escolhas poderão evoluir se os testes, o enunciado ou problemas concretos justificarem ajustes.

A meta final é entregar o trabalho exigido **e** manter um código-fonte que seja consistente, compreensível e agradável de desenvolver — mesmo depois da avaliação acadêmica.
