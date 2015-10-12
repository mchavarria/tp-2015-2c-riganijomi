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

static t_pcb *pcb_create(int PID, char * contextoDeEjecucion) {
	t_pcb *nuevo = malloc(sizeof(t_pcb));
	nuevo->PID = PID;
	nuevo->estado = LISTO;
	nuevo->pc = 0;
	nuevo->quantum = quantumcfg;

	strcpy(nuevo->contextoEjecucion, contextoDeEjecucion);
    return nuevo;
}


int main() {
	archivoLog = log_create("planificador.log", "Planificador", false, 2);
	listaDeBloqueado = malloc(50000);
	listaDeBloqueado = list_create();
	listaDeEjecutado = malloc(50000);
	listaDeEjecutado = list_create();
	listaDeListo = malloc(50000);
	listaDeListo = list_create();
	sem_init(&semProgramas, 0, 0);
	sem_init(&mutexCPU, 0, 0);
	/*listaDeListo->elements_count = 0;
	listaDeProcesos->head = NULL; *//*
	printf("%d", listaDeListo->elements_count);
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

void assert_pcb(t_pcb * pcb, int PID, char * contextoDeEjecucion) {
      PID = pcb->PID;
      strcpy(contextoDeEjecucion, pcb->contextoEjecucion);
}

void * enviarPCBSegunFIFO() {
	sem_wait(&mutexCPU);
	//sem_wait(&semProgramas);
	while(1) {
		if (listaDeListo->elements_count > 0) {
			//sem_wait(sem_CPU_conectada);
			t_pcb * nodoPCB =  list_get(listaDeListo, 0);
			socketEnviarMensaje(socketCPU, nodoPCB, sizeof(t_pcb));
			//saca de lista ready
			list_remove(listaDeListo, 0);
			//se agrega a la lista de ejecucion
			list_add(listaDeEjecutado,nodoPCB );

			t_resp_cpu_plan * nodoRespuesta;
			int nbytes;
			nodoRespuesta = malloc(sizeof(t_resp_cpu_plan));
			while ((nbytes = socketRecibirMensaje(socketCPU, nodoRespuesta,sizeof(t_nodo_mem)) > 0)){
				interpretarLinea(nodoRespuesta);

			}//fin while recibir rta
		}//cierra if cantidad de elementos
	}//cierra while
}

void detectarComando(char * comando) {

	char * resultado;

	if (esElComando(comando, "correr")) {
		resultado = devolverParteUsable(comando, 7);
		agregarALista(resultado);
		//agrega en variable global el nombre del archivo para usar en pcb
		strcpy(nombreArchivo,resultado);

	} else {
		puts("Comando no valido.");
	}
}

char *  conseguirRutaArchivo(char * programa, int socketServidor) {
	//t_log* archivoLog = log_create("planificador.log", "Planificador", false, 2);
	//char directorioActual[100];
	char directorioActualConArchivo[100];
	//getcwd(directorioActual, sizeof(directorioActual));
	//strcpy(directorioActualConArchivo, directorioActual);
	strcat(directorioActualConArchivo, "/mProgs/");
	strcat(directorioActualConArchivo, programa);
	//log_info(archivoLog, "mProg recibido con ubicación %s.",directorioActual);
	return directorioActualConArchivo;
}

void agregarALista(char * programa) {
	printf("%d", listaDeListo->elements_count);

	t_pcb * pcb = malloc(sizeof(t_pcb));

	pcb->PID = contadorPID;
	contadorPID++;

	char rutaArchivo[100];
	strcat(rutaArchivo,"/mProgs/");
	strcat(rutaArchivo,programa);
	//strcpy(rutaArchivo, conseguirRutaArchivo(programa, servidorPlanificador));

	strcpy(pcb->contextoEjecucion, rutaArchivo);

	strcat(pcb->contextoEjecucion, "\0");
	pcb->pc=0;
	pcb->estado=LISTO;
	pcb->quantum=quantumcfg;
	//imprimo pcb
	int antesDeAgregar = listaDeListo->elements_count;
	list_add(listaDeListo, pcb);
	int despuesDeAgregar = listaDeListo->elements_count;
	if (despuesDeAgregar > antesDeAgregar) {
		//sem_post(&semProgramas);
	} else {
		perror("Lista no agregada.");
	}


}

void crearThreadParaComando(char * comando) {
	printf("Procesos listos: %d\n", listaDeListo->elements_count);
	t_hilos* hilo = malloc(sizeof(t_hilos));
	strncpy(hilo->m, comando, sizeof(hilo->m)-1);
	hilo->m[sizeof(hilo->m)-1] = "\0";

	hilo->r = pthread_create( &hilo->thread, NULL, detectarComando, (void*) (hilo->m));

	list_add(listaDeListo, hilo_create(hilo->thread, hilo->m, hilo->r));

}

int consola() {
	fgets(comando, PACKAGESIZE, stdin);
	strtok(comando, "\n");

	crearThreadParaComando(comando);
	consola();
}

void * servidor(){
	char * puerto;
	char directorioActual[1024];
	getcwd(directorioActual, sizeof(directorioActual));
	strcat(directorioActual, "/src/config.cfg\0");//Para eclipse
	//strcat(directorioActual, "/planificador/src/config.cfg\0");//Para consola
	puerto = configObtenerPuertoEscucha(directorioActual);
    quantumcfg = configObtenerQuantum(directorioActual);
	servidorPlanificador = socketCrearServidor(puerto);
	socketCPU = socketAceptarConexion(servidorPlanificador);
	if (socketCPU < 0)	{
		perror("accept failed");
	} else {
		sem_post(&mutexCPU);
	}
}


void interpretarLinea(t_resp_cpu_plan * nodoRespuesta) {

    int tipoResp = nodoRespuesta->tipo;
    int exito = nodoRespuesta->exito;
    int PID = nodoRespuesta ->PID;
    int idCPU = nodoRespuesta ->idCPU;

    bool buscarPorPID(t_pcb * nodoPCB) {
    		return (nodoPCB->PID == PID);
    	}

    t_pcb* nodoPCB=NULL;
	nodoPCB = list_find(listaDeEjecutado,(void*)buscarPorPID);


    switch (tipoResp) {
    		case INICIAR:
				if (exito){
					log_info(archivoLog,"CPU %d: Proceso mProc %d (%s) creado",idCPU,PID,nombreArchivo);
				} else {
					//saco de la lista de ejecutando
					log_debug(archivoLog,"CPU %d: Proceso mProc %d (%s) fallo",idCPU,PID,nombreArchivo);
					list_remove_by_condition(listaDeEjecutado,(void*)buscarPorPID);
					free(nodoPCB);
				}
			break;
    		case LEER:
    			if (exito){
    				char respuesta[nodoRespuesta->valor];
    				socketRecibirMensaje(socketCPU, respuesta,nodoRespuesta->valor);
    				int numPagina;
    				socketRecibirMensaje(socketCPU, numPagina,sizeof(numPagina));
    				log_info(archivoLog,"CPU %d: Proceso mProc %d - pagina %d leida: %s",idCPU,PID,numPagina);
				} else {
					log_debug(archivoLog,"CPU %d:Proceso mProc %d NO leido",idCPU,PID);
				}
    		break;
    		case ESCRIBIR:
    			//No necesita avisarle al cpu
				//TODO Solo loguear y actualizar las estructuras necesarias
    			if (exito){
    				log_info(archivoLog,"CPU %d: Proceso mProc %d - escribio ",idCPU,PID);
				} else {
					log_debug(archivoLog,"CPU %d:Proceso mProc %d NO escribio",idCPU,PID);
				}
    		break;
    		case ENTRADA_SALIDA:
    		   	//No necesita avisarle al cpu
				//TODO Solo loguear y actualizar las estructuras necesarias
				if (exito){
					log_info(archivoLog,"CPU %d: Proceso mProc %d - se bloquea por entrada-salida ",idCPU,PID);
					list_remove_by_condition(listaDeEjecutado,(void*)buscarPorPID);
					list_add(listaDeBloqueado,nodoPCB);
					//usar el valor de nodo respuesta como tiempo de entrada salida
				} else {
					log_info(archivoLog,"CPU %d: Proceso mProc %d - fallo instruccion entrada-salida ",idCPU,PID);
				}
				break;
    		case QUANTUM_ACABADO:
				if (exito){
					t_pcb* nodoPCB=NULL;
					nodoPCB = list_find(listaDeEjecutado,(void*)buscarPorPID);
					//actualizar el pc del nodoPCB con el valor del pc del nodo respuesta
					nodoPCB->pc = nodoRespuesta->pc;
					list_remove_by_condition(listaDeEjecutado,(void*)buscarPorPID);
					list_add(listaDeListo,nodoPCB);
					log_info(archivoLog,"CPU %d: Proceso mProc %d - se bloquea por quantum acabado ",idCPU,PID);
				} else {
				//validar luego que opcion hay para fallar
					log_info(archivoLog,"CPU %d: Proceso mProc %d - fallo la instruccion por quantum",idCPU,PID);

				}
				break;
    		case FINALIZAR:
    			if (exito){
    				log_info(archivoLog,"CPU %d: Proceso mProc %d (%s) finalizado",idCPU,PID,nombreArchivo);
				} else {
					log_info(archivoLog,"CPU %d: Proceso mProc %d (%s) no finalizado",idCPU,PID,nombreArchivo);
				}
    			list_remove_by_condition(listaDeEjecutado,(void*)buscarPorPID);
    			free(nodoPCB);
			break;
    		default:
    		    perror("mensaje erroneo.");
    	}
}
