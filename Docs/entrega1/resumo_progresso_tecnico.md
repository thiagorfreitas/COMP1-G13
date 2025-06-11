# Resumo do Progresso Técnico

Este tópico fornece uma visão geral detalhada do progresso técnico alcançado no projeto, abordando os objetivos, as entregas e os desafios superados.

## Objetivos Planejados das Sprints:

Os seguintes objetivos foram definidos e buscados durante as sprints realizadas:

* **Estudo Individual de Cada Integrante sobre as Tecnologias Flex e Bison:** Garantir que todos os membros da equipe tivessem um entendimento sólido das ferramentas que seriam utilizadas para a análise léxica e sintática do compilador.
* **Levantamento de Ideias para o Projeto:** Realizar brainstorms e discussões para conceituar e planejar as funcionalidades e a arquitetura geral do compilador.
* **Estruturação do Analisador Léxico e Sintático:** Desenvolver e integrar as primeiras fases do compilador, responsáveis por processar a entrada do código-fonte e verificar sua estrutura.
* **Documentação do Projeto:** Manter um registro contínuo do desenvolvimento, das decisões tomadas e das funcionalidades implementadas.

## Entregas Realizadas:

As entregas que foram efetivamente concluídas e apresentadas incluem:

* **Estudo Individual de Cada Integrante sobre as Tecnologias Flex e Bison:** Comprovação do domínio das ferramentas através de demonstrações ou relatórios.
* **Levantamento de Ideias para o Projeto:** Documentação das ideias e do plano inicial do compilador.
* **Estruturação do Analisador Léxico e Sintático:** Versões funcionais do analisador léxico (Flex) e do analisador sintático (Bison), capazes de processar partes da linguagem de origem.
* **Documentação do Projeto:** Atualização contínua dos documentos técnicos e de progresso.

## Dificuldades Enfrentadas e Soluções:

O processo de desenvolvimento não foi isento de desafios:

* **Dificuldade Inicial na Estrutura do Analisador Sintático:** No início do projeto, o grupo encontrou um certo grau de dificuldade para compreender completamente a estrutura e o funcionamento do analisador sintático, especialmente com o Bison e a construção da AST.
* **Solução:** Para superar essa barreira, foi realizada uma **reunião com todo o grupo**. Durante esta reunião, foram discutidos os bloqueios individuais de cada integrante, e um apoio mútuo foi oferecido para garantir que todos tivessem o entendimento necessário para prosseguir com a implementação. Esta abordagem colaborativa foi crucial para o avanço.

## Testes Realizados:

Os testes foram realizados **parcialmente**.

* **Explicação Adicional:** Os testes realizados até o momento focaram principalmente na validação do fluxo do C?parser, que envolve a correta operação do analisador léxico e sintático. Isso inclui a verificação da geração de tokens para palavras-chave, literais e comentários, bem como a correta interpretação da gramática básica (declarações, atribuições e controle). Testes mais abrangentes, incluindo a validação completa da geração de código e a semântica, serão abordados nas próximas etapas.