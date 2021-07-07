#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

#include "comandos.h"

void comandoEspecial(char* comando, char* nome){
	char* comandosEspeciais = "KkSsRr"; //	CHARS dos comandos "especiais"
	char* aux = comando;				//	auxiliar para não mexer no ponteiro do comando

	if(strcasecmp(comando,"show") == 0 || strcasecmp(comando,"start") == 0) //	"show" por si é um comando, não pedido no enunciado, implementei por questões de debugg
		return;							//	irei proibir o nome "how" para não haver nehum jogador imune aos comandos especiais.

	while(*comandosEspeciais){          	   //	percorre todos os chars de comandos "especiais"
		if (*comando == *comandosEspeciais++){ //	caso o comando começa com K,k,S,s,R ou r
			aux++;							   //	o auxiliar fica a apontar para a segunda letra do comando
			while(*aux)					   	   //	enquanto não encontra o '\0'
				*nome++ = *aux++;			   //	vai copiando letra a letra o resto do comando para a variavel nome
			comando[1] = 0;					   //	comando fica reduzido apenas a sua primeira letra K,k,S,s,R ou r
		}
	}				   
}


//interpreta linha escrita no arbitro pelo administrador retorna 1 quando for para encerrar
int interpretaAdmin(char* linha,arbitro *server){ 
	int pontuacao;
	char comando[MAX_COMANDO] = "", *argumento=NULL, *valor=NULL, nome[MAX_NAME] = "", * obter;  //banir nome how do jogo mais tarde devido ao show
	int change = 0;
	RespostaArbitro resposta;
	pJogador jogador = NULL;
	union sigval sig_value;
	sig_value.sival_int = 94357104;
	char *extra = NULL;
	int tempo = 0;
	struct timeval  tv2;
	resposta.erro=0;

	//obtem primeira palavra da linha inserida e coloca na variavel comando
	obter = strtok(linha," ");
	strcpySegura(comando,obter,15,strlen(obter));

	//verifica se é algum comando especial ('s','r','k' etc...) e faz as devidas alterações
	comandoEspecial(comando, nome);


	switchs(comando) {
//-------------------------------------------------------------------------------------------------------------------------
		icases("teste")

		break;
			icases("help")

				if(argumento = strtok(NULL," "))
				{
					printf("%s: extra operand '%s' \n\n",comando,argumento);
					break;
				}

				puts("Comandos disponíveis:");
				puts("players - Listar jogadores em jogo (nome e jogo atribuído). ");
				puts("games   - Listar jogos disponíveis.");
				puts("k<nome> - Remover um jogador do campeonato. EX: “krui” – remove jogador “rui”");
				puts("s<nome> - Suspender a comunicação entre jogador e jogo. ");
				puts("r<nome> - Retomar a comunicação entre jogador e jogo. ");
				puts("end     - Encerrar o campeonato imediatamente. ");
				puts("exit    - Sair, encerrando o árbitro.");
				puts("\nComandos extra:");
				puts("start   - Interrompe o tempo de espera e começa o campeonato!");
				puts("change <argumento> <valor>\n           d  -> altera duracao do campeonato para 'valor' segundos.");
				puts("           e  -> altera tempo de espera por jogadores para 'valor' segundos.");
				puts("show   <argumento> \n           d  -> mostra duracao do campeonato.");
				puts("           e  -> mostra tempo de espera por jogadores.");
				puts("           t  -> mostra tempo restante do campeonato atual.");
				puts("atribuir <nomeJogo> <jogador> - Atribui o jogo passado pelo nome ao jogador.\n");
				break;

//-------------------------------------------------------------------------------------------------------------------------
			icases("players")

				if(argumento = strtok(NULL," "))
				{
					printf("%s: extra operand '%s' \n\n",comando,argumento);
					break;
				}

				
				listaJogadores(server->listaJogadores);


				break;

//-------------------------------------------------------------------------------------------------------------------------
			icases("games")

				if(argumento = strtok(NULL," "))
				{
					printf("%s: extra operand '%s' \n\n",comando,argumento);
					break;
				}

				listaJogos(server->listaJogos);

				break;

//-------------------------------------------------------------------------------------------------------------------------
			icases("atribuir")

				if(!(argumento = strtok(NULL," ")))
				{
					printf("%s: game name required\n\n",comando);
					break;
				}

				if(!(extra = strtok(NULL," ")))
				{
					printf("%s %s: player name required\n\n",comando, argumento);
					break;
				}

				jogador = getJogadorByName(server->listaJogadores,extra);

				if (jogador == NULL)
				{
					printf("Jogador '%s' não tem sessão ativa!\n\n",nome);
					break;
				}

				atribuicaoForcada(&jogador, server->listaJogos, argumento);


				break;

//-------------------------------------------------------------------------------------------------------------------------
			icases("k")

				if(argumento = strtok(NULL," "))
				{
					printf("%s: extra operand '%s' \n",comando,argumento);
					sprintf(comando,"%s%s",comando,argumento);
					printf("Try '%s'\n\n",comando);
					break;
				}

				jogador = getJogadorByName(server->listaJogadores,nome);

				if (jogador == NULL)
				{
					printf("Jogador '%s' não tem sessão ativa!\n\n",nome);
					break;
				}

				sprintf(resposta.mensagem, "[ARBITRO]: Infelizmente foste eliminado pelo administrador do campeonato!\n");
				write(jogador->clientpipe_fd, &resposta, sizeof(resposta));

				sleep(0.1); // apenas para dar tempo de o utilizador ser informado antes de lhe manda o sinal...

				jogador->estado = 5;

				//caso o campeonato esteja a decorrer eliminamos a thread do cliente
				if (server->flagCampeonato == 1)
				{
					pthread_kill(jogador->threadResponde, SIGUSR1);
					sigqueue(jogador->pidJogo   ,10,sig_value);
					pthread_join(jogador->threadResponde,NULL);
				}

				server->listaJogadores = eliminaJogador(server->listaJogadores,jogador->pidJogador);

				//caso estamos no limite minimo de jogadores, a thread que gere o campeonato é cancelada.
				if (contaJogadores(server->listaJogadores) == 1)
				{
					setEstadoDeTodos(4,server->listaJogadores);
					pthread_cancel(server->threadCampeonato);
					if(server->flagCampeonato == 3)
					{
						server->flagCampeonato = 0;
						puts("Ficou apenas um jogador, tempo de espera cancelado!\n");
						difusao("O campeonato foi adiado, espere por novas informações!",server->listaJogadores,0);
					}
					else
					{
						pthread_kill(server->listaJogadores->threadResponde, SIGUSR1);
						sigqueue(server->listaJogadores->pidJogo   ,10,sig_value);
						pthread_join(server->listaJogadores->threadResponde,NULL);
						difusao("Os teus concorrentes desistiram, és o vencedor!",server->listaJogadores,-4);
						server->listaJogadores = libertaListaJogadores(server->listaJogadores);
						server->flagCampeonato = 0;
						while(server->quantos)
							sigqueue(server->avisaFim[--server->quantos],10,sig_value);
					}
				}

				break;

//-------------------------------------------------------------------------------------------------------------------------
			icases("s")

				if(argumento = strtok(NULL," "))
				{
					printf("%s: extra operand '%s' \n",comando,argumento);
					sprintf(comando,"%s%s",comando,argumento);
					printf("Try '%s'\n\n",comando);
					break;
				}

				jogador = getJogadorByName(server->listaJogadores,nome);

				if (jogador == NULL)
				{
					printf("Jogador '%s' não tem sessão ativa!\n\n",nome);
					break;
				}

				if (server->flagCampeonato != 1)
				{
					puts("Não é possivel suspender jogadores se o campeonato não está a decorrer...\n");
					break;
				}

				if (jogador->estado == 1)
				{
					printf("Jogador '%s' já está suspenso!\n\n", nome);
					break;
				}

				jogador->estado = 1;

				sprintf(resposta.mensagem, "\n\n[ARBITRO]: Foste suspenso do campeonato!\n\n");
				write(jogador->clientpipe_fd, &resposta, sizeof(resposta));

				printf("Jogador '%s' suspenso!\n\n",nome);

				break;

//-------------------------------------------------------------------------------------------------------------------------
			icases("r")

				if(argumento = strtok(NULL," "))
				{
					printf("%s: extra operand '%s' \n",comando,argumento);
					sprintf(comando,"%s%s",comando,argumento);
					printf("Try '%s'\n\n",comando);
					break;
				}

				jogador = getJogadorByName(server->listaJogadores,nome);

				if (jogador == NULL)
				{
					printf("Jogador '%s' não tem sessão ativa!\n\n",nome);
					break;
				}

				if (jogador->estado != 1)
				{
					printf("Jogador '%s' não está suspenso!\n\n", nome);
					break;
				}

				if 		(server->flagCampeonato == 0) jogador->estado = 2;
				else if (server->flagCampeonato == 1) jogador->estado = 0;
				else if (server->flagCampeonato == 2) jogador->estado = 3;

				if (!jogador->estado)
				{
					sprintf(resposta.mensagem, "[ARBITRO]: Conexão retmoada ao campeonato!\n");
					write(jogador->clientpipe_fd, &resposta, sizeof(resposta));
				}

				printf("Jogador '%s' suspensão retirada!\n\n",nome);

				break;

//-------------------------------------------------------------------------------------------------------------------------
			icases("end")

				if(argumento = strtok(NULL," "))
				{
					printf("%s: extra operand '%s' \n\n",comando,argumento);
					break;
				}

				if(server->flagCampeonato == 1)
					pthread_kill(server->threadCampeonato, SIGUSR1);
				else
					printf("Não pode terminar um campeonato que ainda não começou!\n");

				if(server->flagCampeonato == 3)
					printf("Try 'start'\nstart   - Interrompe o tempo de espera e começa o campeonato!\n");
				putchar('\n');
				break;

//-------------------------------------------------------------------------------------------------------------------------
			icases("start")

				if(argumento = strtok(NULL," "))
				{
					printf("%s: extra operand '%s' \n\n",comando,argumento);
					break;
				}

				if(server->flagCampeonato == 3)
					pthread_kill(server->threadCampeonato, SIGUSR1);
				else 
					printf("Não pode usar este comando nesta altura do campeonato =)\n\n");

				break;

//-------------------------------------------------------------------------------------------------------------------------				
			icases("show")

				if(!(argumento = strtok(NULL," ")))
				{
					printf("%s: argument required\n\n",comando);
					break;
				}

				switchs(argumento){
			//--------------------------------------------------------------------
					icases("d")
						printf("Duração: %d segundos\n\n", server->DURACAO);
						break;

			//--------------------------------------------------------------------
					icases("e")
						printf("Tempo de espera: %d segundos\n\n", server->ESPERA);
						break;

			//--------------------------------------------------------------------
					icases("t")
						gettimeofday(&tv2, NULL);
						tempo = ((int) (tv2.tv_usec - server->tv3.tv_usec) / 1000000 + (int) (tv2.tv_sec - server->tv3.tv_sec));
						if (server->flagCampeonato == 1)
							printf("Faltam %d segundos até ao fim deste campeonato!\n\n", server->DURACAO - tempo);
						else
							puts("Não existe nenhum campeonato a decorrer!\n");
						break;

			//--------------------------------------------------------------------
					defaults
						printf("show: %s invalid argument\n\n",argumento);
						break;


				} switchs_end;

				break;

//-------------------------------------------------------------------------------------------------------------------------				
			icases("change")

				if(!(argumento = strtok(NULL," ")))
				{
					printf("%s: argument required\n\n",comando);
					break;
				}

				switchs(argumento){
			//--------------------------------------------------------------------
					icases("d")
						if(server->flagCampeonato == 1)
						{
							printf("Não pode alterar a duração do campeonato enquanto este está a decorrer!\n\n");
							break;
						}

						if(!(valor = strtok(NULL," ")))
						{
							printf("%s %s: value required\n\n",comando,argumento);
							break;
						}

						change = strtol(valor, &extra, 10);

						if(strlen(extra) != 0)
						{
							printf("%s %s %d: %s is extra\nTry: '%s %s %d'\n\n",comando,argumento,change,extra,comando,argumento,change);
							break;
						}

						if((extra = strtok(NULL," ")))
						{
							printf("%s %s %d: %s is extra\nTry: '%s %s %d'\n\n",comando,argumento,change,extra,comando,argumento,change);
							break;
						}

						if (change <= 0)
						{
							printf("Apenas valores positivos e não nulos são validos!\n\n");
							break;
						}

						server->DURACAO = change;

						sprintf(resposta.mensagem, "Duração do campeonato alterada!! Duração: %d\n", server->DURACAO);
						difusao(resposta.mensagem,server->listaJogadores,0);

						break;

			//--------------------------------------------------------------------
					icases("e")
						if(server->flagCampeonato == 1 || server->flagCampeonato == 3)
						{
							printf("Não pode alterar o tempo de espera do campeonato enquanto este está a decorrer!\n\n");
							break;
						}

						if(!(valor = strtok(NULL," ")))
						{
							printf("%s %s: value required\n\n",comando,argumento);
							break;
						}

						change = strtol(valor, &extra, 10);

						if(strlen(extra) != 0)
						{
							printf("%s %s %d: %s is extra\nTry: '%s %s %d'\n\n",comando,argumento,change,extra,comando,argumento,change);
							break;
						}

						if((extra = strtok(NULL," ")))
						{
							printf("%s %s %d: %s is extra\nTry: '%s %s %d'\n\n",comando,argumento,change,extra,comando,argumento,change);
							break;
						}

						if (change < 0)
						{
							printf("Apenas valores positivos são validos!\n\n");
							break;
						}

						server->ESPERA = change;

						sprintf(resposta.mensagem, "[ARBITRO]: Tempo de espera por outros jogadores alterado!! Espera minima: %d\n", server->ESPERA);
						jogador = server->listaJogadores;
						while(jogador)
							write(jogador->clientpipe_fd, &resposta, sizeof(resposta));

						break;

			//--------------------------------------------------------------------
					defaults
						printf("change: %s invalid argument\n\n",argumento);
						break;


				} switchs_end;

				break;

//-------------------------------------------------------------------------------------------------------------------------
			icases("exit")
			{
				difusao("Encerrei conexão! Té logoo!!\n",server->listaJogadores,0);
				sleep(0.2);
				jogador = server->listaJogadores;
				while(jogador)
				{
					if (jogador->pidJogo != -1 && server->flagCampeonato == 1)
					{
						pthread_kill(jogador->threadResponde, SIGUSR1);
						sigqueue(jogador->pidJogo   ,10,sig_value);
						pthread_join(jogador->threadResponde,NULL);
					}
					sigqueue(jogador->pidJogador,12,sig_value);
					jogador = jogador->prox;
				}

				return 1;
			}
			defaults
				printf("arbitro: %s: command not found\n\n",strtok(linha," "));
				break;
		} switchs_end;
		return 0;
}

//Comandos do jogador....
void interpretaJogador(PedidoJogador pedido, arbitro *server, RespostaArbitro *resposta){
	char *comando;
	pJogador jogador = server->listaJogadores;
	union sigval sig_value;
	sig_value.sival_int = 94357104;

	comando = strtok(pedido.mensagem," ");

	while(jogador != NULL && jogador->pidJogador != pedido.dados.pidJogador)
		jogador = jogador->prox;

	if (jogador == NULL)
	{
		sprintf(resposta->mensagem,"O teu nome não consta do campeonato, chame o administrador!\n");
		return;
	}

	switchs(comando){
		icases("#mygame")
			printf("%s: #mygame!\n\n", jogador->nome);
			sprintf(resposta->mensagem,"O teu jogo é o %s\n", jogador->jogoAtribuido);
			break;

		icases("#quit")
			printf("%s: #quit!\n\n", jogador->nome);
			jogador->estado = 4;

			//caso o campeonato esteja a decorrer eliminamos a thread do cliente
			if (server->flagCampeonato == 1 && jogador->pidJogo != -1)
			{
				pthread_kill(jogador->threadResponde, SIGUSR1);
				sigqueue(jogador->pidJogo   ,10,sig_value);
				pthread_join(jogador->threadResponde,NULL);
			}

			server->listaJogadores = eliminaJogador(server->listaJogadores,jogador->pidJogador);

			//caso estamos no limite minimo de jogadores, a thread que gere o campeonato é cancelada.
			if (contaJogadores(server->listaJogadores) == 1)
			{
				setEstadoDeTodos(4,server->listaJogadores);
				pthread_cancel(server->threadCampeonato);
				if(server->flagCampeonato == 3)
				{
					server->flagCampeonato = 0;
					puts("Ficou apenas um jogador, tempo de espera cancelado!\n");
					difusao("O campeonato foi adiado, espere por novas informações!",server->listaJogadores,0);
				}
				else
				{
					pthread_kill(server->listaJogadores->threadResponde, SIGUSR1);
					sigqueue(server->listaJogadores->pidJogo   ,10,sig_value);
					pthread_join(server->listaJogadores->threadResponde,NULL);
					difusao("Os teus concorrentes desistiram, és o vencedor!",server->listaJogadores,-4);
					server->listaJogadores = libertaListaJogadores(server->listaJogadores);
					server->flagCampeonato = 0;
					while(server->quantos)
						sigqueue(server->avisaFim[--server->quantos],10,sig_value);
				}
			}
			break;
	} switchs_end;
}