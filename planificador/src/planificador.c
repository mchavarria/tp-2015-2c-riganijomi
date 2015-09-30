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
#include <semaphore.h>

#include <sys/socket.h>

#define PACKAGESIZE 1024

int clientePlanificador = 0;
int servidorPlanificador = 0;
int socketCPU = -1;
int sem_CPU_conectada = 0;
char package[PACKAGESIZE];
char comando[100];

void sem_sockets() {

}
void sem_mem() {

}

static t_pcb *hilo_create(pthread_t thread, char * m, int  r) {
	t_hilos *nuevo = malloc(sizeof(t_hilos));

	nuevo->thread = thread;
	nuevo->r = r;
	strncpy(nuevo->m, m, sizeof(nuevo->m)-1);
	nuevo->m[sizeof(nuevo->m)-1] = "\0";
	puts(nuevo->m);
    return nuevo;
}

static t_pcb *pcb_create(int processID, char * contextoDeEjecucion) {
	t_pcb *nuevo = malloc(sizeof(t_pcb));
	nuevo->processID = processID;
	strcpy(nuevo->contextoEjecucion, contextoDeEjecucion);
    return nuevo;
}

int main() {
	archivoLog = log_create("planificador.log", "Planificador", false, 2);
	listaDeProcesos = list_create();
	listaDeHilos = malloc(50000);
	listaDeHilos = list_create();
	/*listaDeProcesos->elements_count = 0;
	listaDeProcesos->head = NULL; */
	printf("%d", listaDeProcesos->elements_count);
	printf("%d", listaDeHilos->elements_count);
	pthread_t thr1;
	char *m1 = "thr1";
	int  r1;

	pthread_t thr2;
	char *m2 = "thr2";
	int  r2;

	r1 = pthread_create( &thr1, NULL, (void * )servidor, (void*) m1);

	r2 = pthread_create( &thr2, NULL, enviarPCBSegunFIFO, (void*) m2);
	consola();
}

void assert_pcb(t_pcb * pcb, int processID, char * contextoDeEjecucion) {
      processID = pcb->processID;
      puts("en el assert");
      puts(pcb->contextoEjecucion);
      strcpy(contextoDeEjecucion, pcb->contextoEjecucion);
}

void enviarPCBSegunFIFO() {
	while(1) {
		char mensaje[1024];
		int processID;
		char contextoDeEjecucion[1024];
		if (listaDeProcesos->elements_count > 0) {
			//sem_wait(sem_CPU_conectada);

			while (socketCPU < 0) {
				puts("esperando a la CPU");
				sleep(2);
			}
			puts("antes de enviarrrrrrrrrrrr");
			assert_pcb(list_get(listaDeProcesos, 0), &processID, contextoDeEjecucion);

			puts("directo");
			puts(contextoDeEjecucion);
			//socketEnviarMensaje(socketCPU, contextoDeEjecucion, 1024);
			puts("despues de enviar");
			//socketRecibirMensaje(servidorPlanificador, mensaje);
			puts("antes de finalizar");
			socketRecibirMensaje(socketCPU, mensaje, 1024);
			puts(mensaje);
			log_info(archivoLog, mensaje);
			while (!string_equals_ignore_case(mensaje, "finalizar")) {
				puts("entro al while");
				socketRecibirMensaje(socketCPU, mensaje, 1024);
				puts(mensaje);
				puts("despues del receive");
				log_info(archivoLog, mensaje);
			}
			puts("despues de finalizar");
			list_remove(listaDeProcesos, 0);
		}
	}
}

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

void detectarComando(char * comando) {
	puts("Detectar comando"); //BORRAR LINEA

	char * resultado;

	if (esElComando(comando, "correr")) {
		puts("Entro a correr");
		resultado = devolverParteUsable(comando, 7);
		agregarALista(resultado);
	}
}

char *  conseguirRutaArchivo(char * programa, int socketServidor) {
	//t_log* archivoLog = log_create("planificador.log", "Planificador", false, 2);
	char directorioActual[1024];
	char directorioActualConArchivo[1024];
	getcwd(directorioActual, sizeof(directorioActual));
	strcpy(directorioActualConArchivo, directorioActual);
	strcat(directorioActualConArchivo, "/test/");
	strcat(directorioActualConArchivo, programa);
	log_info(archivoLog, package);
	return directorioActualConArchivo;
}

void agregarALista(char * programa) {
	sleep(5);
	puts("Crea la lista"); //BORRAR LINEA
	printf("%d", listaDeProcesos->elements_count);

	t_pcb* pcb = malloc(sizeof(t_pcb));

	pcb->processID = 1;

	char rutaArchivo[1024];
	puts("Antes de copiar"); //BORRAR LINEA
	strcpy(rutaArchivo, conseguirRutaArchivo(programa, servidorPlanificador));

	puts(rutaArchivo);

	strcpy(pcb->contextoEjecucion, rutaArchivo);

	strcat(pcb->contextoEjecucion, "\0");

	puts("despues del strcpy");

	list_add(listaDeProcesos, pcb_create(pcb->processID, pcb->contextoEjecucion));

	puts("Antes de Enviar"); //BORRAR LINEA
	printf("%d", listaDeProcesos->elements_count);

	puts(pcb->contextoEjecucion);

	/*
	socketEnviarMensaje(servidorPlanificador, rutaArchivo);
	puts("Despues de enviar");
	socketRecibirMensaje(servidorPlanificador, package);
	puts("Despues del primer recibir");
	puts(package);
	socketRecibirMensaje(servidorPlanificador, package);
	puts(package);
	socketRecibirMensaje(servidorPlanificador, package);
	puts(package);
	*/
}

void crearThreadParaComando(char * comando) {
	printf("%d", listaDeHilos->elements_count);
	puts("Alocar memoria");
	t_hilos* hilo = malloc(sizeof(t_hilos));
	puts("Memoria lista");
	strncpy(hilo->m, comando, sizeof(hilo->m)-1);
	hilo->m[sizeof(hilo->m)-1] = "\0";
	puts(hilo->m);
	puts("Copiar comando");

	hilo->r = pthread_create( &hilo->thread, NULL, detectarComando, (void*) (hilo->m));

	puts("Thread creado");

	list_add(listaDeHilos, hilo_create(hilo->thread, hilo->m, hilo->r));

	puts("Lista agregada");
}

int consola() {
	puts("Llego a la consola"); //BORRAR LINEA
	fgets(comando, PACKAGESIZE, stdin);
	puts(comando);
	/*
	pthread_t thr2;
	char *m2 = strcpy(m2, comando);
	int  r2;

	r2 = pthread_create( &thr2, NULL, detectarComando, (void*) m2);
*/
	crearThreadParaComando(comando);

	//detectarComando(comando);

	consola();
}

void * servidor(){
	char * puerto;
	char directorioActual[1024];
	getcwd(directorioActual, sizeof(directorioActual));
	strcat(directorioActual, "/src/config.cfg\0");

	puerto = configObtenerPuertoEscucha(directorioActual);

	servidorPlanificador = socketCrearServidor(puerto);
	socketCPU = socketAceptarConexion(servidorPlanificador);
	puts("despues del connect");
}
