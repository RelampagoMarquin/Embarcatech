#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int media(int vetor[], int tamanho);
int lerSensor();
int medicoes[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int main() {
    printf("Olá Embarcatech! Vamos executar as 10 medições: \n");
    // Define a semente para os números aleatórios
    srand(time(0));
    
    for (int i = 0; i < 10; i++) {
        medicoes[i] = lerSensor();
    }

    for (int i = 0; i < 10; i++) {
        printf("%d, ", medicoes[i]);
    }
    printf(" - Média: %d\n", media(medicoes, 10));
    return 0;
}

int media(int vetor[], int tamanho) {
    int soma = 0;
    for (int i = 0; i < tamanho; i++) {
        soma += vetor[i];
    }
    return soma / tamanho;
}

int lerSensor() {
    // Gera um número aleatório entre 25 e 35
    int numero = rand() % ((35 - 25) + 1) + 25; 
    return numero;
}