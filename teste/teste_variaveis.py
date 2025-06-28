def test_large_number_of_variables(self):
    input_code = "int main() {"
    for i in range(1000):  # criando 1000 variáveis
        input_code += f" int var{i} = {i};"
    input_code += " return 0; }"
    
    result = self.run_parser(input_code)
    
    # Verificando se o compilador consegue lidar com o código
    self.assertEqual(result.returncode, 0, "O compilador não conseguiu lidar com um grande número de variáveis.")
