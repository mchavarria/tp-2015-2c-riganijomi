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
#include "commons/collections/list.h"
#include "commons/log.h"
#include <commons/string.h>

#include "planificador.h"
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#define PACKAGESIZE 1024

int clientePlanificador = 0;
int servidorPlanificador = 0;
int clienteAdministradorMemoria = 0;
char package[PACKAGESIZE];
char comando[PACKAGESIZE];

int esElComando(char * package, char * comando) {
	string_to_lower(package);
	puts("Llego"); //BORRAR LINEA
	if (string_starts_with(package, comando)) {
		puts("Entro al IF"); //BORRAR LINEA
		return 1;
	}
	return 0;
	puts("No entro al IF"); //BORRAR LINEA
}

char* devolverParteUsable(char * package, int desde) {
	char * cosaUsable;
	cosaUsable = string_substring_from(package, desde);
	return cosaUsable;
}

void detectarComando(char * comando){
	puts("Detectar comando"); //BORRAR LINEA

	char * resultado;

	if (esElComando(comando, "correr")) {
		resultado = devolverParteUsable(comando, 7);
		crearProcesoNuevo(resultado);
	}
}

char *  conseguirRutaArchivo(char * programa, int socketServidor) {
	t_log* archivoLog = log_create("planificador.log", "Planificador", false, 2);
	char directorioActual[1024];
	char directorioActualConArchivo[1024];
	getcwd(directorioActual, sizeof(directorioActual));
	strcpy(directorioActualConArchivo, directorioActual);
	strcat(directorioActualConArchivo, "/test/");
	strcat(directorioActualConArchivo, programa);
	log_info(archivoLog, package);
	return directorioActualConArchivo;
}

void crearProcesoNuevo(char * programa) {
	puts("Crea la lista"); //BORRAR LINEA

	PCB* proceso1 = malloc(sizeof(PCB));

	proceso1->processID = 1;

	char rutaArchivo[512];
	puts("Antes de copiar"); //BORRAR LINEA
	strcpy(rutaArchivo, conseguirRutaArchivo(programa, servidorPlanificador));


	puts(rutaArchivo);

	strcpy(proceso1->contextoEjecucion, rutaArchivo);

	list_add(listaDeProcesos, proceso1);

	puts("Antes de Enviar"); //BORRAR LINEA

	puts(proceso1->contextoEjecucion);

	socketEnviarMensaje(servidorPlanificador, rutaArchivo);
	puts("Despues de enviar");
	socketRecibirMensaje(servidorPlanificador, package);
	puts("Despues del primer recibir");
	puts(package);
	socketRecibirMensaje(servidorPlanificador, package);
	puts(package);
	socketRecibirMensaje(servidorPlanificador, package);
	puts(package);
}

int consola() {
	puts("Llego a la consola"); //BORRAR LINEA
	fgets(comando, PACKAGESIZE, stdin);
	puts(comando);
	sleep(5);
	detectarComando(comando);

	consola();
}

void servidor(){
	char * puerto;
	char directorioActual[1024];
	getcwd(directorioActual, sizeof(directorioActual));
	strcat(directorioActual, "/src/config.cfg\0");

	puerto = configObtenerPuertoEscucha(directorioActual);

	servidorPlanificador = socketCrearServidor(puerto);
}

int main() {
	listaDeProcesos = list_create();
	pthread_t thr1;
	char *m1 = "thr1";
	int  r1;

	r1 = pthread_create( &thr1, NULL, servidor, (void*) m1);
	consola();

}
