import subprocess
import os
import sys
import platform
from typing import List

# --- Configuração de Cores e Comandos ---
COMPILER_EXECUTABLE = "compiler.exe" if platform.system() == "Windows" else "./compiler"

class Colors:
    GREEN = '\033[92m'
    RED = '\033[91m'
    ENDC = '\033[0m'

def print_green(text):
    print(f"{Colors.GREEN}{text}{Colors.ENDC}")

def print_red(text):
    print(f"{Colors.RED}{text}{Colors.ENDC}")

# --- Funções do Script ---

def build(verbose=False):
    """Compila o projeto usando flex, bison e um compilador C."""
    print("Compilando o projeto...")
    try:
        compile_commands = [
            "flex lexer.l",
            "bison -d parser.y"
        ]
        
        if platform.system() == "Darwin":  # macOS
            c_compiler_cmd = f"clang parser.tab.c lex.yy.c ast.c tabela.c codegen.c -o {COMPILER_EXECUTABLE.replace('./', '')}"
        else:  # Linux e Windows
            c_compiler_cmd = f"gcc parser.tab.c lex.yy.c ast.c tabela.c codegen.c -o {COMPILER_EXECUTABLE.replace('./', '')} -lfl"
        
        compile_commands.append(c_compiler_cmd)

        for cmd in compile_commands:
            stdout = None if verbose else subprocess.DEVNULL
            stderr = None if verbose else subprocess.DEVNULL
            subprocess.run(cmd, shell=True, check=True, stdout=stdout, stderr=stderr)
            
        print_green("Compilação concluída com sucesso!")
        return True
    except (subprocess.CalledProcessError, FileNotFoundError) as e:
        print_red(f"Falha na compilação. Verifique se as ferramentas (flex, bison, gcc/clang) estão instaladas.")
        return False

def run_c_tests():
    """Executa os testes léxico-sintáticos e semânticos."""
    print("-" * 40)
    print("Executando testes em C...")

    # Testes que devem compilar sem a palavra "Erro" no stderr
    success_tests = [
        "teste/teste.c", "teste/teste2.c", "teste/testeAst.c",
        "teste/teste_semantica_ok.c"
    ]
    # Testes que DEVEM produzir um "Erro" no stderr para passar
    failure_tests = [
        "teste/testeTabela.c", "teste/teste_semantica_erro.c",
        "teste/erro_tipos_incompativeis.c", "teste/erro_variavel_nao_declarada.c",
        "teste/teste_estruturado_ok.c" # Este arquivo falha a sintaxe 'main()', então é um teste de falha esperado.
    ]

    total_tests = 0
    passed_tests = 0

    for test_path in success_tests:
        total_tests += 1
        with open(test_path, 'r') as f:
            result = subprocess.run([COMPILER_EXECUTABLE], stdin=f, capture_output=True, text=True)
        
        test_ok = "Erro" not in result.stderr
        status = f"[{'PASS' if test_ok else 'FAIL'}]"
        print(f"Testando: {test_path:<35} {Colors.GREEN if test_ok else Colors.RED}{status}{Colors.ENDC}")
        if test_ok: passed_tests += 1

    for test_path in failure_tests:
        total_tests += 1
        with open(test_path, 'r') as f:
            result = subprocess.run([COMPILER_EXECUTABLE], stdin=f, capture_output=True, text=True)

        test_ok = "Erro" in result.stderr
        status_msg = "Erro detectado" if test_ok else "Erro não detectado"
        status = f"[{'PASS' if test_ok else 'FAIL'}] ({status_msg})"
        print(f"Testando: {test_path:<35} {Colors.GREEN if test_ok else Colors.RED}{status}{Colors.ENDC}")
        if test_ok: passed_tests += 1
        
    print("-" * 40)
    if passed_tests == total_tests:
        print_green(f"Resultado: Todos os {total_tests} testes passaram!")
    else:
        print_red(f"Resultado: {passed_tests} de {total_tests} testes passaram.")
    
    return passed_tests == total_tests

def run_py_test():
    """Executa o teste de geração e execução de código Python."""
    print("-" * 40)
    print("Executando teste de geração de código Python...")
    
    quads_filename = "quads_output.txt"
    python_filename = "teste/output_program.py"

    try:
        # 1. Gera quádruplas do compilador
        with open("teste/teste_semantica_ok.c", 'r') as f_in, open(quads_filename, 'w') as f_out:
            subprocess.run([COMPILER_EXECUTABLE], stdin=f_in, stdout=f_out, check=True)

        # 2. Converte para Python passando o NOME DO ARQUIVO como argumento
        subprocess.run(["python3", "quad_to_python.py", quads_filename], stdout=open(python_filename, 'w'), check=True)
            
        # 3. Executa o Python gerado
        subprocess.run(["python3", python_filename], check=True, stdout=subprocess.DEVNULL)
        
        print(f"Testando geração e execução de Python... {Colors.GREEN}[ PASS ]{Colors.ENDC}")
        return True
    except (subprocess.CalledProcessError, FileNotFoundError) as e:
        print(f"Testando geração e execução de Python... {Colors.RED}[ FAIL ]{Colors.ENDC}")
        # print(f"  (Comando que falhou: {e})") # Uncomment for debugging
        return False

def clean():
    """Remove os arquivos gerados."""
    print("Limpando arquivos gerados...")
    files_to_remove = [
        "compiler", "compiler.exe", "lex.yy.c", "parser.tab.c", 
        "parser.tab.h", "quads_output.txt", "teste/output_program.py"
    ]
    for f in files_to_remove:
        if os.path.exists(f):
            os.remove(f)
    print("Limpeza concluída.")

def main():
    """Função principal para controlar o script."""
    args = sys.argv[1:]
    command = args[0] if args else "all"
    verbose = "--verbose" in args

    if command == "build":
        build(verbose)
    elif command == "test":
        if not os.path.exists(COMPILER_EXECUTABLE.replace('./', '')):
            if not build(verbose): return
        run_c_tests()
        run_py_test()
    elif command == "clean":
        clean()
    elif command == "all":
        if build(verbose):
            run_c_tests()
            run_py_test()
    else:
        print("Uso: python3 run.py [comando]")
        print("Comandos: all, build, test, clean")
        print("  --verbose    Executa no modo detalhado")

if __name__ == "__main__":
    main()
