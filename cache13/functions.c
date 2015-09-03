/*
 * functions.c
 *
 *  Created on: 3/9/2015
 *      Author: utnso
 */
#include <stdio.h>
/*
 * El objetivo de esta funcion es que inicalice el proceso planificador
 */
void planificador() {
	printf("Elegiste el planificador\n");
}

/*
 * El objetivo de esta funcion es que inicalice el proceso administrador de memoria
 */
void admMemoria() {
	printf("Elegiste el administrador de memoria\n");
}

/*
 * El objetivo de esta funcion es que inicalice el proceso administrador de swap
 */
void admSwap() {
	printf("Elegiste el administrador de swap\n");
}

int show_menu() {
	int eleccion;
	int inicio = 0;
	char c;
	do {
		if (inicio != 0){
			printf("Opcion no valida, elija nuevamente \n");
		}
		printf("1 para planificador\n");
		printf("2 para administrador de memoria\n");
		printf("3 para administrador de swap\n");

		inicio++;

	}  while (((scanf("%d%c", &eleccion, &c)!=2 || c!='\n') && clean_stdin()) || eleccion<1 || eleccion>3);

	return eleccion;
}

int clean_stdin()
{
    while (getchar()!='\n');
    return 1;
}
