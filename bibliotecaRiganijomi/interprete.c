#include <string.h>
#include "interprete.h"


int esElComando(char * linea, char * comando) {
	string_to_lower(linea);
	strtok(linea, "\n");
	if (string_starts_with(linea, comando)) {
		return 1;
	}
	return 0;
}

char* devolverParteUsable(char * linea, int desde) {
	char * cosaUsable;
	cosaUsable = string_substring_from(linea, desde);
	strtok(cosaUsable, "\n");
	return cosaUsable;
}

int devolverIntInstruccion(char * linea, int desde) {
	int valor;
	char * substr;
	string_substring_from(linea, desde);
	strtok(substr,"\n");
	strtok(substr,";");
	valor = atoi(substr);
	return valor;
}

