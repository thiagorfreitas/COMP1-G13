// Teste Estruturado Erro

int main() { // Nota: main() não é realmente suportado pela gramática atual
    int valor1
    float valor2;
    int i;

    valor1 = 10 // Erro: Falta ponto e vírgula
    valor2 = 5.5;

    printf("Valores: %d, %f\n", valor1, valor2);

    // Erro: Variável 'j' não declarada
    for (i = 0; i < valor1; i = i + 1) {
        j = i * 2; 
        printf("Iteração: %d, Dobro: %d\n", i, j);
    }

    // Erro: Atribuição incompatível (string para int)
    valor1 = "Erro"; 

    // Erro: Tipos incompatíveis na condição do IF (comparando float com string?)
    // A gramática atual pode não pegar isso, mas é um erro semântico
    if (valor2 == "5.5") { 
        printf("Comparação estranha passou.\n");
    }

    // Erro: Falta chave de fechamento do main
// }

