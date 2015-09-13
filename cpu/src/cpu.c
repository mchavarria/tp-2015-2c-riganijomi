/*
 ============================================================================
 Name        : cpu.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include <sys/stat.h>
#include <commons/collections/dictionary.h>
#include <commons/config.h>
#include <commons/string.h>

#include <cspecs/cspec.h>

int esElComando(char * package, char * comando) {
	string_to_lower(package);
	if (string_starts_with(package, comando)) {
		return 1;
	}
	return 0;
}

char* devolverParteUsable(char * package, int desde) {
	char * cosaUsable;
	cosaUsable = string_substring_from(package, desde);
	return cosaUsable;
}

char * obtenerDirectorio(char * nombreArchivo) {
	char directorioActual[1024];
	getcwd(directorioActual, sizeof(directorioActual));
	strcat(directorioActual, nombreArchivo);
	return directorioActual;
}

void enviarNumeroDePagina (char * resultado) {
	char * directorioActual;
	char * puerto;
	directorioActual = obtenerDirectorio("/src/config.cfg");
	puts(directorioActual);
	puerto = configObtenerPuertoMemoria(directorioActual);
	puts(puerto);
}

void finalizarProcesos() {
	char * directorioActual;
	char * puerto;
	directorioActual = obtenerDirectorio("/src/config.cfg");
	puts(directorioActual);
	//puerto = configObtenerPuertoMemoria(directorioActual);
	strcpy(puerto,"6500");
	puts(puerto);
}

int main(void) {
	char * IP;
	char * puerto;
	char package[1024];
	int socketCliente;

	char directorioActual[1024];
	getcwd(directorioActual, sizeof(directorioActual));
	strcat(directorioActual, "/src/config.cfg\0");


	puts("Antes de mandar la ruta del archivo");

	IP = configObtenerIpPlanificador(directorioActual);
	puts(IP);

	puerto = configObtenerPuertoPlanificador(directorioActual);

	puts(puerto);

	socketCliente = socketCrearCliente(puerto, IP);

	puts("Se creo el cliente");

	recv(socketCliente, package, 1024, 0);

	puts("despues del receive");

	puts(package);

	//return 0; //para probar la recepcion

	strcpy(package, "leer 20\0");
	puts(package);
	puts("antes del if");
	char * resultado;
	if (esElComando(package, "leer")) {

		resultado = devolverParteUsable(package, 5);
		puts(resultado);

		enviarNumeroDePagina(resultado);
		}
	if (esElComando(package, "finalizar")) {
		finalizarProcesos();
	}
	if (esElComando(package, "entrada-salida")) {
		resultado = devolverParteUsable(package, 15);
		puts(resultado);
	}
	if (esElComando(package, "iniciar")) {
		resultado = devolverParteUsable(package, 8);
		puts(resultado);
	}
	if (esElComando(package, "escribir")) {
		char * resultado2;
		resultado = string_substring(package, 9, 1);
		resultado2 = devolverParteUsable(package, 11);

		puts(resultado);
		puts(resultado2);
	}
	puts("despues del if");
	return 0;
}
