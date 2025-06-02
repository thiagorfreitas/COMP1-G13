# Compiladores 1 - Grupo 13

## O projeto

O projeto foi desenvolvido por alunos do curso de Engenharia de Software, do Grupo 13 da disciplina de Compiladores 1 ofertada na Universidade de Brasília (UnB) - Campus FCTE, e tem como objetivo principal a construção de um compilador simples que converte código C para Python. O projeto é uma aplicação prática dos conceitos aprendidos na disciplina e visa proporcionar uma experiência enriquecedora no desenvolvimento de software.

## Funcionalidades

- Análise léxica de código C usando Flex.
- Análise sintática e geração de árvore de derivação com Bison.
- Suporte a estruturas básicas de controle e funções.

## Requisitos

Certifique-se de ter as seguintes ferramentas instaladas:

- **Flex**: Para análise léxica.
- **Bison**: Para análise sintática.
- **GCC**: Para compilar o código gerado.


## Como Usar

1. Compile o analisador léxico e sintático:
    ```bash
    flex lexer.l
    bison -d parser.y
    gcc parser.tab.c lex.yy.c ast.c tabela.c -o compiler -lfl
    ```

2. Testar o fluxo léxico-sintático:
    ```bash
    ./compiler < teste.c
    ./compiler < teste2.c
    ./compiler < testeTabela.c
    ./compiler < testeAst.c
    ```


## Contribuidores

<table>
  <tr>
    <td align="center"><a href="https://github.com/ArthurGabrieel"><img style="border-radius: 50%;" src="https://github.com/ArthurGabrieel.png" width="130px;" alt=""/><br /><sub><b>Arthur Gabriel</b></sub></a><br/></td>
    <td align="center"><a href="https://github.com/Caio-bergbjj"><img style="border-radius: 50%;" src="https://github.com/Caio-bergbjj.png" width="130px;" alt=""/><br /><sub><b>Caio Berg</b></sub></a><br/></td>
    <td align="center"><a href="https://github.com/EmivaltoJrr"><img style="border-radius: 50%;" src="https://github.com/EmivaltoJrr.png" width="130px;" alt=""/><br /><sub><b>Emivalto Junior</b></sub></a><br/></td>
    <td align="center"><a href="https://github.com/thiagorfreitas"><img style="border-radius: 50%;" src="https://github.com/thiagorfreitas.png" width="130px;" alt=""/><br /><sub><b>Thiago Freitas</b></sub></a><br/></td>
    <td align="center"><a href="https://github.com/Wengel-Rodrigues"><img style="border-radius: 50%;" src="https://github.com/Wengel-Rodrigues.png" width="130px;" alt=""/><br /><sub><b>Wengel Rodrigues</b></sub></a><br/></td>
    <td align="center"><a href="https://github.com/wesleysantos00"><img style="border-radius: 50%;" src="https://github.com/wesleysantos00.png" width="130px;" alt=""/><br /><sub><b>Wesley Santos</b></sub></a><br/></td>
  </tr>
</table>


## Histórico de Versões



| Versão |    Data    |                      Descrição                      |      Autor(es)      |
| :----: | :--------: | :-------------------------------------------------: | :-----------------: |
|  1.0   | 23/04/2025 | Versão inicial de documento de descrição do projeto | [Thiago Freitas](https://github.com/thiagorfreitas) |

