int a;
float b;
int a; // Erro: Redeclaração
b = "texto"; // Erro: Atribuição incompatível
c = 10; // Erro: Variável 'c' não declarada
a = b + "outro texto"; // Erro: Tipos incompatíveis para '+'
if ("string_cond") { // Erro: Condição não numérica
  a = 1;
}

