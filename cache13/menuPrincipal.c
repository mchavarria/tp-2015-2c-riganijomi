/*
 * menuPrincipal.c
 *
 *  Created on: 3/9/2015
 *      Author: utnso
 */
#include <stdio.h>

/* Devuelve el valor elegido del menu */
extern int show_menu(void);

extern void planificador(void);
extern void admMemoria(void);
extern void admSwap(void);

int main() {
	int eleccion;
	void (*fArray[]) (void) = {planificador, admMemoria, admSwap};
	for (;;) {
		eleccion = show_menu();
		if (eleccion >= 1 && eleccion <= 3) {
			fArray[eleccion-1]() ;
		} else {
			printf("Opcion no valida, elija otra");
		}
	}
	return 0;
}
