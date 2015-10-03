#include "cpu.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <sys/stat.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/string.h>

#include <unistd.h>
#include <commons/log.h>

//Variables globales
char directorioActual[1024];
char * ipPlanificador;
char * puertoPlanificador;
char * ipADM;
char * puertoADM;
char * cantidadHilos;
int retardo;

int socketADM;
int socketPlanificador = 0;

int main(void) {

	//Levanta su configuracion y se prepara para conectarse al Planificador y al ADM
	cargarCfgs();

	//Prepara el archivo log al cual va a escribir
	t_log* archivoLog = log_create("cpu.log", "CPU", false, 2);


	//Se conecta al Planificador
	socketPlanificador = socketCrearCliente(puertoPlanificador, ipPlanificador);
    socketADM = socketCrearCliente(puertoADM, ipADM);
	if (socketADM == -1) {
    log_info(archivoLog, "Memoria no se pudo conectar");
	}
	if (socketPlanificador == -1) {
	    log_info(archivoLog, "Planificador no se pudo conectar");
	}

	//Voy a recibir la pcb
	pcbProc = malloc(sizeof(t_pcb));

	while ((socketRecibirMensaje(socketPlanificador, pcbProc,sizeof(t_pcb))) > 0){

		FILE * fp;
		char * linea = NULL;
		size_t len = 0;
		ssize_t read;

		fp = fopen(pcbProc->contextoEjecucion, "r");
		if (fp == NULL){
		  exit(EXIT_FAILURE);
		}
		while ((read = getline(&linea, &len, fp)) != -1) {

		  //Lee linea y ejecuta
		  interpretarLinea(linea);
		  sleep(retardo);
		  //sleep(retardo);
		}

		fclose(fp);
		if (linea){
		  free(linea);
		}
	}

	  return 0;
}

void interpretarLinea(char * linea) {

    char * valor;
    if (esElComando(linea, "iniciar")) {
		puts("entro al if iniciar");
		//valor = devolverParteUsable(line, 8);
		instruccionIniciarProceso (linea);

	} else if (esElComando(linea, "leer")) {
		puts("entro al if leer");
		//valor = devolverParteUsable(linea, 5);
		instruccionLeerPagina (linea);

	} else if (esElComando(linea, "entrada-salida")) {
		puts("entro al if entrada-salida");
		//valor= devolverParteUsable(linea, 15);
		instruccionEntradaSalida (linea);
	} else if (esElComando(linea, "escribir")) {
		puts("entro al if escribir");
		char * resultado;
		//resultado = string_substring(linea, 9, 1);
		//valor = devolverParteUsable(linea, 11);
		instruccionEscribirPagina (linea);

	} else if (esElComando(linea, "finalizar")) {
		puts("entro al if finalizar");
		instruccionFinalizarProceso(linea);
	} else {
		perror("comando invaaaalido");
	}
}


/*
char * obtenerDirectorio(char * nombreArchivo) {
	char directorioActual[1024];
	getcwd(directorioActual, sizeof(directorioActual));
	strcat(directorioActual, nombreArchivo);
	return directorioActual;

}*/

void instruccionIniciarProceso (char * instruccion) {

	char respuesta[1024];
	if (socketADM > 0){
		t_nodo_mem * nodoInstruccion = malloc(sizeof(t_nodo_mem));
		nodoInstruccion->pid = pcbProc->processID;
		strcpy(nodoInstruccion->instruccion,instruccion);
		//El socket está linkeado
		if (socketEnviarMensaje(socketADM, nodoInstruccion, sizeof(t_nodo_mem)) > 0){
			//Envíe el packete.. espero respuesta
			if (socketRecibirMensaje(socketADM, respuesta,1024) > 0){
				printf("mProc X - %s",respuesta);
				if (socketPlanificador > 0){
					//El socket está linkeado
					socketEnviarMensaje(socketPlanificador,instruccion,1024);
				}
			}
		}
		free(nodoInstruccion);
	}
}

void instruccionLeerPagina (char * instruccion) {
    char contenido[20];
    t_nodo_mem * nodoInstruccion = malloc(sizeof(t_nodo_mem));
	nodoInstruccion->pid = pcbProc->processID;
	strcpy(nodoInstruccion->instruccion,instruccion);
	socketEnviarMensaje(socketADM, nodoInstruccion, sizeof(t_nodo_mem));
   	socketRecibirMensaje(socketADM, contenido,sizeof(contenido));
    socketEnviarMensaje(socketPlanificador,contenido, sizeof(contenido));
	printf("mProc X - Pagina:%s leida:%s",instruccion,contenido);
	free(nodoInstruccion);
}

void instruccionEscribirPagina (char * instruccion) {
	/*char * directorioActual;
	char * puerto;
	directorioActual = obtenerDirectorio("/src/config.cfg");
	puts(directorioActual);
	puerto = configObtenerPuertoMemoria(directorioActual);
	puts(puerto);*/
}

void instruccionEntradaSalida (char * tiempo) {
}

void instruccionFinalizarProceso(char * instruccion) {
	char respuesta[20];
	t_nodo_mem * nodoInstruccion = malloc(sizeof(t_nodo_mem));
	nodoInstruccion->pid = pcbProc->processID;
	strcpy(nodoInstruccion->instruccion,instruccion);
	socketEnviarMensaje(socketADM, nodoInstruccion, sizeof(t_nodo_mem));

   	//socketRecibirMensaje(socketADM, respuesta,sizeof(respuesta));
	socketEnviarMensaje(socketPlanificador,nodoInstruccion->instruccion,sizeof(nodoInstruccion->instruccion));
	printf("mProc %d finalizado\n",pcbProc->processID);

}


void cargarCfgs() {
	int a;
	getcwd(directorioActual, sizeof(directorioActual));
	strcat(directorioActual, "/src/config.cfg");

	ipPlanificador = configObtenerIpPlanificador(directorioActual);
	puertoPlanificador = configObtenerPuertoPlanificador(directorioActual);
	cantidadHilos = configObtenerCantidadHilos(directorioActual);
	//a = atoi(cantidadHilos);

	ipADM = configObtenerIpADM(directorioActual);
	puertoADM = configObtenerPuertoADM(directorioActual);
	retardo = 2;//configObtenerRetardo(directorioActual);
}






