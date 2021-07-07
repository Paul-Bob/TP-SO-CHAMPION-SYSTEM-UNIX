#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <setjmp.h>
#include <linux/limits.h>
#include <dirent.h>
#include <errno.h>

#include "games.h"

int GameOver = 0;
jmp_buf here;

void output(int score, char valid);

int main(int argc, char** argv){
	setbuf(stdout,NULL);
	char palavra[MAX_LETRAS],c,valida='w',gamedir[PATH_MAX], ficheiro[PATH_MAX], input[50];
	int seconds = 0, pontuacao = 0, w8 = 0;
	char *ptr;
	long resposta = 0;

	//variavel de ambiente GAMEDIR
	if(getGAMEDIR(gamedir) == -1)
		return(EXIT_FAILURE);

	sprintf(ficheiro,"%s/palavras.txt",gamedir);

	struct sigaction new_action;

	new_action.sa_flags = SA_SIGINFO | SA_RESTART;
	new_action.sa_sigaction = &trataSIGUSR1;

	sigfillset(&new_action.sa_mask);

	if (sigaction(SIGUSR1, &new_action, NULL) == -1){
		perror("Error: cannot handle SIGUSR1"); // não deve acontecer
		return EXIT_FAILURE;
	}

	FILE *f;
	f = fopen(ficheiro, "r"); //alterar para estar de acordo com a gamedir
	if (f == NULL){
		printf("Erro no acesso ao ficheiro!\n");
		return EXIT_FAILURE;
	}

 	sigsetjmp(here,1); //Vem para aqui quando o pai manda sinal, volta para scanf caso o sinal nao seja do pai (evita intrusos)
 	while (!GameOver){
 		output(pontuacao,valida);
 		valida = 'x';
 		fscanf(f, "%s", palavra);
 		printf("\nPalavra -> %s\n", palavra);
 		printf("Letras  -> ");
 		
 		if(!scanf("%s", input)){
 			valida = 'I';
 			while ((c = getchar()) != '\n' && c != EOF);
 			continue;
 		}

 		resposta = strtol(input, &ptr, 10);

 		if(*ptr != '\0'){
 			valida = 'I';
 			while ((c = getchar()) != '\n' && c != EOF);
 			continue;
 		}

 		if(resposta == strlen(palavra) && !GameOver){
 			pontuacao ++;
 			valida = 'V';
 		}

 		if(feof(f)){
 			printf("\nInfelizmente nao existem mais palavras disponiveis para continuar o jogo.\n");
 			break;
 		}
 	}

 	fclose(f);

	return pontuacao;
}


void trataSIGUSR1(int sig, siginfo_t *info, void *extra){
	if (info->si_pid == getppid()){ // apenas acaba o jogo caso o sinal tenha sido enviado pelo arbitro
		GameOver = 1;
		siglongjmp(here,1);		//ignora o scanf() , resolve este problema
	}                           //https://stackoverflow.com/questions/65206457/is-there-any-way-to-change-sigaction-flags-during-execution/65206589#65206589
}

void output(int score, char valid){
	printf(ESC "[2J");        //  clear the screen 
	printf(ESC "[H");         //  position cursor at top-left corner
	puts("*******************************************************************");
	puts("*                       Conta Letras                              *");
	puts("* Varias palavras irao surgir no ecra.                            *");
	puts("* O seu objetivo e responder corretamente com o numero de letras. *");
	puts("* Por cada resposta correta ganha um ponto.                       *");
	puts("* Respostas erradas nao tem nenhum efeito sobre a pontuacao.      *");
	puts("*                                                                 *");
	puts("* Exemplo:                                                        *");
	puts("* Palavra -> Laranja                                              *");
	puts("* Letras  -> Aqui insere a sua resposta.                          *");
	puts("* Caso a sua resposta fosse 7 ganharia um ponto!                  *");
	puts("*                                                                 *");
	puts("* A sua pontuacao apenas e apresentada no final do jogo.          *");
	puts("* O jogo acaba quando o tempo pre-definido acabar!                *");
	puts("*                                                                 *");
	puts("* Bom Jogo!!!                                                     *");
	puts("*******************************************************************");
	printf("* Pontuação acumulada: %3d *",score); //L 18 C 22
	switch(valid){
		case 'w':
			printf("\n* Ultima Resposta:         *\n");
			puts("****************************");
			break;
		case 'V':
			printf("\n* Ultima Resposta: Certa   *\n");
			puts("****************************");
			break;
		case 'x':
			printf("\n* Ultima Resposta: Errada  *\n");
			puts("****************************");
			break;
		case 'I':
			puts("                Atenção!              *");
			printf("* Ultima Resposta: Errada  *  Apenas digitos são valores Válidos! *\n");
			puts("*******************************************************************");
			break;
	}

}


/*void gereExecucao(int sig, siginfo_t *info, void *extra){ // não vai funcionar nos outros arbitros :X
	int int_val = info->si_value.sival_int;

	switch(int_val){
	case 0: 
		puts("\nAcabou o tempo!");
		GameOver = 1;
		break;
	case 1:
		puts("O campeonato irá ter inicio em");
		for(int i = 3; i >= 0; i--){
			printf("%d !\n",i);
			sleep(1);
		}
		puts("Começou!");
		break;
	case 2:
		puts("\nArbitro interrompeu o campeonato!");
		GameOver = 1;
		break;
	}
}*/
