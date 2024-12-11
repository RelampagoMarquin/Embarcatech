#include <stdio.h>
#include <math.h>

// Função para chamar o input
double callInput(char variavel) {
    double res;
    printf("Digite o valor de %c: ", variavel);
    if (scanf("%lf", &res) != 1) {
        printf("Erro ao ler o valor de %c.\n", variavel);
        res = 0; // Pode definir um valor padrão ou tratar o erro de outra forma.
    }
    return res;
}

double calcDelta(double a, double b, double c)
{
    double delta = (b * b) - 4 * a * c;
    return delta;
}

double calcX(double a, double b, double delta){
     if (a == 0) {
        printf("Não é uma equação de segundo grau.\n");
        // colocar apenas return gera conflito pois a funcao precisa de um returno double
        return 0;
    }
    if (delta > 0) {
        double x1 = (-b + sqrt(delta)) / (2 * a);
        double x2 = (-b - sqrt(delta)) / (2 * a);
        printf("As raízes são reais e distintas: x1 = %.2f, x2 = %.2f\n", x1, x2);
    } else if (delta == 0) {
        double x = -b / (2 * a);
        printf("As raízes são reais e iguais: x = %.2f\n", x);
    } else {
        double parteReal = -b / (2 * a);
        double parteImaginaria = sqrt(-delta) / (2 * a);
        printf("As raízes são complexas:\n");
        printf("x1 = %.2f + %.2fi\n", parteReal, parteImaginaria);
        printf("x2 = %.2f - %.2fi\n", parteReal, parteImaginaria);
    }
}

void itsReal(double delta){
    if (delta > 0) {
        printf("Delta é positivo (%.2f). As raízes são reais e distintas.\n", delta);
    } else if (delta < 0) {
        printf("Delta é negativo (%.2f). As raízes são complexas.\n", delta);
    } else {
        printf("Delta é zero (%.2f). As raízes são reais e iguais.\n", delta);
    }
}

int main(){
    // há possibilidade de criar mais de uma variavel do mesmo tipo
    double a, b, c, delta;
    int option;
    int count = 3;
    for (int i = 0; i < count; i++)
    {
        switch (i)
        {
        case 0:
            a = callInput('a');
            break;
        case 1:
            b = callInput('b');
            break;
        case 2:
            c = callInput('c');
            break;
        default:
            break;
        }
        
    }
    
    delta = calcDelta(a, b, c);
    itsReal(delta);

    printf("Deseja calcular o x (digite 0 para sim e 1 para nao) ? ");
    scanf("%lf", &option);

    if (option == 0){
        calcX(a, b, delta);
    }

    printf("finalizando");
    return 0;
}