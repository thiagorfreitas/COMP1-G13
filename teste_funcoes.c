// Exemplo de código C com main e função fatorial
// ATENÇÃO: O compilador atual NÃO suporta definição ou chamada de funções.
// Este código causará erros se usado com a versão atual do compilador.

int fatorial(int n) {
    int res;
    if (n <= 1) {
        res = 1;
    } else {
        // Chamada recursiva não suportada!
        // res = n * fatorial(n - 1); 
        // Implementação iterativa para evitar recursão (ainda não suportada)
        res = 1;
        while (n > 1) {
            res = res * n;
            n = n - 1;
        }
    }
    return res; // Retorno de função não suportado!
}

int main() {
    int x;
    int y;
    
    x = 5; 
    // Chamada de função não suportada!
    // y = fatorial(x); 
    
    // Simulação simples sem chamada de função
    y = 120; // Valor esperado de fatorial(5)
    
    if (y > 100) {
        x = 1;
    } else {
        x = 0;
    }
    
    // printf("Fatorial: %d, Flag: %d\n"); // Printf não suportado
    
    return 0; // Retorno de função não suportado!
}

