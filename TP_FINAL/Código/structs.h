#ifndef STRUCTS_H
#define STRUCTS_H

#include <sys/types.h>
#include <time.h>
#include <linux/limits.h>

#define MAX_NAME 20
#define MAX_COMANDO 200

typedef struct game Jogo, *pJogo;
struct game{
	char nomeJogo[PATH_MAX];
	char path[PATH_MAX];
	pJogo prox;
};

typedef struct cliente Jogador, *pJogador;  //estrutura Jogador
struct cliente{
	char nome[MAX_NAME]; 					//nome do jogador
	pid_t pidJogador;						//pid do jogador
	int score;								//pontuação do jogador
	pJogador prox; 							//ponteiro para criação de listas ligadas
	pJogo jogoAtribuido;                    //ponteiro para o jogo atribuido
	pid_t pidJogo;                          //pid jogo atribuido
	int clientpipe_fd; 						//pipe do jogador
	int arbitroJogo_fd[2]; 					//pipe arbitro->jogo
	pthread_t threadResponde;				//thread que trata do jogadro
	int estado; //0 - Conectado e em jogo ativo 
};				//-1 conectado enquanto decorre cameponato
				// 1 - Suspenso 
				// 2 - conectado sem jogo ativo 
				// 3 - campeonato acabou nao consegue comunicar mais 
				// 4 - saiu 	
				// 5 - eliminei


typedef struct servidor arbitro;
struct servidor{
	pid_t pid;					//Pid arbitro
	int MAXPLAYER;				//Limite de jogadores
    long DURACAO; 				//Duração campeonato
    long ESPERA;				//Tempo de espera por jogadores
    char GAMEDIR[PATH_MAX];		//Diretoria de jogos
    pJogador listaJogadores; 	//criaçaõ da lista ligada de jogadores
    pJogo    listaJogos;     	//lista ligada de jogos
    pthread_t threadCampeonato;	//thread que gere o campeonato
    pid_t pidTeste;				//
    struct timeval  tv1, tv3;	//Calcular tempos, até final campeonato etc...
    pid_t avisaFim[30];			//pid's pendentes para registar
    int quantos;				//quantos estao pendetes
    int flagCampeonato; 		//0 - Ainda nao começou 
};								// 1 - A decorrer 
								// 2 - Acabou 
								// 3 - tempo de espera a decorrer


#endif