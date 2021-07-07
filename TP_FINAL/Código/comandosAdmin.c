#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>

#include "switchString.h" //codigo retirado do stackoverflow para poder utilizar o switchcase com strings (posso utilizar?!)
#include "arbitro.h"

void comandoEspecial(char* comando, char* nome){
	char* comandosEspeciais = "KkSsRr"; //	CHARS dos comandos "especiais"
	char* aux = comando;				//	auxiliar para não mexer no ponteiro do comando

	if(strcasecmp(comando,"show") == 0) //	"show" por si é um comando, não pedido no enunciado, implementei por questões de debugg
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
	char comando[MAX_COMANDO] = "", *argumento=NULL, nome[MAX_NAME] = "", * obter;  //banir nome how do jogo mais tarde devido ao show
	int status = 0;
	RespostaArbitro resposta;
	pJogador jogador = NULL;
	union sigval sig_value;
	sig_value.sival_int = 94357104;


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
					printf("%s: extra operand '%s' \n",comando,argumento);
					break;
				}

				puts("Comandos disponíveis:");
				puts("play    - o arbitro inicializa e joga um dos jogos");
				puts("players - Listar jogadores em jogo (nome e jogo atribuído). ");
				puts("games   - Listar jogos disponíveis.");
				puts("k<nome> - Remover um jogador do campeonato. EX: “krui” – remove jogador “rui”");
				puts("s<nome> - Suspender a comunicação entre jogador e jogo. ");
				puts("r<nome> - Retomar a comunicação entre jogador e jogo. ");
				puts("end     - Encerrar o campeonato imediatamente. ");
				puts("exit    - Sair, encerrando o árbitro.");
				puts("show <argumento> \n      duration - mostra duracao do campeonato.");
				puts("      w8time   - mostra tempo de espera por jogadores.");
				break;

//-------------------------------------------------------------------------------------------------------------------------
			icases("play")

				if(argumento = strtok(NULL," "))
				{
					printf("%s: extra operand '%s' \n",comando,argumento);
					break;
				}

				server->pidTeste = fork();

				if ( server->pidTeste == -1 ) {
        			perror("fork failed");
        			return 1;
        		}
    			else if ( server->pidTeste == 0 ) 
    			{
    				strcat(server->GAMEDIR,"/g_contaLetras"); 
        			execl(server->GAMEDIR,"g_contaLetras",NULL);
        			return 1;
        		}

    			//alarm(server->DURACAO);
    			if(pthread_create(&(server->threadRelogio),NULL,thread_relogio, (void *) server) != 0)
    				myabort("Creating clock thread blyat!",-50);

    			if ( waitpid(server->pidTeste, &status, 0) == -1 ) myabort("Falha waitpid!",-1);

    			if ( WIFEXITED(status) ) 
    			{
    				pontuacao = WEXITSTATUS(status);
					printf("Pontuação: %d\n", pontuacao);
        		}

				break;

//-------------------------------------------------------------------------------------------------------------------------
			icases("players")

				if(argumento = strtok(NULL," "))
				{
					printf("%s: extra operand '%s' \n",comando,argumento);
					break;
				}

				
				listaJogadores(server->listaJogadores);


				break;

//-------------------------------------------------------------------------------------------------------------------------
			icases("games")

				if(argumento = strtok(NULL," "))
				{
					printf("%s: extra operand '%s' \n",comando,argumento);
					break;
				}

				listaJogos(server->listaJogos);

				break;

//-------------------------------------------------------------------------------------------------------------------------
			icases("k")

				if(argumento = strtok(NULL," "))
				{
					printf("%s: extra operand '%s' \n",comando,argumento);
					sprintf(comando,"%s%s",comando,argumento);
					printf("Try '%s'\n",comando);
					break;
				}

				jogador = getJogadorByName(server->listaJogadores,nome);

				if (jogador == NULL)
				{
					printf("Jogador <%s> não encontrado no campeonato!\n",nome);
					break;
				}

				printf("%s\n",jogador->nome);
				sprintf(resposta.mensagem, "[ARBITRO]: Infelizmente foste eliminado pelo administrador do campeonato!\n");
				write(jogador->clientpipe_fd, &resposta, sizeof(resposta));

				if (contaJogadores(server->listaJogadores) == 2)
				{
					pthread_cancel(server->threadEspera);
					server->listaJogadores = eliminaJogador(server->listaJogadores,jogador->pidJogador);
					if(server->flagCampeonato == 0)	
					{
						puts("Ficou apenas um jogador, tempo de espera cancelado!");
						sprintf(resposta.mensagem,"\n[ARBITRO]:O campeonato foi adiado, espere por novas informações!\n");
						write(server->listaJogadores->clientpipe_fd, &resposta, sizeof(resposta));
					}
					else 
						if(server->flagCampeonato == 1)
						{
							pthread_cancel(server->threadEspera);
							puts("Ficou apenas um jogador, campeonato acabou!");
							sprintf(resposta.mensagem,"\n[ARBITRO]:Os teus concorrentes desistiram, és o vencedor!\n");
							write(server->listaJogadores->clientpipe_fd, &resposta, sizeof(resposta));
							setEstadoDeTodos(3, server->listaJogadores);
							server->flagCampeonato = 2;
							if(sigqueue(server->listaJogadores->pidJogo   ,10,sig_value) == -1) myabort("sigqueue() comando kill error",-1);
							if(sigqueue(server->listaJogadores->pidJogador,10,sig_value) == -1) myabort("sigqueue() comando kill error",-1);
						}
				}
				else
				{
					if (jogador->estado == 0 || jogador->estado == 1) pthread_cancel(jogador->threadResponde);
					server->listaJogadores = eliminaJogador(server->listaJogadores,jogador->pidJogador);
				}

				break;

//-------------------------------------------------------------------------------------------------------------------------
			icases("s")

				if(argumento = strtok(NULL," "))
				{
					printf("%s: extra operand '%s' \n",comando,argumento);
					sprintf(comando,"%s%s",comando,argumento);
					printf("Try '%s'\n",comando);
					break;
				}

				jogador = getJogadorByName(server->listaJogadores,nome);

				if (jogador == NULL)
				{
					printf("Jogador <%s> não encontrado no campeonato!\n",nome);
					break;
				}

				jogador->estado = 1;

				sprintf(resposta.mensagem, "[ARBITRO]: Foste suspenso do campeonato!\n");
				write(jogador->clientpipe_fd, &resposta, sizeof(resposta));

				break;

//-------------------------------------------------------------------------------------------------------------------------
			icases("r")

				if(argumento = strtok(NULL," "))
				{
					printf("%s: extra operand '%s' \n",comando,argumento);
					sprintf(comando,"%s%s",comando,argumento);
					printf("Try '%s'\n",comando);
					break;
				}

				jogador = getJogadorByName(server->listaJogadores,nome);

				if (jogador == NULL)
				{
					printf("Jogador <%s> não encontrado no campeonato!\n",nome);
					break;
				}

				if 		(server->flagCampeonato == 0) jogador->estado = 2;
				else if (server->flagCampeonato == 1) jogador->estado = 0;
				else if (server->flagCampeonato == 2) jogador->estado = 3;

				if (jogador->estado)
				{
					sprintf(resposta.mensagem, "[ARBITRO]: Conexão retmoada ao campeonato!\n");
					write(jogador->clientpipe_fd, &resposta, sizeof(resposta));
				}

				break;

//-------------------------------------------------------------------------------------------------------------------------
			icases("end")

				if(argumento = strtok(NULL," "))
				{
					printf("%s: extra operand '%s' \n",comando,argumento);
					break;
				}

				puts("comando end por implementar");

				break;

//-------------------------------------------------------------------------------------------------------------------------				
			icases("show")

				if(!(argumento = strtok(NULL," ")))
				{
					printf("arbitro: %s: argument required\n",comando);
					break;
				}

				switchs(argumento){
			//--------------------------------------------------------------------
					icases("duration")
						printf("Duração: %d\n", server->DURACAO);
						break;

			//--------------------------------------------------------------------
					icases("w8time")
						printf("Tempo de espera: %d\n", server->ESPERA);
						break;

			//--------------------------------------------------------------------
					defaults
						printf("arbitro: show: %s invalid argument\n",argumento);
						break;


				} switchs_end;

				break;

//-------------------------------------------------------------------------------------------------------------------------
			icases("exit")
				return 1;
			defaults
				printf("arbitro: %s: command not found\n",strtok(linha," "));
				break;
		} switchs_end;
		return 0;
}

//strcpy sem problema dos tamanhos... Utilizada pelo arbitro
char* strcpySegura(char* destino,char* origem,int tamD,int tamO){              //copia strings de forma segura.
    if(tamD<tamO){
#ifdef DEBUG
		puts("#String origem maior do que string destino.");
#endif  
        return origem;
    }
    for(int i=0; i<tamD; i++)
        destino[i] = origem[i];
    
	return destino;    
}