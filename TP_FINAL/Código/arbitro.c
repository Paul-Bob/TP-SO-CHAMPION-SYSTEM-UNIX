#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <pthread.h>
#include <time.h>
#include <setjmp.h>


#include "arbitro.h"


//Variáveis globais
int pipeServidor_fd;
jmp_buf here;
int ctrlC = 0;

//para o getline();
#define LINE_LENGHT 1024
char * Input_Admin = NULL;

int main(int argc, char** argv) {
	initRandom(); // esta função só deve ser chamada uma vez

	//utilizadas no getopt
	int opcao 	= 0;
	int index 	= 0;
	long valor 	= 0;            //guarda o valor do optarg no getopt (strtol)
	char *extra = NULL;       	//verifica getopt para casos do genero ./arbitro -d 20eakk ...

	//guarda o ultimo comando recebido
	char comando[MAX_COMANDO];  

	//estrutura que guarda os valores do arbitro
	arbitro server;				

	fd_set fds_readset_original, fds_readset_backup; //sets para o select

	//set flags
	struct timeval  tv2;
	int existe 			  = 0;
	int tempo 			  = 0;
	int flagSair 		  = 0;
	server.DURACAO 		  = -1;		
	server.ESPERA 		  = -1; 		
	server.pid 			  = getpid();
	server.flagCampeonato = 0;
	server.quantos 		  = 0;

	//Inicializa listas dinâmicas
	server.listaJogadores = NULL;
	pJogador jogador 	  = NULL;
	server.listaJogos 	  = NULL;

	//Estruturas de comunicação
	PedidoJogador 	pedido;
	RespostaArbitro resposta;
	int nbytes_read;			
	int nbytes_write;

	//para poder usar o getline();
	int nr = 0;
	size_t line_size = LINE_LENGHT;
	Input_Admin = (char *) malloc (LINE_LENGHT + 1);
	if(Input_Admin == NULL) myabort("Falha alocação memoria buffer para getline!",EXIT_FAILURE); 

	signal (SIGPIPE, SIG_IGN); // para nao crashar quando jogador da quit...

	opterr = 0; //Silencia os erros default ex: "./a.out: option requires an argument -- 'd'"

	while((opcao = getopt(argc, argv, "d:e:h")) != -1){ 			//recebe o -d Segundos e/ou o -e Segundos onde d = duração do campeonato
			switch(opcao){																						// e = tempo de espera por jogadores
				case 'h':
					printSintaxe(argv[0]);
					return(1);
					break;

				case 'd':	
					valor = strtol(optarg, &extra, 10); //valor fica com o inteiro do optarg e extra fica com os chars
					if(strlen(extra) != 0)
					{
						fprintf(stderr, "%s: Opção -d %d não permite %s concatenado.\n",argv[0],valor,extra);
						printf("%s: comando %s -h para informações de utilização!\n",argv[0],argv[0]);
						return(EXIT_FAILURE);
					}

					//se Segundos for negatio avisa e termina	
					if(valor <= 0)
					{								
						fprintf(stderr, "%s: Opção -d não permite argumento negativos ou nulos.\n",argv[0]);
						printf("%s: comando %s -h para informações de utilização!\n",argv[0],argv[0]);
						return(EXIT_FAILURE);
					}

					//verifica flag
					if (server.DURACAO == -1) server.DURACAO = valor;							
					else //se flag foi alterada
					{						
						fprintf (stderr, "%s: Opção -d repetida.\n",argv[0]);
						printf("%s: comando %s -h para informações de utilização!\n",argv[0],argv[0]);
						return(EXIT_FAILURE);
					}

					break;

				case 'e':
					valor = strtol(optarg, &extra, 10); //valor fica com o inteiro do optarg e extra fica com os chars

					if(strlen(extra) != 0)
					{
						fprintf(stderr, "%s: Opção -e %d não permite %s concatenado.\n",argv[0],valor,extra);
						printf("%s: comando %s -h para informações de utilização!\n",argv[0],argv[0]);
						return(EXIT_FAILURE);
					}

					//se Segundos for negatio avisa e termina
					if(valor <= 0)
					{		
						fprintf(stderr, "%s: Opção -e não permite argumento negativos ou nulos.\n",argv[0]);
						printf("%s: comando %s -h para informações de utilização!\n",argv[0],argv[0]);
						return(EXIT_FAILURE);
					}

					//verifica flag	
					if (server.ESPERA == -1) server.ESPERA = valor;
					else //se flag foi alterada
					{
						fprintf (stderr, "%s: Opção -e repetida.\n",argv[0]);
						printf("%s: comando %s -h para informações de utilização!\n",argv[0],argv[0]);
						return(EXIT_FAILURE);
					}

					break;


				case '?': 
				//casos em que não apresenta argumentos para -d e/ou para -e , ou utiiliza opções desconhecidas
					if (optopt == 'd' || optopt == 'e')
					{
						fprintf (stderr, "%s: Opção -%c requer um argumento.\n",argv[0], optopt);
						printf("%s: comando %s -h para informações de utilização!\n",argv[0],argv[0]);
					}
					else if (isprint (optopt))
					{
						fprintf(stderr, "%s: Opção -%c desconhecida.\n",argv[0], optopt);
						printf("%s: comando %s -h para informações de utilização!\n",argv[0],argv[0]);
					}
					else
					{
          				fprintf (stderr,"%s: Char `\\x%x' desconhecido.\n",argv[0],optopt);
          				printf("%s: comando %s -h para informações de utilização!\n",argv[0],argv[0]);
          			}

          			return 1;

          		default:
          			abort();
			}
	}

	//caso ponha argumentos sem função sera avisado mas o arbitro inicia.
  	for (index = optind; index < argc; index++) 
    	printf ("Argumento %s não tem funcionalidade.\n", argv[index]);

    //Afinal o arbitro não inicia ='))
    if(index != optind)
    {
    	printf("%s: comando %s -h para informações de utilização!\n",argv[0],argv[0]);
    	return(EXIT_FAILURE);
    }

    //caso não recebe duração atribui uma por omisão
    if (server.DURACAO == -1)
    { 
    	puts("Duração do campeonato não recebida por argumento");
    	printf("Deseja atribuir a duração default ( %d ) ?\n",DURACAO_DEFAULT);
    	if(!obtemResposta())
    	{
			printf("%s: comando %s -h para informações de utilização!\n",argv[0],argv[0]);
    		return 1;
    	}
    	server.DURACAO = DURACAO_DEFAULT;
	}

	//caso não recebe tempo de espera atribui um por omisão
    if (server.ESPERA == -1)
    { 
    	puts("Tempo de espera não recebido por argumento");
    	printf("Deseja atribuir o tempo de espera default ( %d ) ?\n",ESPERA_DEFAULT);
    	if(!obtemResposta())
    	{
    		printf("%s: comando %s -h para informações de utilização!\n",argv[0],argv[0]);
    		return 1;
    	}
    	else server.ESPERA = ESPERA_DEFAULT;
	}

	//variavel de ambiente MAXPLAYER 
	if((server.MAXPLAYER = getMAXPLAYER()) == -1 ) return(EXIT_FAILURE);

	//variavel de ambiente GAMEDIR
	if(getGAMEDIR(server.GAMEDIR) == -1) return(EXIT_FAILURE);

	//Carrega todos o nome dos executaveis começados por g_ da directroia GAMES.
	carregaJogos(&server);
	if(server.listaJogos == NULL) myabort("Nenhum jogo encontrado!",EXIT_FAILURE); 

 	//abre o pipe e ao receber ^C (SIGINT) vai limpar o pipe
	pipeServidor_fd = abrePipeServidor();
	signal(SIGINT, interpretaSIGINT);

	//Inicializar set vazio para select
	FD_ZERO(&fds_readset_original);
	FD_SET(0,			   & fds_readset_original);
	FD_SET(pipeServidor_fd,& fds_readset_original);

	puts("Arbitro iniciado com sucesso,comando 'help' para instruções de utilização.\n");

	sigsetjmp(here,1);
	while(1)
	{	
		if(ctrlC) 
		{
			strcpy(Input_Admin,"exit");
			interpretaAdmin(Input_Admin,&server);
			break;
		}
		existe = 0;
		//O select pode alterar o set inicial...
		fds_readset_backup = fds_readset_original; 

		switch(select(32, &fds_readset_backup, NULL, NULL, NULL)){
			case -1: // Verifica por erro fatal
				if(errno==EINTR) continue;
				else 
				{
					limpeza();
					myabort("Erro no select()!",-2);
				}
				break;
				
			case 0: //TIMEOUT (não tenho)
				printf("Timeout.\n");
				break;

			default: //Retorna numero dos fds que têm algo para ler
				//Se for do teclado
				if(FD_ISSET(0 , &fds_readset_backup)) 
				{
					nbytes_read = getline(&Input_Admin,&line_size, stdin);
					if(nbytes_read == -1) myabort("Falha ao ober comando admin!!",EXIT_FAILURE);
					if ( (strlen(Input_Admin) > 0) && (Input_Admin[strlen(Input_Admin)-1] ==  '\n' ) && Input_Admin[0] != '\n'){
						Input_Admin[strlen(Input_Admin)-1] = '\0';
					}
					else
						continue;

					//caso devovla algo != 0
					if(interpretaAdmin(Input_Admin,&server)){
						flagSair = 1;
						break;
					}
					memset(comando,0,sizeof(comando));
				}

				//se for do pipe
				if(FD_ISSET(pipeServidor_fd, &fds_readset_backup)){
					//reset resposta.erro para cada pedido
					resposta.erro = 0;
					//ler do pipe
					nbytes_read = read(pipeServidor_fd, &pedido, sizeof(pedido));
					if (nbytes_read == -1) myabort("Erro aquando a leitura do próprio pipe!", EXIT_FAILURE);
					else if (nbytes_read != sizeof(pedido)) //recebi tamanho não adequado no pipe, ignoro
						continue;

					//se o pid consta da lista de jogadores
					if (existePidJogador(server.listaJogadores,pedido.dados.pidJogador))
					{
						jogador = getJogadorByName(server.listaJogadores,pedido.dados.nome);

						//se a mensagem começa por # é para o árbitro
						if (pedido.mensagem[0] == '#') 
						{
							interpretaJogador(pedido,&server,&resposta);
							if(existePidJogador(server.listaJogadores,pedido.dados.pidJogador))
								nbytes_write = write(jogador->clientpipe_fd, &resposta, sizeof(resposta)); 
							continue;
						}
						
						//se a mensagem for para o jogo e o jogador estiver a jogar
						if(jogador->estado == 0)
						{	
							existe = kill(jogador->pidJogo,0);
							write(jogador->arbitroJogo_fd[1], &pedido.mensagem, sizeof(pedido.mensagem));
							nbytes_write = write(jogador->arbitroJogo_fd[1], "\n", 1);
							memset(pedido.mensagem,0,sizeof(pedido.mensagem));
							if(nbytes_write == -1 || existe == -1) //broken pipe i guess
							{
								sprintf(resposta.mensagem, "[ARBITRO]: Algo de errado aconteceu com o teu jogo! Iremos rever o jogo!\n");
								nbytes_write = write(jogador->clientpipe_fd, &resposta, sizeof(resposta));
								printf("O jogo %s deu problemas ao jogador %s! Suspensão automatica!\n",jogador->jogoAtribuido->nomeJogo, jogador->nome);
								jogador->estado = 1;
							}
							continue;
						}
						
						//se a mensagem for para o jogo e o jogador estiver suspenso
						if(jogador->estado == 1)
						{
							printf("%s (suspenso) : %s\n\n",jogador->nome,pedido.mensagem);
							sprintf(resposta.mensagem, "\n[ARBITRO]: Estás suspenso do campeonato, aguarde!\n");
							nbytes_write = write(jogador->clientpipe_fd, &resposta, sizeof(resposta));
							continue;
						}

						//se a mensagem for para o jogo mas o campeonato ainda não começou
						if(jogador->estado == 2)
						{
							sprintf(resposta.mensagem, "[ARBITRO] Aguarde pelo inicio do campeonato!\n");
							nbytes_write = write(jogador->clientpipe_fd, &resposta, sizeof(resposta));
							continue;
						}

						//se a mensagem for para o jogo mas o campeonato já acabou
						if(jogador->estado == 3)
						{
							sprintf(resposta.mensagem, "[ARBITRO] O campeonato já acabou!\n");
							nbytes_write = write(jogador->clientpipe_fd, &resposta, sizeof(resposta));
							continue;
						}
						continue;
					}
				
					//caso recebemos #quit de um jogador nao registado ignoramos...
					if(!strcmp(pedido.mensagem,"#quit"))
						continue;

					//Caso se tente conectar enquanto esta um campeonato a decorrer
					if(server.flagCampeonato == 1)
					{
						gettimeofday(&tv2, NULL);
						resposta.erro = -5;
						tempo = ((int) (tv2.tv_usec - server.tv3.tv_usec) / 1000000 + (int) (tv2.tv_sec - server.tv3.tv_sec));
						sprintf(resposta.mensagem, "Ja existe um campeonato a decorrer há %d segundos!\n"
													"Aguarde %d segundos para se inscrever no próximo campeonato!\n",
													tempo, server.DURACAO - tempo);
						resposta.espera = server.DURACAO - tempo + 1;
						enviaParaCliente(resposta,pedido.dados);
						server.avisaFim[server.quantos++] = pedido.dados.pidJogador;
						continue;
					}

					//Caso não há registo do jogador e o pedido é valido verificamos se pode juntar-se
					//se o campeonato não suporta mais jogadores
					if(contaJogadores(server.listaJogadores) >= server.MAXPLAYER)
					{
						sprintf(resposta.mensagem, "Campeonato já atingiu limite máximo de jogadores!\n");
						resposta.erro = -2;
						enviaParaCliente(resposta,pedido.dados);
					}

					//se o nome já existe na lista de jogadores ativos
					if(procuraNomeJogador(server.listaJogadores,pedido.dados.nome))
					{
						sprintf(resposta.mensagem, "Já existe um jogador com esse nome, escolha outro!\n");
						resposta.erro = -1;
						enviaParaCliente(resposta,pedido.dados);
					}

					//nomes banidos how e tart ( para comandos show e start )
					//bani nome how para poder usar comando show sem suspender o how (poderia ter sido feita outra abordagem)
					if(!strcmp(pedido.dados.nome,"how") && !strcmp(pedido.dados.nome,"tart"))
					{
						sprintf(resposta.mensagem, "Esse nome está banido para este campeonato, escolha outro!\n");
						resposta.erro = -3;
						enviaParaCliente(resposta,pedido.dados);
					}

					//se existe algum erro não adicionamos
					if (resposta.erro) continue;


					//inserimos jogador à cabeça da lista ligada e atribuimos um jogo aleatorio.
					server.listaJogadores = insereJogadorInicio(server.listaJogadores, pedido.dados);
					atribuiJogo(&server.listaJogadores, server.listaJogos);
					//Da as boas vindas ao jogador
					sprintf(resposta.mensagem, "\nBem-vindo ao campeonato %s!\n", server.listaJogadores->nome);
					sprintf(resposta.mensagem, "%sFoi-lhe atribuido o jogo %s\n",resposta.mensagem,server.listaJogadores->jogoAtribuido->nomeJogo);
					sprintf(resposta.mensagem, "%sO campeonato terá uma duração de %d segundos!\n",resposta.mensagem,server.DURACAO);
					if(contaJogadores(server.listaJogadores) <= MIN_JOGADORES && server.flagCampeonato != 1)
						sprintf(resposta.mensagem, "%sAguarde! Será avisado alguns segundos antes do campeonato ter início!\n\n",resposta.mensagem);
					else
					{
						gettimeofday(&tv2, NULL);
						tempo = server.ESPERA - ((int) (tv2.tv_usec - server.tv1.tv_usec) / 1000000 + (int) (tv2.tv_sec - server.tv1.tv_sec));
						sprintf(resposta.mensagem, "%s\n[ARBITRO]: O campeonato terá inicio dentro de %d segundos!\n\n",resposta.mensagem, tempo);
					}
					write(server.listaJogadores->clientpipe_fd, &resposta, sizeof(resposta)); //pode bloquear (probabilidade baixa)
					//Caso este é o segundo jogador a dar entrada lançamos a thread que ativa o tempo de espera!
					if(contaJogadores(server.listaJogadores) == MIN_JOGADORES && server.flagCampeonato != 1){
						if(pthread_create(&(server.threadCampeonato),NULL,thread_gereCampeonato, (void *) &server) != 0)
							myabort("Erro no lançamento da thread trata cliente!",-1);
						gettimeofday(&(server.tv1), NULL);
					}
					//se fizemos isto tudo então o cliente já esta tratado, podemos seguir
					continue;
				}		
		}
		if(flagSair) break;
	}
	//limpa mem dinamica
	server.listaJogos     = libertaListaJogos(server.listaJogos);
	server.listaJogadores = libertaListaJogadores(server.listaJogadores);
	limpeza();
	return 0;
}

void limpeza(void){
	char pipeServidor[PATH_MAX];
	sprintf(pipeServidor, "%s%s", PIPE_DIRECTORY, NOME_PIPE_SERVIDOR);

	if(pipeServidor_fd!=-1) {
		close(pipeServidor_fd);
		unlink(pipeServidor);
	}
}

void interpretaSIGINT (int signr){
	if (signr == SIGINT) {
		ctrlC = 1;
		siglongjmp(here,1);
	}
}
