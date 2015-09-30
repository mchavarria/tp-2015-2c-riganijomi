#include "cpu.h"
#include <stdio.h>
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
	puts("Llego"); //BORRAR LINEA
	if (string_starts_with(package, comando)) {
		puts("Entro al IF"); //BORRAR LINEA
		return 1;
	}
	return 0;
	puts("No entro al IF"); //BORRAR LINEA
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
		//socketADM = socketCrearCliente(puertoADM, ipADM);
	if (socketPlanificador == -1 || socketADM == -1) {
		log_info(archivoLog, "Alguno de los dos no se pudo conectar");
	}

	puts("Se creo el cliente");//TODO Borrar luego!!
    printf("%s",ipPlanificador);
	//Recibe un "proceso" para ejecutar
	//socketRecibirMensaje(socketPlanificador, archivo,1024);
	puts("despues del receive");//TODO Borrar luego!!
	printf("%s",archivo);
	puts(archivo);//TODO Borrar luego!!
	puts("voy a leer el archivo");//TODO Borrar luego!!
	char archivo2 [1024];
	strcpy(archivo2, "/home/utnso/rigonijami/tp-2015-2c-riganijomi/planificador/test/programa1.cod");

	size_t len;

    //Abre el archivo recibido por el Planificador
	FILE * file = fopen ( archivo2, "r" );
	puts("pasoooo");
	if ( file != NULL )  {

		char *line ; //Largo a definir de la linea
		//while (fgets ( line, sizeof line, file ) != NULL ) {

		while(line = fgetln(file, &len)){
		//while (fgets ( line, sizeof(line),stdin ) ) {
			//Lee linea y ejecuta
			if (esElComando(line, "finalizar")) {
				puts("entro al if finalizar");
				instruccionFinalizarProceso();
			}
			if (esElComando(line, "leer")) {
				puts("entro al if leer");
				valor = devolverParteUsable(line, 5);
				instruccionLeerPagina (valor);

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
			sleep(retardo);
			/*el log por ahora lo hace el planificador
			//Logueo la instruccion
			log_info(archivoLog,"envie el archivo a nico");
			log_info(archivoLog,line);
			puts("ya envie");//TODO Borrar luego!!
			}
			//Le envio mensaje al planificador cuando termino de ejecutar el proceso
			socketEnviarMensaje(socketPlanificador, "Termine el proceso");

			fclose ( file );
			}else {
			   log_error(archivoLog,"error leyendo el archivo");
			   perror ( archivo ); /* why didn't the file open? */

		}
	}else{
		puts("ES NULL");
	}
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
	/*
	char* respuesta;
	socketEnviarMensaje(socketADM,paginas);
	socketRecibirMensaje(socketADM, respuesta);
	char * texto1;
	char * texto2;
	char * texto3;
	socketEnviarMensaje(socketPlanificador,respuesta);
	texto1 = strcat("mProc X - ",respuesta);
	printf("%s",texto3);
	*/
	puts("inicia");
	socketEnviarMensaje(socketPlanificador,paginas,1024);

}

void instruccionLeerPagina (char * resultado) {
    char * contenido;
    char * texto1;
    char * texto2;
    char * texto3;
   	socketEnviarMensaje(socketADM,resultado);
   	socketRecibirMensaje(socketADM, contenido);
   	texto1 = strcat("mProc X - Pagina:",resultado);
   	texto2 = strcat(texto1,"leida:");
	texto3 = strcat(texto2,contenido);
	printf("%s",texto3);

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
	/*
	socketEnviarMensaje(socketADM,"finalizar");
	printf("%s","mProc X finalizado");
    */
	puts("finalizar");
	socketEnviarMensaje(socketPlanificador,"finalizar",1024);

}






