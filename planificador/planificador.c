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

int clientePlanificador = 0;
int servidorPlanificador = 0;
int clienteAdministradorMemoria = 0;
char package[PACKAGESIZE];
/*
 * El objetivo de esta funcion es que inicalice el proceso planificador
 */
void crearCPU(char * puerto, char * puerto2) {
	int pid;
	int pid2;
	int status;
	pid = fork();
	if (pid == 0) {
		sleep(5);
		printf("Va a crearse el CPU.\n");
		crearSocketClienteSinReferencia(puerto, "192.168.1.119");

	} else {
		pid2 = fork();
		int server;
		if (pid2 == 0) {
			sleep(30);
			printf("Va a crearse el socket con la memoria.\n");
			crearSocketCliente(puerto2, "192.168.1.126", &server);
			//printf("%s", package);
			send(server, package, PACKAGESIZE, 0);
		} else {
			consola();
		}
	}
}

int consola() {
	char message[PACKAGESIZE];

	fgets(message, PACKAGESIZE, stdin);
	printf(message);

	consola();
}

int main() {
	int pid;
	int pid2;
	printf("Se va a crear un planificador.\n");

	pid = fork();
	if (pid == 0) {
		printf("Va a crearse el servidor del planificador.\n");
		servidorPlanificador = crear_socket_servidor("6500");
	} else {
		crearCPU("6500", "6667");
	}
}
