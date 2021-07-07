#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#define MAX_NAME 20
#define MSG_BUFFER PIPE_BUF
#define PIPE_DIRECTORY ""
#define NOME_PIPE_SERVIDOR  "arbitro.pipe"
#define PADRAO_PIPE_CLIENTE "cliente-%d.pipe"

#include "structs.h"

//pedido ao arbitro pelo cliente
typedef struct MensagemClienteParaServidor {
	Jogador dados;
	char mensagem[MSG_BUFFER];
} PedidoJogador;

//resposta do arbitro ao cliente 						//Erros
typedef struct MensagemServidorParaCliente {			// -1 já existe esse nome 
	char mensagem[MSG_BUFFER];							// -2 campeonato full 
	int erro; 											// -3 nome banido 
	int espera;	//quanto esperar pelo novo Campeonato 	// -4 - Campeonato acabo, fazer reinscrição. 
} RespostaArbitro;										// -5 está a decorrer campeonato
														
														

void difusao(char* msg,pJogador jogador,int erro);
void enviaParaCliente(RespostaArbitro resposta, Jogador jogador);

#endif