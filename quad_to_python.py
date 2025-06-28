#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Tradutor de Código Intermediário de Alto Nível (HLIR) para Python.

Este script lê um arquivo de quádruplas estruturadas gerado por um compilador
(como o `codegen.c` que projetaamos) e o converte em um script Python funcional.

Ele é projetado para entender opcodes de alto nível como:
- Expressões aritméticas e relacionais explícitas (ADD, LT, EQ, etc.)
- Estruturas de controle de fluxo claras (IF_START, THEN_START, WHILE_COND, etc.)
- Chamadas de função com parâmetros empilhados (PARAM, CALL)

A tradução para Python se torna quase direta, gerenciando um nível de indentação
e um stack de parâmetros.
"""

import sys
import re

def parse_instruction(line):
    """
    Analisa uma única linha do HLIR e a transforma em um dicionário estruturado.
    Isso centraliza a lógica de parsing, limpando a função de tradução.
    """
    line = line.strip()
    if not line:
        return None

    # Padrão geral para capturar opcode e o resto da linha
    match = re.match(r"^\s*\d*:\s*([A-Z_]+)\s*(.*)$", line)
    if not match:
        return None

    opcode, rest = match.groups()
    instr = {"opcode": opcode, "args": []}

    # Padrão para "res = arg1 op arg2" ou "res = op arg1"
    # Ex: t0 = a < b, t1 = -t0, res = a
    match_expr = re.match(r"([^=]+)=\s*(.*)", rest)
    if match_expr:
        instr["result"] = match_expr.group(1).strip()
        expression = match_expr.group(2).strip()
        
        # Divide a expressão por espaços para obter os operandos
        # Ex: "a < b" -> ["a", "<", "b"]
        # Ex: "a + b" -> ["a", "+", "b"]
        instr["args"] = expression.split()
        return instr

    # Padrão para opcodes com argumentos separados por vírgula ou um único argumento
    # Ex: CALL "printf", 2
    # Ex: IF t0
    # Ex: PARAM 5
    if rest:
        instr["args"] = [p.strip() for p in rest.split(',')]
    
    return instr


def translate_hlir_to_python(instructions):
    """
    Traduz uma lista de instruções HLIR (já analisadas) para código Python.
    """
    python_code = []
    indent_level = 0
    param_stack = []

    # Mapeia opcodes do HLIR para operadores Python
    op_map = {
        'ADD': '+', 'SUB': '-', 'MUL': '*', 'DIV': '/', 'MOD': '%',
        'LT': '<', 'GT': '>', 'LTE': '<=', 'GTE': '>=', 'EQ': '==', 'NEQ': '!='
    }

    for instr in instructions:
        opcode = instr["opcode"]
        args = instr.get("args", [])
        result = instr.get("result")
        indent_str = "    " * indent_level

        # --- Expressões e Atribuições ---
        if opcode == 'ASSIGN':
            python_code.append(f"{indent_str}{result} = {args[0]}")
        
        elif opcode in op_map:
            # Formato: resultado = arg1 op arg2
            arg1 = args[0]
            op_symbol = op_map[opcode]
            arg2 = args[2] # Pula o próprio operador que está em args[1]
            python_code.append(f"{indent_str}{result} = {arg1} {op_symbol} {arg2}")

        elif opcode == 'UMINUS':
            # Formato: resultado = -arg1
            arg1 = args[1] # Pula o "-" que está em args[0]
            python_code.append(f"{indent_str}{result} = -{arg1}")

        # --- Chamada de Função ---
        elif opcode == 'PARAM':
            param_stack.append(args[0])

        elif opcode == 'CALL':
            func_name = args[0].strip('"')
            num_params = int(args[1])
            
            # Parâmetros foram empilhados na ordem inversa, então pop() os recupera na ordem correta
            call_args = [param_stack.pop() for _ in range(num_params)]

            # Tratamento especial para printf, que não existe em Python
            if func_name == 'printf':
                format_str = call_args[0]
                vars_to_format = call_args[1:]
                
                # Se houver variáveis para formatar, usa o operador %
                if vars_to_format:
                    # Se for mais de uma variável, o operador % espera uma tupla
                    vars_str = f"({', '.join(vars_to_format)})" if len(vars_to_format) > 1 else vars_to_format[0]
                    python_code.append(f'{indent_str}print({format_str} % {vars_str})')
                else:
                    # Se não houver, apenas imprime a string
                    python_code.append(f'{indent_str}print({format_str})')
            else:
                # Para outras funções, gera uma chamada de função normal
                args_str = ", ".join(call_args)
                python_code.append(f"{indent_str}{func_name}({args_str})")

        # --- Controle de Fluxo Estruturado ---
        elif opcode == 'IF_START':
            condition = args[0]
            python_code.append(f"{indent_str}if {condition}:")
        
        elif opcode == 'WHILE_START' or opcode == 'DO_WHILE_START':
            python_code.append(f"{indent_str}while True:")

        elif opcode == 'THEN_START' or opcode == 'BODY_START':
            indent_level += 1
            
        elif opcode == 'BREAK_IF_FALSE':
            condition = args[0]
            python_code.append(f"{indent_str}if not {condition}:")
            python_code.append(f"{indent_str}    break")

        elif opcode == 'ELSE_START':
            else_indent_str = "    " * (indent_level - 1)
            python_code.append(f"{else_indent_str}else:")

        elif opcode == 'IF_END' or opcode == 'WHILE_END' or opcode == 'DO_WHILE_END':
            indent_level -= 1

        elif opcode == 'THEN_START' or opcode == 'BODY_START':
            indent_level += 1

        elif opcode == 'ELSE_START':
            # O 'else' pertence ao nível de indentação anterior
            else_indent_str = "    " * (indent_level - 1)
            python_code.append(f"{else_indent_str}else:")

        elif opcode == 'IF_END' or opcode == 'WHILE_END':
            indent_level -= 1

        elif opcode == 'WHILE_START':
            python_code.append(f"{indent_str}while True:")
        
        elif opcode == 'THEN_START' or opcode == 'BODY_START':
            indent_level += 1
            
        elif opcode == 'BREAK_IF_FALSE':
            condition = args[0]
            python_code.append(f"{indent_str}if not {condition}:")
            python_code.append(f"{indent_str}    break")

        elif opcode == 'IF_END' or opcode == 'WHILE_END':
            indent_level -= 1

        # Opcodes que são apenas marcadores para o gerador de código
        # e não têm ação direta no tradutor Python podem ser ignorados.
        elif opcode in ['WHILE_START']:
            pass

    return python_code


# --- Bloco de Execução Principal ---
def main():
    """
    Ponto de entrada do script. Lê um arquivo de entrada (ou stdin)
    e imprime o código Python traduzido.
    """
    if len(sys.argv) > 1:
        input_file = sys.argv[1]
        try:
            with open(input_file, 'r', encoding='utf-8') as f:
                hlir_lines = f.readlines()
        except FileNotFoundError:
            print(f"Erro: Arquivo de entrada '{input_file}' não encontrado.", file=sys.stderr)
            sys.exit(1)
    else:
        print("Lendo da entrada padrão (stdin). Pressione Ctrl+D (Linux/macOS) ou Ctrl+Z (Windows) para finalizar.", file=sys.stderr)
        hlir_lines = sys.stdin.readlines()

    # Filtra e analisa as linhas do código intermediário
    instructions = [parse_instruction(line) for line in hlir_lines]
    instructions = [instr for instr in instructions if instr] # Remove linhas nulas/vazias

    # Realiza a tradução
    python_output_lines = translate_hlir_to_python(instructions)

    # Imprime o resultado formatado
    print("#" + "="*38)
    print("# Código Python Gerado Automaticamente #")
    print("#" + "="*38)
    print()
    
    if not python_output_lines:
        print("# (Nenhum código foi gerado)")
    else:
        for py_line in python_output_lines:
            print(py_line)
    
    print()
    print("#" + "="*40)
    print("#       Fim do Código Gerado         #")
    print("#" + "="*40)


if __name__ == "__main__":
    main()