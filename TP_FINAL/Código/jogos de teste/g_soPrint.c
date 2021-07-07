#include <stdio.h>

int main() {
	setbuf(stdout,NULL);
	char pergunta[100];
	int tam = 0;
	puts("Supostamente sou um jogo! Vou só escrever isto no STDOUT e dar return de -10, não tenho tratamento de sinais");
	return -10;
}