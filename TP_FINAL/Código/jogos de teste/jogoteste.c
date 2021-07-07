#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main(){
	setbuf(stdout,NULL);
	char msg[50];

	puts("Vou replicar todo o input!");

	while(1){

		printf("INPUT: ");
		scanf(" %s", msg);
		printf("Escreveste %s !\n\n", msg);
		sleep(1);
	}
	return 0;
}
