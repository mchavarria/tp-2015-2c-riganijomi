#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <cspecs/cspec.h>

#include <sys/stat.h>
#include <commons/collections/dictionary.h>
#include <commons/config.h>

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

int main(void) {
	//Variables
	int i = 0;
	char archivo[1024];
    char * valor;
	int socketPlanificador;
    int socketADM;

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
	if (socketPlanificador == -1 || socketADM == -1) {
		log_info(archivoLog, "Alguno de los dos no se pudo conectar");
	}
	sleep(10);
	puts("Se creo el cliente");//TODO Borrar luego!!

	//Recibe un "proceso" para ejecutar
	socketRecibirMensaje(socketPlanificador, archivo);
	puts("despues del receive");//TODO Borrar luego!!
	printf("%d",archivo);
	puts(archivo);//TODO Borrar luego!!
	puts("voy a leer el archivo");//TODO Borrar luego!!
    //Abre el archivo recibido por el Planificador
	FILE *file = fopen ( archivo, "r" );
	if ( file != NULL )  {
		char line [ 128 ]; //Largo a definir de la linea
		while (fgets ( line, sizeof line, file ) != NULL ) {
		//Lee linea y ejecuta
        if (esElComando(line, "leer")) {
           	valor = devolverParteUsable(line, 5);
           	instruccionLeerPagina (valor);

   		}
	   	if (esElComando(line, "finalizar")) {
	   		instruccionFinalizarProceso();
	   	}
	   	if (esElComando(line, "entrada-salida")) {
	   		valor= devolverParteUsable(line, 15);
   	    	instruccionEntradaSalida (valor);
	   	}
	   	if (esElComando(line, "iniciar")) {
	  		valor = devolverParteUsable(line, 8);
    		instruccionIniciarProceso (valor);

	  	}
	   	if (esElComando(line, "escribir")) {
	   		char * resultado;
	   		resultado = string_substring(line, 9, 1);
	   		valor = devolverParteUsable(line, 11);
	   		instruccionEscribirPagina (resultado,valor);

	   	}

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
	return 0;
}

/*
char * obtenerDirectorio(char * nombreArchivo) {
	char directorioActual[1024];
	getcwd(directorioActual, sizeof(directorioActual));
	strcat(directorioActual, nombreArchivo);
	return directorioActual;

}*/

void instruccionIniciarProceso (int * paginas) {
	/*char * directorioActual;
	char * puerto;
	directorioActual = obtenerDirectorio("/src/config.cfg");
	puts(directorioActual);
	puerto = configObtenerPuertoMemoria(directorioActual);
	puts(puerto);*/
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


void sem_sockets() {

}
void sem_mem() {

}

void cargarCfgs(){
	getcwd(directorioActual, sizeof(directorioActual));
	strcat(directorioActual, "/src/config.cfg\0");

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
}
