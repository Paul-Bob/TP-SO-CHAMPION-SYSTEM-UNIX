all: g_contaLetras.o ../getEnvVars.o
	cc g_contaLetras.o ../getEnvVars.o -o g_contaLetras

g_contaLetras.o:g_contaLetras.c games.h
	cc -c g_contaLetras.c

clear:
	rm *.o g_contaLetras