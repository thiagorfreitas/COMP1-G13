import sys
import re
from collections import defaultdict

# --- Quadruple Parsing ---
def parse_quad_line(line, line_num):
    """Parses a single line of quadruple text output, adding line number."""
    line = line.strip()
    # Match lines like "  0: ASSIGN x = 5"
    match = re.match(r"^\s*(\d+):\s*(.*)$", line)
    if not match:
        # Try matching lines like "LABEL L0:"
        match_label = re.match(r"^\s*LABEL\s+(\w+):", line)
        if match_label:
            label_name = match_label.group(1)
            return {"line_num": line_num, "original": line, "opcode": "LABEL", "args": [label_name], "result": None}
        # print(f"Warning: Could not parse line structure: {line}", file=sys.stderr)
        return None
    
    quad_num = int(match.group(1))
    content = match.group(2).strip()
    parts = content.split()
    
    if not parts:
        return None
        
    opcode_str = parts[0]
    args = []
    result = None

    # Heuristic parsing (needs improvement for robustness)
    try:
        if opcode_str == "ASSIGN": # RESULT = ARG1
            result = parts[1]
            args.append(parts[3])
        elif opcode_str in ["ADD", "SUB", "MUL", "DIV", "MOD"]: # RESULT = ARG1 OP ARG2
            result = parts[1]
            args.append(parts[3])
            args.append(parts[5])
        elif opcode_str == "UMINUS": # RESULT = - ARG1
            result = parts[1]
            args.append(parts[4])
        elif opcode_str == "PRINT": # PRINT ARG1
            args.append(parts[1])
        elif opcode_str == "LABEL": # LABEL ARG1:
             # Already handled by regex above, but keep for consistency if format changes
            args.append(parts[1].replace(":", ""))
        elif opcode_str == "GOTO": # GOTO ARG1
            args.append(parts[1])
        elif opcode_str.startswith("IF_"): # IF_OP ARG1 OP ARG2 GOTO RESULT
            args.append(parts[1]) # arg1
            args.append(parts[3]) # arg2
            result = parts[5] # The destination label is the 'result' here
        else:
            # print(f"Warning: Unrecognized opcode format for {opcode_str} in line: {line}", file=sys.stderr)
            return None
            
    except IndexError:
        # print(f"Warning: IndexError parsing line: {line}", file=sys.stderr)
        return None

    return {"line_num": line_num, "original": line, "opcode": opcode_str, "args": args, "result": result}

# --- Python Code Generation ---

def format_operand(operand):
    """Formats operand for Python code."""
    if operand is None:
        return "None" # Should not happen often in valid quads
    if isinstance(operand, str):
        if operand.startswith("\"") and operand.endswith("\""):
            return operand # Keep strings as is
        # Basic check for numbers (int/float)
        if re.fullmatch(r"-?\d+", operand) or re.fullmatch(r"-?\d+\.\d+", operand):
            return operand
        # Assume variable/temp otherwise
        return operand
    return str(operand) # Fallback

def get_comparison_op(opcode):
    """Maps IF_OP opcode to Python comparison operator."""
    op_map = {"IF_EQ": "==", "IF_NEQ": "!=", "IF_LT": "<", "IF_GT": ">", "IF_LEQ": "<=", "IF_GEQ": ">="}
    return op_map.get(opcode)

def translate_quads_to_python(quad_lines):
    """Translates quadruple strings into Python code lines with basic control flow."""
    
    parsed_quads = [parse_quad_line(line, i) for i, line in enumerate(quad_lines)]
    parsed_quads = [q for q in parsed_quads if q] # Filter out None results

    if not parsed_quads:
        return ["# No valid quadruples found"]

    python_code = []
    indent_level = 0
    label_to_line = {q["args"][0]: q["line_num"] for q in parsed_quads if q["opcode"] == "LABEL"}
    line_to_label = {v: k for k, v in label_to_line.items()}
    
    # Basic block identification (very simplified)
    leaders = {0} # First instruction is a leader
    for i, q in enumerate(parsed_quads):
        if q["opcode"] == "LABEL":
            leaders.add(i)
        if q["opcode"].startswith("IF_") or q["opcode"] == "GOTO":
            leaders.add(i + 1) # Instruction after jump is a leader
            target_label = q["result"] if q["opcode"].startswith("IF_") else q["args"][0]
            if target_label in label_to_line:
                 leaders.add(label_to_line[target_label]) # Target of jump is a leader
                 
    leaders = sorted(list(leaders))
    basic_blocks = []
    for i in range(len(leaders)):
        start = leaders[i]
        end = leaders[i+1] if i+1 < len(leaders) else len(parsed_quads)
        # Ensure block is not empty and start is within bounds
        if start < end and start < len(parsed_quads):
             basic_blocks.append(parsed_quads[start:end])

    # --- Translation Pass --- 
    # This part is still complex and needs a proper CFG analysis for robust translation.
    # The following is a simplified attempt and likely won't handle all cases correctly,
    # especially nested loops or complex conditions.
    
    processed_lines = set()
    current_line_index = 0
    
    # Add a marker for the start
    python_code.append("# Start of generated code")
    python_code.append("import sys # Added for potential runtime needs")
    python_code.append("") # Blank line

    # --- Very Basic Translation (No real control flow reconstruction) ---
    # This just translates instructions linearly and adds comments for jumps/labels
    # A real translator needs a Control Flow Graph (CFG)
    
    for i, quad in enumerate(parsed_quads):
        opcode = quad["opcode"]
        args = quad["args"]
        result = quad["result"]
        line_py = ""
        indent_str = "    " * indent_level

        # Check if this line starts a known label
        if i in line_to_label:
             # Simple comment for label, real handling is complex
             python_code.append(f"{indent_str}# --- LABEL {line_to_label[i]} ---")

        if opcode == "ASSIGN":
            line_py = f"{indent_str}{format_operand(result)} = {format_operand(args[0])}"
        elif opcode == "ADD":
            line_py = f"{indent_str}{format_operand(result)} = {format_operand(args[0])} + {format_operand(args[1])}"
        elif opcode == "SUB":
            line_py = f"{indent_str}{format_operand(result)} = {format_operand(args[0])} - {format_operand(args[1])}"
        elif opcode == "MUL":
            line_py = f"{indent_str}{format_operand(result)} = {format_operand(args[0])} * {format_operand(args[1])}"
        elif opcode == "DIV": 
            line_py = f"{indent_str}{format_operand(result)} = {format_operand(args[0])} // {format_operand(args[1])} # Using integer division"
        elif opcode == "MOD":
            line_py = f"{indent_str}{format_operand(result)} = {format_operand(args[0])} % {format_operand(args[1])}"
        elif opcode == "UMINUS":
            line_py = f"{indent_str}{format_operand(result)} = -{format_operand(args[0])}"
        elif opcode == "PRINT":
            line_py = f"{indent_str}print({format_operand(args[0])})"
        elif opcode == "LABEL":
            # Handled above by adding a comment marker
            continue # Don't generate a separate line for the quad itself
        elif opcode == "GOTO":
            line_py = f"{indent_str}# GOTO {args[0]} # Control flow not fully implemented"
            # In a real implementation, this would affect the next instruction processed
        elif opcode.startswith("IF_"):
            comp_op = get_comparison_op(opcode)
            if comp_op:
                 # Basic IF structure - This is highly simplified!
                 # It doesn't handle block structure or else/elif correctly.
                 condition = f"{format_operand(args[0])} {comp_op} {format_operand(args[1])}"
                 line_py = f"{indent_str}# IF {condition} GOTO {result} # Control flow not fully implemented"
                 # A real implementation would likely generate an `if condition:` block
                 # and potentially use `continue` or `break` if inside a loop, or structure
                 # the code based on the target label.
            else:
                 original_quad_str = quad["original"]
                 line_py = f"{indent_str}# Unknown IF condition: {original_quad_str}"
        else:
            original_quad_str = quad["original"]
            line_py = f"{indent_str}# Quad not translated: {original_quad_str}"

        if line_py:
            python_code.append(line_py)

    python_code.append("") # Blank line
    python_code.append("# End of generated code")
    return python_code

# --- Main Execution ---
if __name__ == "__main__":
    if len(sys.argv) > 1:
        try:
            with open(sys.argv[1], 'r') as f:
                quad_input_lines = f.readlines()
        except FileNotFoundError:
            print(f"Error: Input file '{sys.argv[1]}' not found.", file=sys.stderr)
            sys.exit(1)
    else:
        # Read from stdin if no file argument
        quad_input_lines = sys.stdin.readlines()
    
    print("# --- Generated Python Code ---")
    python_output_lines = translate_quads_to_python(quad_input_lines)
    for py_line in python_output_lines:
        print(py_line)
    # print("# --- End Generated Code ---") # Redundant with the one inside the function

