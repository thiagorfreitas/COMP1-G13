# Plano de Implementação: Análise Semântica e Código Intermediário

## Fase 1: Análise Semântica

**Objetivo:** Implementar verificações semânticas para garantir a correção do código C de entrada, focando em tipos e declarações.

**Arquivos a Modificar:**

1.  **`parser.y`:**
    *   Integrar chamadas para funções de análise semântica nas regras gramaticais relevantes (atribuições, expressões, declarações).
    *   Melhorar a verificação de tipos em operações aritméticas e lógicas na regra `expr`.
    *   Refinar a verificação de tipos na regra `atribuicao`, comparando o tipo da variável com o tipo da expressão atribuída.
    *   Garantir que a verificação de declaração/redeclaração na regra `declaracao_var` e uso de variáveis não declaradas (`ID` em `expr`, `atribuicao`) seja robusta.
    *   Melhorar o tratamento e a comunicação de erros semânticos.
2.  **`ast.h` / `ast.c`:**
    *   Revisar a estrutura `NoAST` para garantir que o campo `tipoDado` armazene e propague consistentemente as informações de tipo necessárias para a análise semântica.
    *   Potencialmente adicionar funções auxiliares em `ast.c` para facilitar a recuperação e verificação de tipos na AST.
3.  **`tabela.h` / `tabela.c`:**
    *   Avaliar a necessidade de aprimorar a tabela de símbolos para suportar escopos (embora, dada a simplicidade atual, o escopo global possa ser mantido inicialmente).
    *   Garantir que a busca (`buscar_simbolo`) e adição (`adicionar_simbolo`) funcionem corretamente com as verificações semânticas.
4.  **(Opcional) `semantic.c` / `semantic.h`:**
    *   Considerar a criação de arquivos dedicados para encapsular a lógica de análise semântica, como funções de verificação de tipo (`check_types`, `compare_types`) e gerenciamento de erros semânticos.

**Verificações Principais:**

*   **Compatibilidade de Tipos:** Verificar se os operandos em expressões (`+`, `-`, `*`, `/`, `%`, `==`, `!=`, `<`, `>`, `<=`, `>=`) são de tipos compatíveis.
*   **Atribuição:** Verificar se o tipo da expressão à direita da atribuição é compatível com o tipo da variável à esquerda.
*   **Declarações:** Verificar se variáveis são declaradas antes do uso e se não há redeclarações no mesmo escopo (considerando o escopo global atual).

## Fase 2: Geração de Código Intermediário (Código de Três Endereços)

**Objetivo:** Traduzir a Árvore Sintática Abstrata (AST), validada semanticamente, para uma representação de código intermediário (CI), como código de três endereços (ex: quádruplas).

**Arquivos a Modificar/Criar:**

1.  **`codegen.h` / `codegen.c` (Novos):**
    *   Definir a estrutura para representar as instruções do código intermediário (ex: `struct Quadrupla { Operador op; Endereco arg1; Endereco arg2; Endereco resultado; }`).
    *   Implementar funções para gerar novas instruções de CI.
    *   Implementar funções para gerenciar variáveis temporárias (`new_temp`) e rótulos (`new_label`).
    *   Criar a função principal de geração de código que percorre a AST (`generate_code(NoAST* node)`).
2.  **`ast.h` / `ast.c`:**
    *   Pode ser necessário adicionar campos à `NoAST` para armazenar informações relevantes para a geração de código (ex: o endereço/temporário que contém o resultado de uma expressão).
3.  **`parser.y`:**
    *   Após a análise sintática e semântica bem-sucedida (no final da regra `%start` ou `programa`), chamar a função principal de geração de código (`generate_code(raizAST)`).
4.  **`main` (em `parser.y` ou arquivo separado):**
    *   Adicionar lógica para imprimir ou salvar o código intermediário gerado.

**Lógica de Geração:**

*   **Percorrer a AST:** Usar uma travessia pós-ordem (ou similar) na AST.
*   **Expressões:** Para cada nó de expressão (`AST_EXPR`), gerar instruções de três endereços para calcular o resultado, armazenando-o em um temporário. O endereço do temporário pode ser associado ao nó da AST.
*   **Atribuições:** Gerar uma instrução de cópia (`MOV` ou `=`) para mover o resultado da expressão (obtido do nó filho direito) para o endereço da variável (obtido do nó filho esquerdo).
*   **Estruturas de Controle (If, While, For):** Gerar instruções de salto condicional (`IF_FALSE_GOTO`, `GOTO`) e rótulos para implementar a lógica de controle de fluxo.
*   **Declarações:** Podem não gerar código diretamente, mas a informação da tabela de símbolos é crucial.

## Fase 3: Validação

*   **Novos Testes:** Criar novos arquivos `teste*.c` que incluam casos para testar as verificações semânticas (erros de tipo, variáveis não declaradas, redeclarações) e a geração de código intermediário para diferentes construções (expressões complexas, atribuições, if/else, while).
*   **Execução:** Compilar o compilador modificado e executá-lo com os novos arquivos de teste.
*   **Verificação:** Analisar a saída de erros semânticos e o código intermediário gerado para garantir a correção.

