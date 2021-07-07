#ifndef AUXILIAR_H
#define AUXILIAR_H

char* strcpySegura(char* destino,char* origem,int tamD,int tamO);
int intUniformRnd(int a, int b);
void initRandom();
void printSintaxe(char* argv0);
void myabort(const char * msg, int exit_status);
int obtemResposta();

#endif