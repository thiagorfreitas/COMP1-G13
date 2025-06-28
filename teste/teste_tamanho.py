def test_large_code(self):
    input_code = "int main() {"
    input_code += " int a = 0;" * 10000  # Repetindo uma linha de código muitas vezes
    input_code += " return 0; }"
    
    result = self.run_parser(input_code)
    
    # Verificando se o compilador consegue lidar com códigos grandes
    self.assertEqual(result.returncode, 0, "O compilador não conseguiu lidar com um código muito grande.")
