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
int a;
char * retardo;

int socketADM;
int socketPlanificador;


int esElComando(char * package, char * comando) {
	string_to_lower(package);
	if (string_starts_with(package, comando)) {
		return 1;
	}
	return 0;
}


void cargarCfgs() {
	getcwd(directorioActual, sizeof(directorioActual));
	strcat(directorioActual, "/src/config.cfg");

	ipPlanificador = configObtenerIpPlanificador(directorioActual);
	puts(ipPlanificador);//TODO Borrar luego!!

	puertoPlanificador = configObtenerPuertoPlanificador(directorioActual);
	puts(puertoPlanificador);//TODO Borrar luego!!

	cantidadHilos = configObtenerCantidadHilos(directorioActual);
	a = atoi(cantidadHilos);
	printf("%d", a);

	ipADM = configObtenerIpADM(directorioActual);
	puts(ipADM);//TODO Borrar luego!!

	puertoADM = configObtenerPuertoADM(directorioActual);
	puts(puertoADM);//TODO Borrar luego!!

	retardo = configObtenerRetardo(directorioActual);
	puts(retardo);//TODO Borrar luego!!
}

int main(void) {
	//Variables
	int i = 0;
	char archivo[1024];
    char * valor;

    t_pcb* pcb = malloc(sizeof(t_pcb));

	//Prepara el archivo log al cual va a escribir
	t_log* archivoLog = log_create("cpu.log", "CPU", false, 2);

	//Levanta su configuracion y se prepara para conectarse al Planificador y al ADM
	cargarCfgs();

	for (i = 0; i < a; i++) {
		printf("%d", i);
	}


	//Se conecta al Planificador
	socketPlanificador = socketCrearCliente(puertoPlanificador, ipPlanificador);
    socketADM = socketCrearCliente(puertoADM, ipADM);
	//if (socketPlanificador == -1 || socketADM == -1) {
    //		log_info(archivoLog, "Alguno de los dos no se pudo conectar");
	//}

	puts("Se creo el cliente");//TODO Borrar luego!!
    printf("%s",ipPlanificador);
    //defino tamaño
    uint32_t tamanio;
	socketRecibirMensajeInt(socketPlanificador,tamanio,4);
	//Recibe un "proceso" para ejecutar
	socketRecibirMensaje(socketPlanificador, archivo,tamanio+1);
	printf("%s",archivo);






	FILE * fp;
	  char * line = NULL;
	  size_t len = 0;
	  ssize_t read;

	  fp = fopen(archivo, "r");
	  if (fp == NULL)
	  exit(EXIT_FAILURE);

	  while ((read = getline(&line, &len, fp)) != -1) {

  //Lee linea y ejecuta

		    if (esElComando(line, "finalizar")) {
				puts("entro al if finalizar");
				instruccionFinalizarProceso();
			}
			if (esElComando(line, "leer")) {
				puts("entro al if leer");
				valor = devolverParteUsable(line, 5);
				//instruccionLeerPagina (valor);

			}
			if (esElComando(line, "entrada-salida")) {
				puts("entro al if entrada-salida");
				valor= devolverParteUsable(line, 15);
				instruccionEntradaSalida (valor);
			}
			if (esElComando(line, "iniciar")) {
				puts("entro al if iniciar");
				valor = devolverParteUsable(line, 8);
				instruccionIniciarProceso (valor);

			}
			if (esElComando(line, "escribir")) {
				puts("entro al if escribir");
				char * resultado;
				resultado = string_substring(line, 9, 1);
				valor = devolverParteUsable(line, 11);
				instruccionEscribirPagina (resultado,valor);

			}



	  }

	  fclose(fp);
	  if (line)
	  free(line);
	  exit(EXIT_SUCCESS);



	  return 0;
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
	socketEnviarMensaje(socketADM,paginas,1024);
	socketRecibirMensaje(socketADM, respuesta,1024);
	socketEnviarMensaje(socketPlanificador,respuesta,1024);
	printf("mProc X - %s",&respuesta);


}

void instruccionLeerPagina (char * resultado) {
    char contenido[1024];
   	socketEnviarMensaje(socketADM,resultado);
   	socketRecibirMensaje(socketADM, contenido);
	printf("mProc X - Pagina:%s leida:%s",&resultado,&contenido);


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
	socketEnviarMensaje(socketPlanificador,"finalizar",1024);

	socketEnviarMensaje(socketADM,"finalizar",1024);
	printf("%s","mProc X finalizado");

}






