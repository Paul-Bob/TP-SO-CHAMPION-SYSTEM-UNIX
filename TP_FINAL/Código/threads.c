#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>
#include <errno.h> 

#include "threads.h"

void desbloquearThread(int signal){} // apenas para desbloquear o read...

void * threadEsperaJogo(void * cliente){
	pJogador jogador = (pJogador ) cliente;		//ponteiro para o jogador que vai ser tratado pela thread
	RespostaArbitro resposta; 					//estrutura para responder ao jogador
	int status;									//variavel que recebe o exitstatus do jogo

	if ( waitpid(jogador->pidJogo, &status, 0) == -1 ) myabort("Falha waitpid!",-1);

	if ( WIFEXITED(status) ) 
	{
		jogador->score = WEXITSTATUS(status);
		memset(resposta.mensagem, 0, sizeof(resposta.mensagem));
		if(jogador->score > 125 || jogador->score < 0)
		{
			jogador->score -= 256;
			sprintf(resposta.mensagem, "Sua pontuação: %d! Contacte o administrador, houve problemas com o jogo que lhe foi atribuido.\n", jogador->score);	
		}
		else
		{
			sprintf(resposta.mensagem, "\nSua pontuação: %d!\n", jogador->score);
		}
			write(jogador->clientpipe_fd, &resposta, sizeof(resposta));
	}

	jogador->pidJogo = -1;
}

void * thread_trataCliente(void * cliente){
	//arbitro * server = (arbitro *) servidor; 	//ponteiro para a estrutura do árbitro
	pJogador jogador = (pJogador ) cliente;		//ponteiro para o jogador que vai ser tratado pela thread
	RespostaArbitro resposta; 					//estrutura para responder ao jogador
	int jogoArbitro_fd[2]; 						//pipe anônimo que trata de receber o output do jogo
	int nr = 0, nr_write;
	pthread_t exit;

	//técnica para desbloquear o read quando jogador da quit ou é eliminado.
	struct sigaction desbloquear;
	memset(&desbloquear, 0, sizeof(desbloquear));
	desbloquear.sa_flags = 0;
	desbloquear.sa_handler = desbloquearThread;
	sigaction(SIGUSR1, &desbloquear, NULL);
	signal (SIGPIPE, SIG_IGN); // para nao crashar quando jogador da quit...

	//abre o pipe que recebe output do jogo / encerra arbitro caso falhe 
	//ALTERAR ESTRATÉGIA: Avisar cliente de erro no jogo e encerrar comunicações.
	if(pipe(jogoArbitro_fd)<0)  myabort("FILHO: Erro ao criar pipe anonimo jA",EXIT_FAILURE);
	
	resposta.erro = 0; //erro reportado a jogador fica a zero just in case...

	//Avisa o jogador que o campeonato começou
	sprintf(resposta.mensagem, "O campeonato começou! Boa sorte %s!\n", jogador->nome);
	write(jogador->clientpipe_fd, &resposta, sizeof(resposta));

	//Faz fork para poder executar o jogo
	jogador->pidJogo = fork();

	//Se fork falhou encerra...
	//ALTERAR ESTRATÉGIA: Avisar cliente de erro no jogo e encerrar comunicações.
	if(jogador->pidJogo == -1) myabort("Falha fork() thread_trataCliente!",EXIT_FAILURE);
	if(jogador->pidJogo ==  0){
		//Processo filho
		//Fecha fds que não utiliza
		if(close(jogoArbitro_fd[0]) 		 == -1) myabort("FILHO: Erro ao fechar extremidade de leitura jA0!",EXIT_FAILURE);
		if(close(jogador->arbitroJogo_fd[1]) == -1) myabort("FILHO: Erro ao fechar extremidade de escrita aJ1!",EXIT_FAILURE);

		//Redireciona stdin / stdout
		if(dup2(jogador->arbitroJogo_fd[0],STDIN_FILENO ) == -1) myabort("FILHO: Erro na duplicação do pipe aJ0 (read  end)",EXIT_FAILURE);		
		if(dup2(jogoArbitro_fd[1]		  ,STDOUT_FILENO) == -1) myabort("FILHO: Erro na duplicação do pipe jA1 (write end)",EXIT_FAILURE);

		//executa o jogo do jogador
		execl(jogador->jogoAtribuido->path,jogador->jogoAtribuido->nomeJogo,NULL);

		//caso falhou na execução encerra o árbitro
		//ALTERAR ESTRATÉGIA: Avisar cliente de erro no jogo e encerrar comunicações.
		myabort("execl correu mal!",EXIT_FAILURE);
	}
	else{
		//Processo pai
		//Fecha fds que não utiliza
		if(close(jogoArbitro_fd[1])          == -1) puts("Erro ao fechar extremidade de escrita jA1!");

		//lança thread que espera pelo exit status
		if(pthread_create(&exit,NULL,threadEsperaJogo,  (void *) jogador) != 0)
			myabort("Erro no lançamento da thread trata cliente!",-1);

		//Enquanto o jogador está no campeonato le do pipe de saida do jogo e redireciona para o pipe do cliente.
		while(jogador->estado < 2)
		{
			memset(resposta.mensagem, 0, sizeof(resposta.mensagem));
			nr = read(jogoArbitro_fd[0], resposta.mensagem, sizeof(resposta.mensagem)-1);
			if (nr > 0 && jogador->estado == 0){
				resposta.mensagem[nr] = '\0';
				nr_write = write(jogador->clientpipe_fd, &resposta, sizeof(resposta));
				if(nr_write == -1)
				{
					printf("Jogador %s tem problemas com o pipe! Suspenso automaticamente!", jogador->nome);
					jogador->estado = 1;
				}
			}
			if(nr < 0)
			{
				if(errno != 4)
					printf("!!Jogador %s errno %d!!\n",jogador->nome,errno);
				jogador->estado = 5;
			}
		}

		if(close(jogoArbitro_fd[0])  == -1) myabort("Erro ao fechar extremidade de leitura jA0!",EXIT_FAILURE);
		pthread_join(exit,NULL);
	}
}	


void * thread_gereCampeonato(void * ptr){
	arbitro * server = (arbitro *) ptr;
	pJogador jogador = server->listaJogadores;
	char mensagem[300];
	union sigval sig_value;
	sig_value.sival_int = 94357104;
	int vencedor = 0;

	struct sigaction desbloquear;
	memset(&desbloquear, 0, sizeof(desbloquear));
	desbloquear.sa_flags = 0;
	desbloquear.sa_handler = desbloquearThread;
	sigaction(SIGUSR1, &desbloquear, NULL);

	printf("Limite minimo de jogadores atingido!\nCampeonato irá ter inicio dentro de '%d' segundos!\n\n",server->ESPERA);

	sleep(0.1); //implementação a pedreiro, evita atropelamentos de mensagem no segundo cliente

	sprintf(mensagem, "O campeonato terá inicio dentro de %d segundos!\n", server->ESPERA);
	difusao(mensagem,server->listaJogadores,0);

	server->flagCampeonato = 3;
	sleep(server->ESPERA);
	server->flagCampeonato = 1;
	gettimeofday(&(server->tv3), NULL);
	puts("Acabou o tempo de espera, começou o campeonato!\n");

	jogador = server->listaJogadores;
	while(jogador != NULL)
	{
		jogador->estado = 0;
		//lançamos a thread que trata de estabelecer a comunicação jogo -> arbitro -> cliente
		if(pthread_create(&(jogador->threadResponde),NULL,thread_trataCliente, (void *) jogador) != 0)
			myabort("Erro no lançamento da thread trata cliente!",-1);
		jogador = jogador->prox;
	}

	sleep(server->DURACAO);
	server->flagCampeonato = 2;
	puts("Acabou o campeonato!\n");

	jogador = server->listaJogadores;
	while(jogador != NULL)
	{
		if(jogador->estado != 2)
			jogador->estado = 3;
		if(jogador->pidJogo != -1)
			if(sigqueue(jogador->pidJogo   ,10,sig_value) == -1) myabort("sigqueue() error",EXIT_FAILURE);
		if(sigqueue(jogador->pidJogador,10,sig_value) == -1) myabort("sigqueue() error",EXIT_FAILURE);
		jogador = jogador->prox;
	}

	jogador = server->listaJogadores;
	while(jogador != NULL)
	{
		if(jogador->estado == 3)
		pthread_join(jogador->threadResponde,NULL);
		jogador = jogador->prox;
	}

	//desnecessario
	sleep(2.5);

	//procura maior score
	jogador = server->listaJogadores;
	while(jogador != NULL)
	{
		if(jogador->score > vencedor && jogador->estado)
			vencedor = jogador->score;
		jogador = jogador->prox;
	}

	//cria string de vencedores
	sprintf(mensagem, "Lista dos vencedores (Maior pontuação obtida = %d)\n", vencedor);
	puts("Tabela de pontuação:");
	jogador = server->listaJogadores;
	while(jogador != NULL)
	{
		printf("-> %-10s --- %3d pontos!\n",jogador->nome,jogador->score);
		if(jogador->score == vencedor && jogador->estado != 2)
			sprintf(mensagem, "%s-> %s\n",mensagem, jogador->nome);
		jogador = jogador->prox;
	}

	printf("\n%s",mensagem);

	sprintf(mensagem, "%s\nJá se pode inscrever no próximo campeonato caso deseja jogar!!\n\n",mensagem);

	//avisa todos os jogadores dos vencedores
	difusao(mensagem,server->listaJogadores,-4);


	//elimina todos os jogadores
	server->listaJogadores = libertaListaJogadores(server->listaJogadores);

	server->flagCampeonato = 0;

	while(server->quantos)
		sigqueue(server->avisaFim[--server->quantos],10,sig_value);
} 

