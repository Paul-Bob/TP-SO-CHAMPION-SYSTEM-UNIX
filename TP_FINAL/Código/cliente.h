#ifndef CLIENTE_H
#define CLIENTE_H

#include "communication.h"
#include "auxiliar.h"

void myabort(const char * msg, int exit_status);
int abrePipeJogador(const PedidoJogador * pedido);
int abrePipeArbitro(void);
void preparaJogador(PedidoJogador * pedido);
void interpretaSIGINT (int signr);
void limpeza(void);
void trataSIGUSR1(int sig, siginfo_t *info, void *extra);


#endif