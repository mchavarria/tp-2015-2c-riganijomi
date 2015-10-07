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

#include "planificador.h"


static t_pcb *hilo_create(pthread_t thread, char * m, int  r) {
	t_hilos *nuevo = malloc(sizeof(t_hilos));

	nuevo->thread = thread;
	nuevo->r = r;
	strncpy(nuevo->m, m, sizeof(nuevo->m)-1);
	nuevo->m[sizeof(nuevo->m)-1] = "\0";
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
	sem_init(&semProgramas, 0, 0);
	sem_init(&mutexCPU, 0, 0);
	/*listaDeProcesos->elements_count = 0;
	listaDeProcesos->head = NULL; *//*
	printf("%d", listaDeProcesos->elements_count);
	printf("%d", listaDeHilos->elements_count);*/
	pthread_t thr1;
	char *m1 = "thr1";
	int  r1;

	pthread_t thr2;
	char *m2 = "thr2";
	int  r2;

	r1 = pthread_create( &thr1, NULL, (void * )servidor, (void*) m1);

	r2 = pthread_create( &thr2, NULL, (void * )enviarPCBSegunFIFO, (void*) m2);
	consola();
}

void assert_pcb(t_pcb * pcb, int processID, char * contextoDeEjecucion) {
      processID = pcb->processID;
      strcpy(contextoDeEjecucion, pcb->contextoEjecucion);
}

void * enviarPCBSegunFIFO() {
	sem_wait(&mutexCPU);
	//sem_wait(&semProgramas);
	while(1) {
		char mensaje[1024];
		int processID;
		char contextoDeEjecucion[1024];
		if (listaDeProcesos->elements_count > 0) {
			//sem_wait(sem_CPU_conectada);
			assert_pcb(list_get(listaDeProcesos, 0), &processID, &contextoDeEjecucion);
			t_pcb * nodoPCB =  malloc(sizeof(t_pcb));
			strcpy(nodoPCB->contextoEjecucion, contextoDeEjecucion);
			socketEnviarMensaje(socketCPU, list_get(listaDeProcesos, 0), sizeof(t_pcb));
			//socketRecibirMensaje(servidorPlanificador, mensaje);
			socketRecibirMensaje(socketCPU, mensaje, 1024);
			while (!string_equals_ignore_case(mensaje, "finalizar")) {
				log_info(archivoLog, mensaje);
				socketRecibirMensaje(socketCPU, mensaje, 1024);
			}
			log_info(archivoLog, mensaje);
			list_remove(listaDeProcesos, 0);
		}
	}
}

void detectarComando(char * comando) {

	char * resultado;

	if (esElComando(comando, "correr")) {
		resultado = devolverParteUsable(comando, 7);
		agregarALista(resultado);
	} else {
		puts("Comando no valido.");
	}
}

char *  conseguirRutaArchivo(char * programa, int socketServidor) {
	//t_log* archivoLog = log_create("planificador.log", "Planificador", false, 2);
	char directorioActual[100];
	char directorioActualConArchivo[100];
	getcwd(directorioActual, sizeof(directorioActual));
	strcpy(directorioActualConArchivo, directorioActual);
	strcat(directorioActualConArchivo, "/mProgs/");
	strcat(directorioActualConArchivo, programa);
	log_info(archivoLog, "mProg recibido con ubicación %s.",directorioActual);
	return directorioActualConArchivo;
}

void agregarALista(char * programa) {
	printf("%d", listaDeProcesos->elements_count);

	t_pcb* pcb = malloc(sizeof(t_pcb));

	pcb->processID = contadorProcessID;
	contadorProcessID++;

	char rutaArchivo[1024];
	strcpy(rutaArchivo, conseguirRutaArchivo(programa, servidorPlanificador));

	strcpy(pcb->contextoEjecucion, rutaArchivo);

	strcat(pcb->contextoEjecucion, "\0");
	int antesDeAgregar = listaDeProcesos->elements_count;
	list_add(listaDeProcesos, pcb_create(pcb->processID, pcb->contextoEjecucion));
	int despuesDeAgregar = listaDeProcesos->elements_count;
	if (despuesDeAgregar > antesDeAgregar) {
		//sem_post(&semProgramas);
	} else {
		perror("Lista no agregada.");
	}


}

void crearThreadParaComando(char * comando) {
	printf("%d", listaDeHilos->elements_count);
	t_hilos* hilo = malloc(sizeof(t_hilos));
	strncpy(hilo->m, comando, sizeof(hilo->m)-1);
	hilo->m[sizeof(hilo->m)-1] = "\0";

	hilo->r = pthread_create( &hilo->thread, NULL, detectarComando, (void*) (hilo->m));

	list_add(listaDeHilos, hilo_create(hilo->thread, hilo->m, hilo->r));

}

int consola() {
	fgets(comando, PACKAGESIZE, stdin);
	strtok(comando, "\n");
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
	strcat(directorioActual, "/planificador/src/config.cfg\0");

	puerto = configObtenerPuertoEscucha(directorioActual);

	servidorPlanificador = socketCrearServidor(puerto);
	socketCPU = socketAceptarConexion(servidorPlanificador);
	if (socketCPU < 0)	{
		perror("accept failed");
	} else {
		sem_post(&mutexCPU);
	}
}
