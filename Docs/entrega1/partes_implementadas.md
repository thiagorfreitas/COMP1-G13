# Partes Já Implementadas

Este tópico detalha as componentes do compilador que já foram desenvolvidas e integradas ao projeto.

## Componentes Atuais do Compilador:

As seguintes fases do compilador foram implementadas com sucesso até o momento:

1.  **Analisador Léxico (Scanner):**
    * **O que é:** É a primeira fase de um compilador. Ele lê o código-fonte caractere por caractere e agrupa-os em sequências significativas chamadas "tokens".
    * **Função no Projeto:** O analisador léxico é responsável por identificar palavras-chave, identificadores, operadores, literais e outros símbolos na linguagem de origem (C?), transformando o fluxo de caracteres em uma sequência de tokens que o analisador sintático pode processar. A implementação utiliza **Flex**.

2.  **Analisador Sintático (Parser):**
    * **O que é:** Recebe a sequência de tokens do analisador léxico e verifica se ela está de acordo com a gramática da linguagem de programação. Ele constrói uma representação hierárquica do código, como uma árvore de análise sintática (Parse Tree ou AST - Abstract Syntax Tree).
    * **Função no Projeto:** O analisador sintático do projeto (implementado com **Bison**) é capaz de processar declarações, atribuições e estruturas de controle, garantindo que a estrutura do código-fonte esteja correta. Isso inclui a validação da sintaxe do C?parser.

3.  **Geração de Código Inicial:**
    * **O que é:** Esta fase é responsável por converter a representação intermediária do código (como a AST gerada pelo analisador sintático) em um formato que pode ser processado posteriormente ou que serve como base para a geração do código final.
    * **Função no Projeto:** Embora seja uma "geração de código inicial", indica que o compilador já é capaz de produzir alguma forma de saída a partir da análise léxica e sintática. No contexto do projeto, isso inclui a **base para AST (Abstract Syntax Tree)** e a **geração de código Python**, sugerindo um protótipo de transpiler C?Python.

A implementação dessas partes fundamentais demonstra um progresso significativo no desenvolvimento do compilador, estabelecendo a base para as fases subsequentes, como a análise semântica e a otimização.