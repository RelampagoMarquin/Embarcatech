#include <stdio.h>
#include <math.h>

#define Pi 3.14159

double calc(double raio){
    double area;
    area = Pi + pow(raio, 2);
    printf("A area da circuferencia e: %.2f", area);
    return area;
}

int main(){
    double raio;
    printf("Digite o valor do raio: ");
    scanf("%lf", &raio);
    calc(raio);
    return 0;
}