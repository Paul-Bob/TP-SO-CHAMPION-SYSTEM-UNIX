#ifndef MEMDINAMICA_h
#define MEMDINAMICA_h

#include "structs.h"
#include "auxiliar.h"
#include "communication.h"

pJogador insereJogadorInicio(pJogador listaJogadores, Jogador novoJogador); //aloca memoria e adiciona jogador à cabeça da lista ligada 
pJogador eliminaJogador(pJogador listaJogadores,pid_t pidJogador);			//elimina um jogador especifico, identificado pelo pid
pJogador libertaListaJogadores(pJogador listaJogadores);                    //elimina todos os jogadores libertando a memoria 
int existePidJogador(pJogador listaJogadores, int pid); 					//procura por pid na lista de jogadores
int contaJogadores(pJogador listaJogadores);								//conta o nr total de jogadores 
void listaJogadores(pJogador listaJogadores); 								//mostra no ecrã todos os jogadores alocados em memoria
void atribuiJogo(pJogador *jogador, pJogo listaJogos); 						//Recebe um jogador e a lista de jogos, aleatoriamente atribui um jogo ao jogador
pJogo insereJogoInicio(pJogo listaJogos, Jogo novoJogo);					//aloca memoria e adiciona um jogo à cabeça da lista ligada de jogos
pJogo libertaListaJogos(pJogo listaJogos);									//elimina todos os jogos libertando a memoria
int contaJogos(pJogo listaJogos); 											//conta o nr total de jogos
void listaJogos(pJogo listaJogos);											//mostra no ecrã todos os jogos alocados em memoria
void carregaJogos(arbitro *server); 										//vai a GAMEDIR e cria a lista ligada de todos os jogos disponiveis na directoria
int procuraNomeJogador(pJogador listaJogadores, char* nome);
pJogador getJogadorByName(pJogador listaJogadores,char *nome);
void setEstadoDeTodos(int estado, pJogador listaJogadores);
void atribuicaoForcada(pJogador *jogador, pJogo listaJogos, char* jogo);

#endif