def test_syntax_error(self):
    input_code = """
    int main() {
        int a = 10;
        int b = 20;
        int c = a + ; // erro de sintaxe
        return c;
    }
    """
    result = self.run_parser(input_code)
    
    # Verificando se o compilador falhou com código não válido (erro de sintaxe)
    self.assertNotEqual(result.returncode, 0, "O compilador não detectou erro de sintaxe.")
