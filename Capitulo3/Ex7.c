//incluindo a bibliteca stdio
#include <stdio.h>

void main() {
    char C = 'U';
    char D;

    D = C;  //Caractere D recebe conteudo de C
    printf("O caractere D armazena %d \n", D);
   
    D = !C; //Caractere D recebe negacao logica do conteudo de C
    printf("O caractere D armazena %d \n", D);

    D = !D; //Caractere D recebe negacao logica do seu conteudo anterior
    printf("O caractere D agora armazena %d\n\n", D);

}