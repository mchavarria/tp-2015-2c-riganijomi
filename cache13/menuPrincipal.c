/*
 * menuPrincipal.c
 *
 *  Created on: 3/9/2015
 *      Author: utnso
 */
#include <stdio.h>
#include "functions.h"


int main() {
	int eleccion;

	void (*functionsArray[]) (void) = {planificador, admMemoria, admSwap};

	/* Ciclo infinito para mantener la consola activa */
	for (;;) {
		eleccion = show_menu();

		if (eleccion >= 1 && eleccion <= 3) {
			/* utiliza el puntero de la funcion que fue agregada en el array */
			/* de esta manera llamamos a la funcion sin necesidad de usar if/case */
			functionsArray[eleccion-1]() ;
		}
	}
	return 0;
}
