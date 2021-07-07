#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "communication.h"

void difusao(char* msg,pJogador jogador,int erro)
{
	RespostaArbitro resposta;
	resposta.erro = erro;
	sprintf(resposta.mensagem,"\n[ARBITRO]: %s\n",msg);
	while(jogador)
	{
		write(jogador->clientpipe_fd, &resposta, sizeof(resposta));
		jogador = jogador->prox;
	}
}

void enviaParaCliente(RespostaArbitro resposta, Jogador jogador)
{
	//comunicação com clientes
	int clientpipe_fd; 			//FD utilizado para responder a um cliente que é rejeitado
	char clientpipe [PATH_MAX]; //Construção do named pipe para o cliente rejeitado

	//construção do pipe do cliente ao qual deu algum tipo de erro!
	sprintf(clientpipe, PADRAO_PIPE_CLIENTE, jogador.pidJogador);
	sprintf(clientpipe, "%s%s", PIPE_DIRECTORY, clientpipe);				

	 //pode bloquear caso cliente esteja a funcionar mal (implementar outra estratégia TODO) 
	clientpipe_fd = open(clientpipe, O_WRONLY); 
	if (clientpipe_fd == -1) 
	{
		fprintf(stderr, "Erro ao abrir o pipe <%s> ! Vou ignorar pedido!\n", clientpipe);
		return;
	}
	else write(clientpipe_fd, &resposta, sizeof(resposta)); //pode bloquear (probabilidade baixa)
	close(clientpipe_fd);
}