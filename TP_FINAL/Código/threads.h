#ifndef THREADS_H
#define THREADS_H

#include "memDinamica.h"
#include "communication.h"
#include "auxiliar.h"

void * thread_relogio(void * ptr);
void * thread_trataCliente(void * cliente);
void * thread_gereCampeonato(void * ptr);

#endif