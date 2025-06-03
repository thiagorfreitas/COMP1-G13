// Teste Estruturado OK

int main() { // Nota: main() não é realmente suportado pela gramática atual, mas a estrutura interna é válida
    int contador;
    int limite;
    float media;
    int soma;

    limite = 10;
    contador = 0;
    soma = 0;
    media = 0.0;

    printf("Iniciando loop até %d...\n", limite);

    while (contador < limite) {
        printf("Contador: %d\n", contador);
        soma = soma + contador;
        if (contador % 2 == 0) {
            int temp_par;
            temp_par = contador * 2;
            printf("  -> Par, dobro: %d\n", temp_par);
        } else {
            printf("  -> Impar\n");
        }
        contador = contador + 1;
    }

    if (limite > 0) {
        media = soma / limite; // Divisão inteira aqui, C padrão
        printf("Loop finalizado. Soma: %d, Media (int): %f\n", soma, media);
    } else {
        printf("Loop não executado (limite <= 0).\n");
    }

    // return 0; // return não suportado
}

