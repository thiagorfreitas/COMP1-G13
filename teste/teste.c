int main(){
  int idade;
  int d;
  idade = 25;
  d = 5;

  printf("Idade: %d %d", idade,d);

  if (d < 10) {
    idade = 1;
  };

  if (idade < 30) {
      idade = idade + 1;
  };

  while (idade < 30) 
  {
    printf("Idade: %d\n", idade);
    idade = idade + 1;
  };

  do{
    printf("Idade: %d\n", idade);
    idade = idade + 1;
  } while (idade < 35);
}