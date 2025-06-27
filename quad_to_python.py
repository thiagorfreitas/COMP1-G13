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

def invert_comparison_op(opcode):
    """Inverts the comparison operator for IF statements that jump to ELSE."""
    invert_map = {
        "IF_EQ": "!=", "IF_NEQ": "==", 
        "IF_LT": ">=", "IF_GT": "<=", 
        "IF_LEQ": ">", "IF_GEQ": "<"
    }
    return invert_map.get(opcode)

def detect_if_else_structure(parsed_quads, start_idx):
    """Detects IF-ELSE structure starting from an IF_ instruction."""
    if start_idx >= len(parsed_quads):
        return None
    
    if_quad = parsed_quads[start_idx]
    if not if_quad["opcode"].startswith("IF_"):
        return None
    
    else_label = if_quad["result"]  # Label to jump to for ELSE
    
    # Find the ELSE label position
    else_idx = None
    end_label = None
    end_idx = None
    
    for i in range(start_idx + 1, len(parsed_quads)):
        quad = parsed_quads[i]
        if quad["opcode"] == "LABEL" and quad["args"][0] == else_label:
            else_idx = i
            break
        elif quad["opcode"] == "GOTO":
            # This might be the GOTO to skip the ELSE block
            end_label = quad["args"][0]
    
    # Find the end label position
    if end_label:
        for i in range(start_idx + 1, len(parsed_quads)):
            quad = parsed_quads[i]
            if quad["opcode"] == "LABEL" and quad["args"][0] == end_label:
                end_idx = i
                break
    
    if else_idx is not None:
        return {
            "if_idx": start_idx,
            "else_idx": else_idx,
            "end_idx": end_idx,
            "else_label": else_label,
            "end_label": end_label
        }
    
    return None

def detect_while_structure(parsed_quads, start_idx):
    """Detects WHILE structure starting from a LABEL followed by IF_."""
    if start_idx >= len(parsed_quads):
        return None
    
    # Check if this is a LABEL that could start a while loop
    if parsed_quads[start_idx]["opcode"] != "LABEL":
        return None
    
    while_label = parsed_quads[start_idx]["args"][0]
    
    # Look for IF_ instruction that jumps out of the loop
    if_idx = None
    end_label = None
    goto_back_idx = None
    
    for i in range(start_idx + 1, len(parsed_quads)):
        quad = parsed_quads[i]
        if quad["opcode"].startswith("IF_"):
            if_idx = i
            end_label = quad["result"]  # Where to jump to exit the loop
            break
    
    if not if_idx:
        return None
    
    # Look for GOTO that jumps back to the beginning
    for i in range(if_idx + 1, len(parsed_quads)):
        quad = parsed_quads[i]
        if quad["opcode"] == "GOTO" and quad["args"][0] == while_label:
            goto_back_idx = i
            break
        elif quad["opcode"] == "LABEL" and quad["args"][0] == end_label:
            # Found the end label before GOTO, this confirms it's a while loop
            end_idx = i
            return {
                "label_idx": start_idx,
                "if_idx": if_idx,
                "goto_idx": goto_back_idx,
                "end_idx": end_idx,
                "while_label": while_label,
                "end_label": end_label
            }
    
    return None

def translate_quads_to_python(quad_lines):
    """Translates quadruple strings into Python code lines with control flow reconstruction."""
    
    parsed_quads = [parse_quad_line(line, i) for i, line in enumerate(quad_lines)]
    parsed_quads = [q for q in parsed_quads if q] # Filter out None results

    if not parsed_quads:
        return ["# No valid quadruples found"]

    python_code = []
    processed_indices = set()  # Track which quads we've already processed
    
    # Add a marker for the start
    python_code.append("# Start of generated code")
    python_code.append("import sys # Added for potential runtime needs")
    python_code.append("")  # Blank line after import

    i = 0
    while i < len(parsed_quads):
        if i in processed_indices:
            i += 1
            continue
            
        quad = parsed_quads[i]
        opcode = quad["opcode"]
        args = quad["args"]
        result = quad["result"]
        
        # Check for WHILE structure starting with LABEL
        if opcode == "LABEL":
            while_structure = detect_while_structure(parsed_quads, i)
            if while_structure:
                # Generate WHILE block
                if_quad = parsed_quads[while_structure["if_idx"]]
                
                # Invert the condition since IF jumps out of the loop
                inverted_op = invert_comparison_op(if_quad["opcode"])
                condition = f"{format_operand(if_quad['args'][0])} {inverted_op} {format_operand(if_quad['args'][1])}"
                
                python_code.append(f"while {condition}:")
                
                # Process loop body (instructions between IF and GOTO back)
                body_start = while_structure["if_idx"] + 1
                body_end = while_structure["goto_idx"] if while_structure["goto_idx"] else while_structure["end_idx"]
                
                for j in range(body_start, body_end):
                    if j < len(parsed_quads) and parsed_quads[j]["opcode"] not in ["GOTO", "LABEL"]:
                        line_py = translate_single_quad(parsed_quads[j], 1)
                        if line_py:
                            python_code.append(line_py)
                        processed_indices.add(j)
                
                # Mark all processed indices
                for j in range(while_structure["label_idx"], while_structure["end_idx"] + 1):
                    processed_indices.add(j)
                
                # Skip to after the WHILE structure
                i = while_structure["end_idx"] + 1
                continue
        
        # Check for IF-ELSE structure
        if opcode.startswith("IF_"):
            # First check if this is a while loop by looking for specific pattern
            # LABEL -> IF_ -> body -> GOTO back to LABEL
            is_while_loop = False
            if i > 0 and parsed_quads[i-1]["opcode"] == "LABEL":
                # Check if there's a GOTO that jumps back to the previous label
                prev_label = parsed_quads[i-1]["args"][0]
                for j in range(i+1, min(len(parsed_quads), i+10)):
                    if (parsed_quads[j]["opcode"] == "GOTO" and 
                        parsed_quads[j]["args"][0] == prev_label):
                        is_while_loop = True
                        break
            
            if is_while_loop:
                # This is a while loop
                inverted_op = invert_comparison_op(opcode)
                condition = f"{format_operand(args[0])} {inverted_op} {format_operand(args[1])}"
                
                python_code.append(f"while {condition}:")
                
                # Find the GOTO that jumps back
                goto_idx = None
                for j in range(i+1, len(parsed_quads)):
                    if parsed_quads[j]["opcode"] == "GOTO":
                        goto_idx = j
                        break
                
                # Process loop body (between IF and GOTO)
                if goto_idx:
                    for j in range(i+1, goto_idx):
                        if j < len(parsed_quads):
                            line_py = translate_single_quad(parsed_quads[j], 1)
                            if line_py:
                                python_code.append(line_py)
                            processed_indices.add(j)
                
                # Mark processed indices
                processed_indices.add(i-1)  # The LABEL
                processed_indices.add(i)    # The IF
                if goto_idx:
                    processed_indices.add(goto_idx)  # The GOTO
                
                # Find and mark the end label
                end_label = result  # The label IF jumps to
                for j in range(i+1, len(parsed_quads)):
                    if (parsed_quads[j]["opcode"] == "LABEL" and 
                        parsed_quads[j]["args"][0] == end_label):
                        processed_indices.add(j)
                        i = j + 1
                        break
                else:
                    i = goto_idx + 1 if goto_idx else i + 1
                continue
            else:
                # Regular IF-ELSE structure
                if_structure = detect_if_else_structure(parsed_quads, i)
                if if_structure:
                    # Generate IF-ELSE block
                    if_quad = parsed_quads[if_structure["if_idx"]]
                    
                    # Invert the condition since IF jumps to ELSE
                    inverted_op = invert_comparison_op(if_quad["opcode"])
                    condition = f"{format_operand(if_quad['args'][0])} {inverted_op} {format_operand(if_quad['args'][1])}"
                    
                    python_code.append(f"if {condition}:")
                    
                    # Process THEN block (instructions between IF and ELSE label)
                    then_start = if_structure["if_idx"] + 1
                    then_end = if_structure["else_idx"]
                    
                    for j in range(then_start, then_end):
                        if j < len(parsed_quads) and parsed_quads[j]["opcode"] != "GOTO":
                            line_py = translate_single_quad(parsed_quads[j], 1)
                            if line_py:
                                python_code.append(line_py)
                            processed_indices.add(j)
                    
                    # Process ELSE block if it exists
                    if if_structure["else_idx"] is not None:
                        python_code.append("else:")
                        
                        else_start = if_structure["else_idx"] + 1  # Skip the LABEL
                        else_end = if_structure["end_idx"] if if_structure["end_idx"] else len(parsed_quads)
                        
                        for j in range(else_start, else_end):
                            if j < len(parsed_quads):
                                line_py = translate_single_quad(parsed_quads[j], 1)
                                if line_py:
                                    python_code.append(line_py)
                                processed_indices.add(j)
                    
                    # Mark all processed indices
                    processed_indices.add(if_structure["if_idx"])
                    if if_structure["else_idx"]:
                        processed_indices.add(if_structure["else_idx"])
                    if if_structure["end_idx"]:
                        processed_indices.add(if_structure["end_idx"])
                    
                    # Skip to after the IF-ELSE structure
                    i = if_structure["end_idx"] + 1 if if_structure["end_idx"] else else_end
                    continue
                else:
                    # Fallback for unrecognized IF structure
                    comp_op = get_comparison_op(opcode)
                    if comp_op:
                        condition = f"{format_operand(args[0])} {comp_op} {format_operand(args[1])}"
                        python_code.append(f"# IF {condition} GOTO {result} # Control flow not fully implemented")
        
        # Handle other opcodes
        elif opcode == "LABEL":
            # Skip labels that are part of control structures
            pass
        elif opcode == "GOTO":
            # Skip GOTOs that are part of control structures
            pass
        else:
            # Translate regular instructions
            line_py = translate_single_quad(quad, 0)
            if line_py:
                python_code.append(line_py)
        
        processed_indices.add(i)
        i += 1

    python_code.append("") # Blank line
    python_code.append("# End of generated code")
    return python_code

def translate_single_quad(quad, indent_level):
    """Translates a single quadruple to Python code with specified indentation."""
    opcode = quad["opcode"]
    args = quad["args"]
    result = quad["result"]
    indent_str = "    " * indent_level

    if opcode == "ASSIGN":
        return f"{indent_str}{format_operand(result)} = {format_operand(args[0])}"
    elif opcode == "ADD":
        return f"{indent_str}{format_operand(result)} = {format_operand(args[0])} + {format_operand(args[1])}"
    elif opcode == "SUB":
        return f"{indent_str}{format_operand(result)} = {format_operand(args[0])} - {format_operand(args[1])}"
    elif opcode == "MUL":
        return f"{indent_str}{format_operand(result)} = {format_operand(args[0])} * {format_operand(args[1])}"
    elif opcode == "DIV": 
        return f"{indent_str}{format_operand(result)} = {format_operand(args[0])} // {format_operand(args[1])} # Using integer division"
    elif opcode == "MOD":
        return f"{indent_str}{format_operand(result)} = {format_operand(args[0])} % {format_operand(args[1])}"
    elif opcode == "UMINUS":
        return f"{indent_str}{format_operand(result)} = -{format_operand(args[0])}"
    elif opcode == "PRINT":
        orig = quad["original"]
        raw = orig.split("PRINT", 1)[1].strip()
        if ',' in raw:
            lit_raw, var = raw.split(",", 1)
            var = var.strip()
        else:
            lit_raw, var = raw, None

        inner = lit_raw.strip().strip('"')
        fmt = f'"{inner}"'

        if var:
            return f"{indent_str}print({fmt} % {var})"
        else:
            return f"{indent_str}print({fmt})"
    elif opcode == "LABEL" or opcode == "GOTO":
        # These are handled by control flow reconstruction
        return None
    else:
        return f"{indent_str}# Quad not translated: {quad['original']}"

# --- Main Execution ---
if __name__ == "__main__":
    if len(sys.argv) > 1:
        try:
            with open(sys.argv[1], 'r', encoding='utf-8') as f:
                quad_input_lines = f.readlines()
        except FileNotFoundError:
            print(f"Error: Input file '{sys.argv[1]}' not found.", file=sys.stderr)
            sys.exit(1)
        except UnicodeDecodeError:
            # Try with latin-1 encoding if utf-8 fails
            try:
                with open(sys.argv[1], 'r', encoding='latin-1') as f:
                    quad_input_lines = f.readlines()
            except Exception as e:
                print(f"Error reading file: {e}", file=sys.stderr)
                sys.exit(1)
    else:
        # Read from stdin if no file argument
        quad_input_lines = sys.stdin.readlines()
    
    print("# --- Generated Python Code ---")
    python_output_lines = translate_quads_to_python(quad_input_lines)
    for py_line in python_output_lines:
        print(py_line)
    # print("# --- End Generated Code ---") # Redundant with the one inside the function

