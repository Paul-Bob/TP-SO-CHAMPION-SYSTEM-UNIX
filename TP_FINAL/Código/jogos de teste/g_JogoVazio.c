#include <stdio.h>
#include <unistd.h>

int main() {
	setbuf(stdout,NULL);
	char pergunta[100];
	int tam = 0;
	puts("Supostamente sou um jogo! Vou só replicar o input e dar return de -10, não tenho tratamento de sinais");
	while(1){
		scanf(" %s", pergunta);
		printf("Recebi '%s' no STDIN !\n", pergunta);
	}
	return -10;
}