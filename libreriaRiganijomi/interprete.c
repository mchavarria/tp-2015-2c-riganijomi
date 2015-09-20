#include <commons/string.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/stat.h>
#include <commons/collections/dictionary.h>
#include <commons/config.h>
#include <unistd.h>
#include <commons/log.h>
#include "interprete.h"


int esElComando(char * linea, char * comando) {
	string_to_lower(linea);
	if (string_starts_with(linea, comando)) {
		return 1;
	}
	return 0;
}

char* devolverParteUsable(char * linea, int desde) {
	char * cosaUsable;
	cosaUsable = string_substring_from(linea, desde);
	return cosaUsable;
}

