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
	strcat(directorioActualConArchivo, "/");
	strcat(directorioActualConArchivo, programa);
	puts("tiene archivo pasado");
	char * filename = directorioActualConArchivo;
	FILE *file = fopen ( filename, "r" );
	if ( file != NULL )  {
	   char line [ 128 ]; /* or other suitable maximum line size */
	   while (fgets ( line, sizeof line, file ) != NULL ) {
		   fputs ( line, stdout ); /* write the line */
		   write(socketServidor,line, PACKAGESIZE);
		   socketEnviarMensaje(socketServidor, line);
		   socketRecibirMensaje(socketServidor, line);
	   }
	   fclose ( file );
	}
	else {
	   perror ( filename ); /* why didn't the file open? */
	}
}

int main() {
	int pid;
	int pid2;
	printf("Se va a crear un planificador.\n");

	pid = fork();
	if (pid == 0) {
		printf("Va a crearse el servidor del planificador.\n");
		servidorPlanificador = crear_socket_servidor("6900");
		abrirArchivo("programa1.pro", servidorPlanificador);
	} else {
		consola();
	}
}
