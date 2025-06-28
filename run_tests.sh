#!/bin/bash

# Cores para a saída
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Determina se o modo verbose está ativo
VERBOSE=false
if [[ "$1" == "--verbose" ]]; then
    VERBOSE=true
fi

# Função para compilar o projeto
build() {
    echo "Compilando o projeto para macOS..."
    if [ "$VERBOSE" = true ]; then
        flex lexer.l
        bison -d parser.y
        clang parser.tab.c lex.yy.c ast.c tabela.c codegen.c -o compiler
    else
        # Compilação silenciosa
        flex lexer.l > /dev/null 2>&1
        bison -d parser.y > /dev/null 2>&1
        clang parser.tab.c lex.yy.c ast.c tabela.c codegen.c -o compiler > /dev/null 2>&1
    fi

    if [ $? -eq 0 ]; then
        echo -e "${GREEN}Compilação concluída com sucesso!${NC}"
    else
        echo -e "${RED}Falha na compilação.${NC}"
        exit 1
    fi
}

# Função para rodar os testes C com log limpo
test_c() {
    echo "----------------------------------------"
    echo "Executando testes léxico-sintáticos..."

    # Testes que devem ser compilados com sucesso
    SUCCESS_TESTS=(
        "teste/teste.c"
        "teste/teste2.c"
        "teste/testeAst.c"
        "teste/teste_semantica_ok.c"
        "teste/teste_estruturado_ok.c"
    )

    # Testes que devem falhar
    FAILURE_TESTS=(
        "teste/testeTabela.c"
        "teste/teste_semantica_erro.c"
        "teste/erro_tipos_incompativeis.c"
        "teste/erro_variavel_nao_declarada.c"
    )

    TOTAL_TESTS=0
    PASSED_TESTS=0

    # Executa testes de sucesso esperado
    for test_file in "${SUCCESS_TESTS[@]}"; do
        ((TOTAL_TESTS++))
        printf "Testando: %-35s" "$test_file"
        ERROR_OUTPUT=$(./compiler < "$test_file" 2>&1 >/dev/null)
        # Sucesso significa que a palavra "Erro" NÃO está na saída de erro
        if [[ "$ERROR_OUTPUT" != *"Erro"* ]]; then
            echo -e "${GREEN}[ PASS ]${NC}"
            ((PASSED_TESTS++))
        else
            echo -e "${RED}[ FAIL ]${NC}"
        fi
    done

    # Executa testes de falha esperada
    for test_file in "${FAILURE_TESTS[@]}"; do
        ((TOTAL_TESTS++))
        printf "Testando: %-35s" "$test_file"
        ERROR_OUTPUT=$(./compiler < "$test_file" 2>&1 >/dev/null)
        # Sucesso aqui significa que a palavra "Erro" ESTÁ na saída de erro
        if [[ "$ERROR_OUTPUT" == *"Erro"* ]]; then
            echo -e "${GREEN}[ PASS ] (Erro detectado corretamente)${NC}"
            ((PASSED_TESTS++))
        else
            echo -e "${RED}[ FAIL ] (Erro não foi detectado)${NC}"
        fi
    done

    echo "----------------------------------------"
    if [ "$PASSED_TESTS" -eq "$TOTAL_TESTS" ]; then
        echo -e "${GREEN}Resultado: Todos os $TOTAL_TESTS testes passaram!${NC}"
    else
        echo -e "${RED}Resultado: $PASSED_TESTS de $TOTAL_TESTS testes passaram.${NC}"
    fi
}

# Função para rodar o teste de geração de código Python
test_py() {
    echo "----------------------------------------"
    echo "Executando teste de geração de código Python..."
    printf "Testando geração e execução de Python..."

    # Gera quádruplas e converte para Python, verificando cada passo
    ./compiler < "teste/teste_semantica_ok.c" > quads_teste_c.txt 2>/dev/null
    if [ $? -ne 0 ]; then
        echo -e "${RED}[ FAIL ] (Compilador falhou ao gerar quádruplas)${NC}"
        return
    fi

    python3 quad_to_python.py quads_teste_c.txt > teste/output_program.py 2>/dev/null
    if [ $? -ne 0 ]; then
        echo -e "${RED}[ FAIL ] (Script de conversão para Python falhou)${NC}"
        return
    fi
    
    python3 teste/output_program.py > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}[ PASS ]${NC}"
    else
        echo -e "${RED}[ FAIL ] (Execução do script Python gerado falhou)${NC}"
    fi
}

# Função para limpar os arquivos gerados
clean() {
    echo "Limpando arquivos gerados..."
    rm -f compiler lex.yy.c parser.tab.c parser.tab.h
    rm -f teste/output_quads.txt teste/output_program.py quads_teste_c.txt
    echo "Limpeza concluída."
}

# Menu de ajuda
usage() {
    echo "Uso: $0 [comando]"
    echo
    echo "Comandos:"
    echo "  all          Compila e executa todos os testes (padrão)."
    echo "  build        Apenas compila o projeto."
    echo "  test         Executa os testes léxico-sintáticos e de Python."
    echo "  clean        Remove os arquivos gerados."
    echo "  --verbose    Executa no modo detalhado, mostrando todos os logs."
    echo
}

# Controlador principal do script
main() {
    if [ "$VERBOSE" = true ]; then
        build
        echo "MODO VERBOSO ATIVADO"
        echo "--- Teste de sucesso esperado (teste_estruturado_ok.c) ---"
        ./compiler < teste/teste_estruturado_ok.c
        echo "--- Teste de falha esperada (testeTabela.c) ---"
        ./compiler < teste/testeTabela.c
        exit 0
    fi

    case "$1" in
        all|"")
            build
            test_c
            test_py
            ;;
        build)
            build
            ;;
        test)
            test_c
            test_py
            ;;
        clean)
            clean
            ;;
        *)
            usage
            ;;
    esac
}

main "$1"
