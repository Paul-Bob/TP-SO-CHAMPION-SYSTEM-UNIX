#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

#include "cliente.h"

//Declarado globalmente para o conseguir limpar/informar arbitro no sinal...
int pipeCliente_fd = -1;
char pipeCliente [PATH_MAX];
int pipeServidor_fd = -1;   


int main(int argc, char** argv) {
	setbuf(stdout,NULL);
	PedidoJogador pedido;
	RespostaArbitro resposta;
	int nbytes_write, nbytes_read;
	pedido.dados.prox = NULL;
	pedido.dados.jogoAtribuido = NULL; 
	fd_set fds_readset_original, fds_readset_backup;
	preparaJogador(&pedido);

	//Prepara os pipes de comunicação
	pipeServidor_fd = abrePipeArbitro();
	pipeCliente_fd  = abrePipeJogador(&pedido);

	//Alerta o árbitro da sua existencia
	memset(pedido.mensagem,0,sizeof(pedido.mensagem));
	resposta.erro = 0;
	nbytes_write = write(pipeServidor_fd, &pedido, sizeof(pedido));
	if (nbytes_write == -1) myabort("[CLIENTE] Erro aquando a escrita para o pipe do árbitro!",EXIT_FAILURE); //broken pipe
	else if (nbytes_write != sizeof(pedido)) //inesperado
		fprintf(stderr, "[CLIENTE] Número inesperado de bytes escritos <%d/%d>. Envio descartado!\n",nbytes_write,sizeof(pedido));


	//Ignora o SIGPIPE, forçando assim a chamada ao sistema write() devolver -1 em vez de terminar o processo (REVER ESTA PARTE!!!!!) <-------------------------------[!!!!!!!!!!!!!!!!]
	signal(SIGPIPE, SIG_IGN);

	//ao receber ^C (SIGINT) vai limpar o pipe
	//signal(SIGINT, interpretaSIGINT);

	//tratamento do sinal do arbitro
	struct sigaction new_action;

	new_action.sa_flags = SA_SIGINFO;
	new_action.sa_sigaction = &trataSIGUSR1;

	sigfillset(&new_action.sa_mask);
													 // não deve acontecer
	if (sigaction(SIGUSR1, &new_action, NULL) == -1) myabort("Error: cannot handle SIGUSR1",EXIT_FAILURE);
	if (sigaction(SIGUSR2, &new_action, NULL) == -1) myabort("Error: cannot handle SIGUSR2",EXIT_FAILURE);
	if (sigaction(SIGINT , &new_action, NULL) == -1) myabort("Error: cannot handle SIGINT" ,EXIT_FAILURE);



	FD_ZERO(&fds_readset_original); //Initialize as an empty set
	FD_SET(0,			   & fds_readset_original);
	FD_SET(pipeCliente_fd,& fds_readset_original);

	//Cliente entra em ciclo "infinito" de comunicação com o árbitro!
	while(1) {
		fds_readset_backup = fds_readset_original; // Remeber that select() may change the provided sets
		memset(pedido.mensagem,0,sizeof(pedido.mensagem));

		switch(select(32, &fds_readset_backup, NULL, NULL, NULL)){
			case -1: // Check if it is a fatal error
				if(errno==EINTR) continue;
				else myabort("Erro no select()!",-2);
				break;
				
			case 0: //TIMEOUT
				printf("Timeout.\n");
				break;

			default: //select returns the number os descritors ready to be processed and the set of them
				if(FD_ISSET(0 , &fds_readset_backup)) {
					if(resposta.erro == 0){
						//fgets(pedido.mensagem, sizeof(pedido.mensagem), stdin);
						//if ( (strlen(pedido.mensagem) > 1) && (pedido.mensagem[strlen(pedido.mensagem)-1] ==  '\n' ))
						//	pedido.mensagem[strlen(pedido.mensagem)-1] = '\0';
						//else
						//	continue;
						scanf(" %[^\n]",pedido.mensagem);
					}
					else if(resposta.erro == -1 || resposta.erro == -3 || resposta.erro == -4){
						scanf("%s",pedido.dados.nome);
					} else continue;
					nbytes_write = write(pipeServidor_fd, &pedido, sizeof(pedido));
					if (nbytes_write == -1) { //normalmente acontece quando existe "broken pipe" (arbitro fechou o seu pipe)
						myabort("[CLIENTE] Erro aquando a escrita para o pipe do árbitro!",EXIT_FAILURE);
					} else if (nbytes_write != sizeof(pedido)) { //inesperado
						fprintf(stderr, "[CLIENTE] Número inesperado de bytes escritos <%d/%d>. Envio descartado!\n",nbytes_write,sizeof(pedido));
						continue;
					}
				}
				

				if(FD_ISSET(pipeCliente_fd, &fds_readset_backup)){

					//fprintf(stderr, "#[SELECT]  Recebi algo pelo pipe, vou ver o que é!\n");

					nbytes_read = read(pipeCliente_fd, &resposta, sizeof(resposta));
					if (nbytes_read == -1) {
						myabort("[CLIENTE] Erro aquando a leitura do próprio pipe!",EXIT_FAILURE);
					} else if (nbytes_read == 0) {
						myabort("[CLIENTE] Nenhum byte escrito pelo pipe do árbitro! Provavelmente o árbitro encerrou!",EXIT_FAILURE);
					} else if (nbytes_read != sizeof(resposta)) {
						fprintf(stderr, "[CLIENTE] Número inesperado de bytes lidos <%d/%d>. Leitura descartada!\n",nbytes_read,sizeof(resposta));
						continue;
					}


					printf("%s",resposta.mensagem);

					if(resposta.erro == -1 || resposta.erro == -3){
						printf("Nome: ");
					}
					else if(resposta.erro == -4)
					{
						printf("Deseja inscrever-se no próximo campeonato?! ");
						if(!obtemResposta())
						{
							close(pipeServidor_fd);
							limpeza();
							return -1;
						}
						printf("Nome: ");
					}
					else if(resposta.erro == -2){
						puts("Estabeleça outra ligação no próximo campeonato!");
						limpeza();
						return -1;
					}
					else if(resposta.erro == -5)
					{
						printf("Deseja inscrever-se no próximo campeonato?! ");
						if(!obtemResposta())
						{
							close(pipeServidor_fd);
							limpeza();
							return -1;
						}
						puts("Será avisado quando o registo for efetuado! Aguarde!");
						sleep(resposta.espera);
						memset(pedido.mensagem,0,sizeof(pedido.mensagem));
						resposta.erro = 0;
						nbytes_write = write(pipeServidor_fd, &pedido, sizeof(pedido));
						if (nbytes_write == -1) myabort("[CLIENTE] Erro aquando a escrita para o pipe do árbitro!",EXIT_FAILURE); //broken pipe
						else if (nbytes_write != sizeof(pedido)) //inesperado
							fprintf(stderr, "[CLIENTE] Número inesperado de bytes escritos <%d/%d>. Envio descartado!\n",nbytes_write,sizeof(pedido));
					}
				}
		}
	}
	return 0;
}

void preparaJogador(PedidoJogador * pedido){
	printf("Nickname: ");
	scanf("%s",pedido->dados.nome);
	pedido->dados.pidJogador = getpid();
}

int abrePipeJogador(const PedidoJogador * pedido){
	sprintf(pipeCliente, PADRAO_PIPE_CLIENTE, pedido->dados.pidJogador); //cria nome do pipe com o PID do cliente
	sprintf(pipeCliente, "%s%s",PIPE_DIRECTORY, pipeCliente);   //talvez use uma diretoria para os pipes mais tarde

	#ifdef DEBUG_PIPES
    fprintf(stderr, "#[PIPE] Vou tentar criar o pipe <%s>.\n",pipeCliente);
	#endif

    //verifica se já existe algum pipe com este nome
    if (access(pipeCliente, F_OK) == -1) {
    	if (mkfifo(pipeCliente, S_IRUSR | S_IWUSR) != 0)
    		myabort("[PIPE] Erro aquando a criação do próprio pipe!",EXIT_FAILURE);
    } else {
    	myabort("[PIPE] Já existe um pipe com este PID! Vou sair por questões de segurança!",EXIT_FAILURE);
    }

    #ifdef DEBUG_PIPES
    fprintf(stderr, "#[PIPE] Sucesso, agora vou tentar abrir o meu pipe <%s>.\n",pipeCliente);
    #endif

    if((pipeCliente_fd = open(pipeCliente, O_RDWR)) == -1)
    	myabort("[PIPE] Erro aquando a abertura do próprio pipe!",EXIT_FAILURE);

    return pipeCliente_fd;
}

int abrePipeArbitro(void) {
	char pipeServidor [PATH_MAX];

	sprintf(pipeServidor, "%s%s", PIPE_DIRECTORY, NOME_PIPE_SERVIDOR);
	//verifica a existencia do pipe
	if (access(pipeServidor, F_OK) == -1) {
		myabort("[PIPE] O pipe do árbitro não existe. Provavelmente o arbitro não está em execução.",EXIT_FAILURE);
	}

	#ifdef DEBUG_PIPES
	fprintf(stderr, "#[PIPE] Vou esperar enquanto o árbitro não abre o seu pipe <%s> ... \n",pipeServidor);
	#endif

	//Abrir em modo Read / Write evita receber SIGPIPE no write()
	if((pipeServidor_fd = open(pipeServidor, O_WRONLY)) == -1)
		myabort("[PIPE] Erro aquando a abertura do pipe do árbitro",EXIT_FAILURE);

	return pipeServidor_fd;
}



void limpeza(void){
	if(pipeCliente_fd != -1)
	{
		close(pipeCliente_fd);
		unlink(pipeCliente);
	}
}

void trataSIGUSR1(int sig, siginfo_t *info, void *extra){

	if (sig == SIGINT) 
	{
		PedidoJogador despedida;
		despedida.dados.pidJogador = getpid();
		sprintf(despedida.mensagem, "#quit");
		write(pipeServidor_fd, &despedida, sizeof(despedida));
		close(pipeServidor_fd);
		write(STDOUT_FILENO, "\nConexão interrompida com o árbitro!\n",
					  sizeof("\nConexão interrompida com o árbitro!\n"));
		close(pipeCliente_fd);
		unlink(pipeCliente);
		signal(sig,SIG_DFL);
		kill(getpid(),sig);
	}


	if(info->si_value.sival_int != 94357104)
		return;

	if(sig == SIGUSR1)
	{
		write(STDOUT_FILENO, "\nO campeonato acabou!\n",	
					  sizeof("\nO campeonato acabou!\n"));
		sleep(1);
		return;
	}	

	if(sig == SIGUSR2)
	{
		close(pipeServidor_fd);
		close(pipeCliente_fd);
		unlink(pipeCliente);
		signal(SIGINT,SIG_DFL);
		write(STDOUT_FILENO, "\nConexão interrompida com o árbitro!\n",
					  sizeof("\nConexão interrompida com o árbitro!\n"));
		kill(getpid(),SIGINT);
		return;
	}
}

