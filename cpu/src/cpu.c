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

int esElComando(char * package, char * comando) {
	string_to_lower(package);
	if (string_starts_with(package, comando)) {
		return 1;
	}
	return 0;
}

char * devolverParteUsable(package, desde) {
	char * cosaUsable;
	cosaUsable = string_substring_from(package, desde);
	return cosaUsable;
}

int main(void) {
	char package[1024];
	/* aca va lo del socket y se llena package */

	strcpy(package, "escribir 500 asdasdsadas\0");
	puts(package);
	puts("antes del if");
	char * resultado;
	if (esElComando(package, "leer")) {

		resultado = devolverParteUsable(package, 5);
		puts(resultado);

		}
	if (esElComando(package, "finalizar")) {
		puts("Finalizo");
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
