//Ficheiro auxiliar.c  **Nome original era aux.c mas quando tentei partilhar os ficheiros para o windows tive que alterar visto que o windows nao permite 
//esse nome de ficheiro... Caso esteja referido aux.c em mais alguma parte do codigo é porque me esqueci de ir alterar**
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "arbitro.h"
#include "auxiliar.h"

//obtem resposta Y y N ou n para as perguntas feitas quando faltam argumentos na inicialização do arbitro
int obtemResposta(void){
	char resposta = 'k';
	char extraResposta[50];

	puts("[Y/n]");

	while((resposta != 'y' && resposta != 'Y' && resposta != 'n' && resposta != 'N') || (strlen(extraResposta) != 0)){
		extraResposta[0] = '\0';
		scanf(" %c", &resposta);
		scanf("%[^\n]", extraResposta);}

	if(resposta == 'n' || resposta == 'N')
		return 0;

	return 1;
}

int intUniformRnd(int a, int b){  //Devolve um valor inteiro aleatorio distribuido uniformemente entre [a, b] 
    return a + rand()%(b-a+1);
}

void initRandom(){               // Inicializa o gerador de numeros aleatorios.Esta funcao deve ser chamada apenas uma vez no inicio da execucao do programa
    srand(time(NULL));
}

void printSintaxe(char* argv0){
	printf("Sintaxes:   %s \n",argv0);
	printf("            %s -d <segundos> \n",argv0);
	printf("            %s -e <segundos> \n",argv0);
	printf("            %s -d <segundos> -e <segundos> \n",argv0);
	printf("            %s -e <segundos> -d <segundos> \n",argv0);
	puts("Argumentos: -d --> duração do campeonato.");
	puts("            -e --> tempo de espera por jogadores.");
	puts("Caso inicie o arbitro sem argumentos ele inicia com os valor default!");
	printf(" -> Espera default:  %d\n",ESPERA_DEFAULT);
	printf(" -> Duração default: %d\n",DURACAO_DEFAULT);
}

void myabort(const char * msg, int exit_status) {
	perror(msg);
	exit(exit_status);
}

//strcpy sem problema dos tamanhos... Utilizada pelo arbitro
char* strcpySegura(char* destino,char* origem,int tamD,int tamO){ //copia strings de forma segura.
    if(tamD<tamO)
        return origem;
    for(int i=0; i<tamD; i++)
        destino[i] = origem[i];
	return destino;    
}


