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


struct PCB {
	int processID;
	int estado;
	int programCounter;
	char contextoEjecucion[512];
};

typedef struct PCB PCB;

int main(void) {
	//Variables
	char * IP;
	char * puerto;
	char archivo[1024];
	int socketPlanificador;
	PCB * pcbProg = malloc(sizeof(PCB));


	//Prepara el archivo log al cual va a escribir
	t_log* archivoLog = log_create("cpu.log", "CPU", false, 2);


	//Levanta su configuracion y se prepara para conectarse
	char directorioActual[1024];
	getcwd(directorioActual, sizeof(directorioActual));
	strcat(directorioActual, "/src/config.cfg\0");


	puts("Antes de mandar la ruta del archivo");//TODO Borrar luego!!

	IP = configObtenerIpPlanificador(directorioActual);
	puts(IP);//TODO Borrar luego!!

	puerto = configObtenerPuertoPlanificador(directorioActual);

	puts(puerto);//TODO Borrar luego!!
	//Se conecta al Planificador
	socketPlanificador = socketCrearCliente(puerto, IP);
	sleep(10);
	puts("Se creo el cliente");//TODO Borrar luego!!

	//Recibe un "proceso" para ejecutar
	socketRecibirMensaje(socketPlanificador, pcbProg);
	puts("despues del receive");//TODO Borrar luego!!
	printf("%d",pcbProg->processID);
	puts(pcbProg->contextoEjecucion);//TODO Borrar luego!!
	puts("voy a leer el archivo");//TODO Borrar luego!!


	//Abre el archivo recibido por el Planificador
	FILE *file = fopen ( pcbProg->contextoEjecucion, "r" );
	if ( file != NULL )  {
	   char line [ 128 ]; //Largo a definir de la linea
	   while (fgets ( line, sizeof line, file ) != NULL ) {
		   //Lee mientras haya lineas para leer en el archivo

		   //fputs ( line, stdout ); /* write the line */

		   puts("\n estoy por enviar");//TODO Borrar luego!!

		   ////////////////////////////////////////////////////////////
		   ////////////////////////////////////////////////////////////
		   ////////////////////////////////////////////////////////////
		   //Por cada linea leida debe interpretar la instrucción a ejecutar!!
		   ////////////////////////////////////////////////////////////
		   ////////////////////////////////////////////////////////////
		   //////////////////////////////////////////////////////////////

		   //Le envio mensaje al planificador de resultado de cada instruccion
		   socketEnviarMensaje(socketPlanificador, line);
		   //Logueo la instruccion
		   log_info(archivoLog,"envie el archivo a nico");
		   log_info(archivoLog,line);

		   puts("ya envie");//TODO Borrar luego!!
	   }

	   //Le envio mensaje al planificador cuando termino de ejecutar el proceso
	   socketEnviarMensaje(socketPlanificador, "Termine el proceso");

	   fclose ( file );
	}
	else {

	   log_error(archivoLog,"error leyendo el archivo");
	   perror ( pcbProg->contextoEjecucion ); /* why didn't the file open? */
	}


	/*strcpy(paquete, "leer 20\0");
	puts(paquete);
	puts("antes del if");
	char * resultado;
	if (esElComando(paquete, "leer")) {

		resultado = devolverParteUsable(paquete, 5);
		puts(resultado);

		enviarNumeroDePagina(resultado);
		}
	if (esElComando(paquete, "finalizar")) {
		finalizarProcesos();
	}
	if (esElComando(paquete, "entrada-salida")) {
		resultado = devolverParteUsable(paquete, 15);
		puts(resultado);
	}
	if (esElComando(paquete, "iniciar")) {
		resultado = devolverParteUsable(paquete, 8);
		puts(resultado);
	}
	if (esElComando(paquete, "escribir")) {
		char * resultado2;
		resultado = string_substring(paquete, 9, 1);
		resultado2 = devolverParteUsable(paquete, 11);

		puts(resultado);
		puts(resultado2);
	}*/
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
	/*char * directorioActual;
	char * puerto;
	directorioActual = obtenerDirectorio("/src/config.cfg");
	puts(directorioActual);
	puerto = configObtenerPuertoMemoria(directorioActual);
	puts(puerto);*/
}

void instruccionEscribirPagina (int * pagina, char * texto) {
	/*char * directorioActual;
	char * puerto;
	directorioActual = obtenerDirectorio("/src/config.cfg");
	puts(directorioActual);
	puerto = configObtenerPuertoMemoria(directorioActual);
	puts(puerto);*/
}

void instruccionEntradaSalida (int * tiempo) {
	/*char * directorioActual;
	char * puerto;
	directorioActual = obtenerDirectorio("/src/config.cfg");
	puts(directorioActual);
	puerto = configObtenerPuertoMemoria(directorioActual);
	puts(puerto);*/
}

void instruccionFinalizarProceso() {
	/*char * directorioActual;
	char * puerto;
	directorioActual = obtenerDirectorio("/src/config.cfg");
	puts(directorioActual);
	//puerto = configObtenerPuertoMemoria(directorioActual);
	strcpy(puerto,"6500");
	puts(puerto);*/
}
