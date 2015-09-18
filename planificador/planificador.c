/*
 * planificador.c
 *
 *  Created on: 13/9/2015
 *      Author: utnso
 */
/*
 * functions.c
 *
 *  Created on: 3/9/2015
 *      Author: utnso
 */
#include <stdio.h>
#include "sockets/socket.h"
#include <unistd.h>

#define PACKAGESIZE 1024

struct PCB {
	int processID;
	int estado;
	int programCounter;
	char contextoEjecucion[1024];
};

typedef struct PCB PCB;

int clientePlanificador = 0;
int servidorPlanificador = 0;
int clienteAdministradorMemoria = 0;
char package[PACKAGESIZE];

int consola() {
	char message[PACKAGESIZE];

	fgets(message, PACKAGESIZE, stdin);
	printf(message);

	consola();
}

void abrirArchivo(char * programa, int socketServidor) {
	char directorioActual[1024];
	char directorioActualConArchivo[1024];
	getcwd(directorioActual, sizeof(directorioActual));
	strcpy(directorioActualConArchivo, directorioActual);
	strcat(directorioActualConArchivo, "/test/");
	strcat(directorioActualConArchivo, programa);
	socketEnviarMensaje(socketServidor, directorioActualConArchivo);
	socketRecibirMensaje(socketServidor, package);
	puts(package);
}

int main() {
	int pid;
	printf("Se va a crear un planificador.\n");

	pid = fork();
	if (pid == 0) {
		printf("Va a crearse el servidor del planificador.\n");
		servidorPlanificador = socketCrearServidor("6667");
		abrirArchivo("programa1.pro", servidorPlanificador);
	} else {
		consola();
	}
}
