#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <linux/limits.h>
#include <unistd.h>

//obtem variavel de ambiente GAMEDIR usada pelo arbitro e por alguns jogos
int getGAMEDIR(char *dir){ 
	char* var = getenv("GAMEDIR");
	DIR* directory;

	if (var == NULL){	//caso a variavel de ambiente não foi definida avisa 											
		fprintf(stderr, "arbitro: Variavel de ambiente \"GAMEDIR\" não foi definida!\n");
	   	if (getcwd(dir, PATH_MAX) != NULL) {
       		printf("arbitro: Directoria atual definida como diretoria dos jogos.\n");
   		} else {
       		perror("getcwd() error");
       		return -1;
   		}
   		return 1;
	}


	if (sscanf(var, "%s", dir)!=1){ //caso falhe a leitura para string
		fprintf(stderr, "arbitro: Variável de ambiente \"GAMEDIR\" tem um valor incorreto: %s\n",var);
		fprintf(stderr, "arbitro: A variável de ambiente \"GAMEDIR\" deve conter a diretoria dos jogos.\n");
		return -1;
	}	

	directory = opendir(dir);

	if (directory) {
		//directoria existe
		closedir(directory);
		return 1;
	} else if(ENOENT == errno){
		//directoria não existe
		fprintf(stderr, "arbitro: '%s' directoria inexistente.\n",dir);
		return -1;
	} else {
		//o opendir falhou por outra razão qualquer
		fprintf(stderr, "arbitro: erro inesperado na abertura da directoria '%s'.\n",dir);
		return -1;
	}

	return 1;
}


//obtem variavel de ambiente MAXPLAYERS utilizada no arbitro e nos clientes
int getMAXPLAYER(){
	int value;
	char *var = getenv("MAXPLAYER");		

	if (var == NULL){	//caso a variavel de ambiente não foi definida avisa e termina													
		fprintf(stderr, "arbitro: Variavel de ambiente \"MAXPLAYER\" desconhecida!\n");
		return -1;
	}

	if ((sscanf(var, "%d", &value)!=1) || value < 2){// caso a leitura para o inteiro falhe
		fprintf(stderr, "arbitro: Variável de ambiente \"MAXPLAYER\" tem um valor incorreto: %s\n",var);
		fprintf(stderr, "arbitro: A variável de ambiente \"MAXPLAYER\" deve conter um valor inteiro maior do que 2.\n");
		return -1;
	}

	return value;
}

/*unsigned contaPalavras(char* linha){
	int estado = 0; // flag
	unsigned cP = 0;// conta palavras

	// Percore todos os caráteres um por um
	while(*linha)
	{ 
		// Se próximo caráter é algum separador, coloca flag OFF
		if (*linha == ' ' || *linha == '\t' || *linha == '\n')
			estado = 0;
		// Se próximo caráter não é nenhum dos separadores e flag está OFF
		// Então coloca flag a ON e incrementa o contador
		else if (estado == 0)
		{
			estado = 1;
			cP++;
		}

		// Move para o próximo caráter
		linha++;
	}

	return cP;
}*/