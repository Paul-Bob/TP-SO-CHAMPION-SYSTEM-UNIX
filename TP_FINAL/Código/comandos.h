#ifndef COMANDOS_H
#define COMANDOS_H

#include "communication.h"
#include "structs.h"
#include "switchString.h" //codigo retirado do stackoverflow para poder utilizar o switchcase com strings (posso utilizar?!)
#include "auxiliar.h"
#include "memDinamica.h"


void comandoEspecial(char* comando, char* nome);
int interpretaAdmin(char* linha,arbitro *server);
void interpretaJogador(PedidoJogador pedido ,arbitro *server, RespostaArbitro *resposta);

#endif