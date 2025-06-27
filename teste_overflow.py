def test_integer_overflow(self):
    input_code = """
    int main() {
        int a = 2147483647;  // valor máximo para um int em C
        int b = a + 1;  // Isso pode causar overflow
        return b;
    }
    """
    result = self.run_parser(input_code)
    
    # Verificando se o compilador detecta ou lida com overflow corretamente
    self.assertEqual(result.returncode, 0, "O compilador falhou ao lidar com overflow de inteiros.")
