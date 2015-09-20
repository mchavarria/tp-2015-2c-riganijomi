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


int main(void) {
	//Variables
	char * IP;
	char * puerto;
	char archivo[1024];
	int socketPlanificador;
    char * valor;

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
		}
		else {

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
