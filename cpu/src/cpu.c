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
char * retardo;

int socketADM;
int socketPlanificador = 0;

int main(void) {

	//Levanta su configuracion y se prepara para conectarse al Planificador y al ADM
	cargarCfgs();

	//Prepara el archivo log al cual va a escribir
	t_log* archivoLog = log_create("cpu.log", "CPU", false, 2);


	//Se conecta al Planificador
	socketPlanificador = socketCrearCliente(puertoPlanificador, ipPlanificador);
    //socketADM = socketCrearCliente(puertoADM, ipADM);
	//if (socketPlanificador == -1 || socketADM == -1) {
    //		log_info(archivoLog, "Alguno de los dos no se pudo conectar");
	//}

	//Voy a recibir la pcb
	t_pcb * pcbProc = malloc(sizeof(t_pcb));

	while ((socketRecibirMensaje(socketPlanificador, pcbProc,sizeof(t_pcb))) > 0){

		FILE * fp;
		char * line = NULL;
		size_t len = 0;
		ssize_t read;

		fp = fopen(pcbProc->contextoEjecucion, "r");
		if (fp == NULL){
		  exit(EXIT_FAILURE);
		}
		while ((read = getline(&line, &len, fp)) != -1) {

		  //Lee linea y ejecuta
		  interpretarLinea(line);
		  //sleep(retardo);
		}

		fclose(fp);
		if (line){
		  free(line);
		}
	}

	  return 0;
}

void interpretarLinea(char * line) {

    char * valor;
    if (esElComando(line, "iniciar")) {
		puts("entro al if iniciar");
		valor = devolverParteUsable(line, 8);
		instruccionIniciarProceso (valor);

	} else if (esElComando(line, "leer")) {
		puts("entro al if leer");
		valor = devolverParteUsable(line, 5);
		instruccionLeerPagina (valor);

	} else if (esElComando(line, "entrada-salida")) {
		puts("entro al if entrada-salida");
		valor= devolverParteUsable(line, 15);
		instruccionEntradaSalida (valor);
	} else if (esElComando(line, "escribir")) {
		puts("entro al if escribir");
		char * resultado;
		resultado = string_substring(line, 9, 1);
		valor = devolverParteUsable(line, 11);
		instruccionEscribirPagina (resultado,valor);

	} else if (esElComando(line, "finalizar")) {
		puts("entro al if finalizar");
		instruccionFinalizarProceso();
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

void instruccionIniciarProceso (char * paginas) {

	char respuesta[1024];
	if (socketADM > 0){
		//El socket está linkeado
		if (socketEnviarMensaje(socketADM,paginas,1024) > 0){
			//Envíe el packete.. espero respuesta
			if (socketRecibirMensaje(socketADM, respuesta,1024) > 0){
				printf("mProc X - %s",respuesta);
				if (socketPlanificador > 0){
					//El socket está linkeado
					socketEnviarMensaje(socketPlanificador,paginas,1024);
				}
			}
		}
	}
}

void instruccionLeerPagina (char * resultado) {
    char contenido[] = "las bolas";
   	//socketEnviarMensaje(socketADM,resultado);
   	//socketRecibirMensaje(socketADM, contenido);
    socketEnviarMensaje(socketPlanificador,contenido, sizeof(contenido));
	printf("mProc X - Pagina:%s leida:%s",resultado,contenido);


}

void instruccionEscribirPagina (int * pagina, char * texto) {
	/*char * directorioActual;
	char * puerto;
	directorioActual = obtenerDirectorio("/src/config.cfg");
	puts(directorioActual);
	puerto = configObtenerPuertoMemoria(directorioActual);
	puts(puerto);*/
}

void instruccionEntradaSalida (char * tiempo) {
}

void instruccionFinalizarProceso() {
	char respuesta[] = "finalizar";
	socketEnviarMensaje(socketPlanificador,respuesta,sizeof(respuesta));

	//socketEnviarMensaje(socketADM,"finalizar",1024);
	printf("%s","mProc X finalizado");

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
	retardo = configObtenerRetardo(directorioActual);
}






