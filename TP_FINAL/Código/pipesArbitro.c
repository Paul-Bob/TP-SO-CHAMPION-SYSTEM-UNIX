#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "arbitro.h"

int abrePipeServidor() {
	char pipeServidor[PATH_MAX];
	int  pipeServidor_fd;

	sprintf(pipeServidor, "%s%s", PIPE_DIRECTORY, NOME_PIPE_SERVIDOR);

	//verifica se já existe
	if (access(pipeServidor, F_OK) == -1) {
        if(mkfifo(pipeServidor,  S_IRUSR | S_IWUSR)!=0) 
        	myabort("[ARBITRO] Erro ao criar o pipe!",EXIT_FAILURE);
    } else {
    	myabort("Já existe um processo a tratar da mecânica do árbitro...!",EXIT_FAILURE);
    }

  	if((pipeServidor_fd = open(pipeServidor, O_RDWR))==-1) 
		myabort("[Arbitro] Erro ao abrir o proprio pipe!",EXIT_FAILURE);	

	return pipeServidor_fd;
}