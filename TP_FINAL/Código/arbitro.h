#ifndef ARBITRO_H
#define ARBITRO_H

#define DURACAO_DEFAULT  120
#define ESPERA_DEFAULT    60
#define MAXPLAYER_DEFAULT 30
#define MIN_JOGADORES 2

#include "communication.h"
#include "structs.h"
#include "comandos.h"
#include "auxiliar.h"
#include "memDinamica.h"
#include "threads.h"

//Ficheiro pipesArbitro.c
int abrePipeServidor();

//Ficheiro getEnvVars.c
int getGAMEDIR(char *dir);
int getMAXPLAYER();

//limpeza ctrl c
void interpretaSIGINT (int signr);
void limpeza(void);


#endif