#include "interprete.h"


int esElComando(char * linea, char * comando) {
        char * temp;
        strcpy(temp,linea);
	string_to_lower(temp);
	strtok(temp, "\n");
        strtok(temp,";");
	if (string_starts_with(temp, comando)) {
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

int devolverParteUsableInt(char * linea, int desde) {
	char * cosaUsable;
	int valor;
	cosaUsable = string_substring_from(linea, desde);
	valor = atoi(cosaUsable);
	return valor;
}

int devolverIntInstruccion(char * linea, int desde) {
	int valor;
	char * substr;
	substr = string_substring_from(linea, desde);
	strtok(substr,"\n");
	strtok(substr,";");
	valor = atoi(substr);
	return valor;
}

char * traducirExitoStatus(int exito){
	char * resultado;
	resultado = malloc(strlen("")+1);
	if (exito)
	{
		strcpy(resultado,"Exito");
	} else {
		strcpy(resultado,"Fallo");
	}
	return resultado;
}
