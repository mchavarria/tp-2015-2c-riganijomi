/*
 * functions.c
 *
 *  Created on: 3/9/2015
 *      Author: utnso
 */

void planificador() {
	printf("Elegiste el planificador\n\n\n\n\n");
}

void admMemoria() {
	printf("Elegiste el administrador de memoria\n\n\n\n\n");
}

void admSwap() {
	printf("Elegiste el administrador de swap\n\n\n\n\n");
}

int show_menu() {
	printf("1 para planificador\n");
	printf("2 para administrador de memoria\n");
	printf("3 para administrador de swap\n");

	int eleccion;

	scanf("%d", &eleccion);

	return eleccion;
}
