def test_empty_code(self):
    input_code = ""  # código vazio
    result = self.run_parser(input_code)
    
    # Verificando se o compilador lida com a entrada vazia corretamente
    self.assertNotEqual(result.returncode, 0, "O compilador aceitou um código vazio.")
