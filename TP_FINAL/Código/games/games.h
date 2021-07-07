#ifndef GAMES_H
#define GAMES_H

#define MAX_LETRAS 20
#define ESC "\033" 

void trataSIGUSR1(int sig, siginfo_t *info, void *extra);
int getGAMEDIR(char *dir);

#endif