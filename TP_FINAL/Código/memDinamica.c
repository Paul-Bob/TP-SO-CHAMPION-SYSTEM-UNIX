#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "memDinamica.h"

pJogador insereJogadorInicio(pJogador listaJogadores, Jogador novoJogador){
	pJogador aloca;
	char clientpipe [PATH_MAX];

	aloca = malloc(sizeof(Jogador));
	if(aloca == NULL){
		fprintf(stderr,"Erro de alocação de memoria para novo jogador!\n");
		return listaJogadores;
	}

	//abrimos os pipe anónimo pelo qual vamos reencaminhar mensagens do jogador ao jogo... arbitro -> jogo
	if(pipe(novoJogador.arbitroJogo_fd)<0)  myabort("Erro ao criar pipe anonimo aJ",EXIT_FAILURE);
	//printf("Abrir %d %d %s  aJ0 aJ1\n",novoJogador.arbitroJogo_fd[0],novoJogador.arbitroJogo_fd[1],novoJogador.nome);

	//construção do pipe do  jogador
	sprintf(clientpipe, PADRAO_PIPE_CLIENTE, novoJogador.pidJogador);
	sprintf(clientpipe, "%s%s", PIPE_DIRECTORY, clientpipe);

	novoJogador.clientpipe_fd = open(clientpipe, O_WRONLY); //pode bloquear caso cliente esteja a funcionar mal<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	if (novoJogador.clientpipe_fd == -1) {
		fprintf(stderr, "Erro ao abrir o pipe <%s> ! Vou ignorar pedido!\n", clientpipe);
		return listaJogadores;
	}
	//printf("Abrir %d %s  PIPE CLIENTE\n",novoJogador.clientpipe_fd,novoJogador.nome);
	novoJogador.estado = 2;
	novoJogador.score  = 0;
	novoJogador.pidJogo=-1;


	*aloca = novoJogador;
	aloca->prox = listaJogadores;
	listaJogadores = aloca;

	printf("[Nova Sessão]\nNome: '%s'\nPID : '%d'\n",novoJogador.nome,novoJogador.pidJogador);

	return listaJogadores;
}

pJogador eliminaJogador(pJogador listaJogadores,pid_t pidJogador){
	pJogador jogador = listaJogadores, anterior=NULL;
	union sigval sig_value;
	sig_value.sival_int = 94357104;

	//mete ponteiro a apontar para o pid do jogador a eliminar
	while(jogador != NULL && jogador->pidJogador != pidJogador){		
		anterior = jogador;
		jogador = jogador->prox;
	}

	//se o jogador não existe devolve a lista igual
	if(jogador == NULL){									
		printf("Pid '%d' não encontrado no campeonato! Impossivel eliminar jogador inexistente!\n",pidJogador);
		return listaJogadores;
	}

	// se o ponteiro aponta para o inicio da lista entao elimia o primeiro jogador
	// caso contrario eliminar qualquer outro jogador (jogadoriza os ponteiros) 
	if(jogador == listaJogadores)							
		listaJogadores = listaJogadores->prox;
	else
		anterior->prox = jogador->prox;					 

	//Fecha pipes utilizados pelo cliente
	close(jogador->clientpipe_fd);
	close(jogador->arbitroJogo_fd[0]);
	close(jogador->arbitroJogo_fd[1]);
	printf("[Sessão Encerrada]\nNome: '%s'\nPID : '%d'\n\n",jogador->nome,jogador->pidJogador);

	//manda sinal ao jogador que vai encerrar o processo
	// não sei se é suposto encerrar o cliente ou apenas retirar do campeonato que decorre...
	sigqueue(pidJogador,12,sig_value);

	//if (jogador->estado == 0 || jogador->estado == 1)
	//	sigqueue(jogador->pidJogo   ,10,sig_value);

	//Liberta memória e retorna lista atualizada
	free(jogador);
	return listaJogadores;
}

void setEstadoDeTodos(int estado, pJogador listaJogadores)
{
	while(listaJogadores != NULL){
		listaJogadores->estado = estado;
		listaJogadores = listaJogadores->prox;
	}
}

pJogador getJogadorByName(pJogador listaJogadores,char *nome)
{
	while(listaJogadores != NULL){
		if(!strcmp(listaJogadores->nome,nome))
			return listaJogadores;
		listaJogadores = listaJogadores->prox;
	}
	return NULL;
}

int procuraNomeJogador(pJogador listaJogadores, char* nome){
	while(listaJogadores != NULL){
		if (strcmp(listaJogadores->nome,nome) == 0)
			return 1;
	listaJogadores = listaJogadores->prox;
	}
	return 0;
}

pJogador libertaListaJogadores(pJogador listaJogadores){
	pJogador aux;
	while(listaJogadores != NULL){
		aux = listaJogadores;
		listaJogadores = listaJogadores->prox;
		close(aux->clientpipe_fd);
		close(aux->arbitroJogo_fd[0]);
		close(aux->arbitroJogo_fd[1]);
		free(aux);
	}
	puts("Todas as sessões foram encerradas!\n");
	return NULL;
}

int existePidJogador(pJogador listaJogadores, int pid){
	while(listaJogadores != NULL){
		if (listaJogadores->pidJogador == pid)
			return 1; //existe
		listaJogadores = listaJogadores->prox;
	}
	return 0; //n existe
}

int contaJogadores(pJogador listaJogadores){
	int total = 0;
	while(listaJogadores != NULL){
		if(listaJogadores->estado != -1)
			total++;
		listaJogadores = listaJogadores->prox;
	}
	return total;
}

void listaJogadores(pJogador listaJogadores){

	if (listaJogadores == NULL){
		puts("[0 Jogadores]\n");
		return;
	}
	printf("[%d Jogadores]\n",contaJogadores(listaJogadores));
	while(listaJogadores != NULL){
		printf("Nome: %-15s | PID: %-5d | Jogo: %s",listaJogadores->nome,listaJogadores->pidJogador, listaJogadores->jogoAtribuido->nomeJogo);
		if(listaJogadores->estado == -1)
			printf(" | Espetador! \n");
		else
			putchar('\n');
		listaJogadores = listaJogadores->prox;
	}
	putchar('\n');
}

void atribuiJogo(pJogador *jogador, pJogo listaJogos){
	int i = 100;
	int nrJogoRandom = intUniformRnd(1,contaJogos(listaJogos)); //retorna valor entre 1 e o nr total de jogos
	while(nrJogoRandom!=1) { //avança na lista de jogos o numero que retornou random (caso retorno > 1)
		listaJogos = listaJogos->prox;
		nrJogoRandom--;
	}
	(*jogador)->jogoAtribuido = listaJogos; //atribui o jogo escolhido aleatoreamente!

	printf("Jogo: '%s' \n\n",listaJogos->nomeJogo);
}

void atribuicaoForcada(pJogador *jogador, pJogo listaJogos, char* jogo)
{
	while(listaJogos)
	{
		if (strcmp(listaJogos->nomeJogo,jogo))
			listaJogos = listaJogos->prox;
		(*jogador)->jogoAtribuido = listaJogos;
		printf("Jogo '%s' atribuido ao jogador '%s'\n\n",listaJogos->nomeJogo,(*jogador)->nome);
		return;
	}
	printf("Jogo '%s' não encontrado na lista!\n\n",jogo);
}

pJogo insereJogoInicio(pJogo listaJogos, Jogo novoJogo){
	pJogo aloca;

	aloca = malloc(sizeof(Jogo));
	if(aloca == NULL){
		fprintf(stderr,"Erro de alocação de memoria para novo jogador!\n");
		return listaJogos;
	}

	*aloca = novoJogo;
	aloca->prox = listaJogos;
	listaJogos = aloca;

	return listaJogos;
}

pJogo libertaListaJogos(pJogo listaJogos){
	pJogo aux;
	while(listaJogos != NULL){
		aux = listaJogos;
		listaJogos = listaJogos->prox;
		free(aux);
	}
	return NULL;
}

int contaJogos(pJogo listaJogos){
	int total = 0;
	while(listaJogos != NULL){
		total++;
		listaJogos = listaJogos->prox;
	}
	return total;
}

void listaJogos(pJogo listaJogos){

	if (listaJogos == NULL){
		puts("Não existem jogos disponíveis!");
		return;
	}
	puts("[Jogos disponíveis]");
	while(listaJogos != NULL){
		printf("%s \n",listaJogos->nomeJogo);
		listaJogos = listaJogos->prox;
	}
	putchar('\n');
}

void carregaJogos(arbitro *server){
	Jogo novoJogo;
	struct dirent *ficheiro;  // Ponteiro para ler ficheiros da directoria
    struct stat informacao;   // informaçaõ sobre o ficheiro
    DIR *directoria = opendir(server->GAMEDIR); // ponteiro para a directoria
    int flagTroca = 0;
    char cwd[PATH_MAX];
  
    if (directoria == NULL) myabort("Erro ao tentar aceder a directoria dos jogos!",EXIT_FAILURE);

    //para conseguir ter acesso ao nome completo do ficheiro e obter informações pelo stat
    if (getcwd(cwd, sizeof(cwd)) == NULL) myabort("getcwd() erro!",EXIT_FAILURE);
    if (strcmp(cwd,server->GAMEDIR))
    {
    	if (chdir(server->GAMEDIR) != 0) myabort("Erro ao tentar mudar para a directoria dos jogos!",EXIT_FAILURE);
    	flagTroca = 1;
    }

    while ((ficheiro = readdir(directoria)) != NULL) { //enquanto há ficheiros para ler da directoria
    	//não interessa nenhum ficheiro que não começa com g_ (primeiras strcmp seriam desnecessarias neste projeto...)
	  	if (!(strcmp (ficheiro->d_name, ".")) || !(strcmp (ficheiro->d_name, "..")) || (ficheiro->d_name[0] != 'g') || (ficheiro->d_name[1] != '_'))
            continue;

		if (stat(ficheiro->d_name, &informacao) == -1) myabort("Erro ao tentar obter informações de ficheiro!",EXIT_FAILURE);

        if((stat(ficheiro->d_name, &informacao) >= 0) && (informacao.st_mode > 0) && (S_IEXEC & informacao.st_mode))
        	if (!S_ISDIR(informacao.st_mode)){
        		memset(&novoJogo,0,sizeof(novoJogo));
        		sprintf(novoJogo.nomeJogo, ficheiro->d_name);
				//Cria o path do jogo
				strcat(novoJogo.path,server->GAMEDIR);
				strcat(novoJogo.path,"/"); 
				strcat(novoJogo.path,novoJogo.nomeJogo);
        		server->listaJogos = insereJogoInicio(server->listaJogos, novoJogo);
        	}
    }
    closedir(directoria);  

    if(flagTroca == 1)
    	if (chdir("..") != 0) myabort("Erro ao tentar voltar a directoria!",EXIT_FAILURE);
}