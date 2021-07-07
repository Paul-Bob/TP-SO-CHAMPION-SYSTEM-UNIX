#https://stackoverflow.com/questions/1789594/how-do-i-write-the-cd-command-in-a-makefile ??							
all: arbitro.o getEnvVars.o memDinamica.o pipesArbitro.o comandos.o auxiliar.o cliente.o threads.o communication.o
	cc arbitro.o getEnvVars.o memDinamica.o pipesArbitro.o comandos.o auxiliar.o threads.o communication.o -o arbitro -pthread
	cc cliente.o auxiliar.o -o cliente
	@$(MAKE) -C games


arbitro.o: arbitro.c arbitro.h structs.h switchString.h communication.h
	cc -c arbitro.c 
	
getEnvVars.o: getEnvVars.c
	cc -c getEnvVars.c

memDinamica.o: memDinamica.c memDinamica.h
	cc -c memDinamica.c

pipesArbitro.o: pipesArbitro.c
	cc -c pipesArbitro.c

comandos.o: comandos.c comandos.h switchString.h
	cc -c comandos.c

auxiliar.o: auxiliar.c auxiliar.h
	cc -c auxiliar.c

cliente.o: cliente.c cliente.h structs.h communication.h
	cc -c cliente.c

threads.o: threads.c threads.h
	cc -c threads.c

communication.o: communication.c communication.h
	cc -c communication.c

clear:
	rm *.o arbitro cliente
	@$(MAKE) -C games clear
	rm *.pipe
