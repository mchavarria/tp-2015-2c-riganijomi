/*
 * planificador.h
 *
 *  Created on: 17/9/2015
 *      Author: utnso
 */

struct PCB {
	int processID;
	int estado;
	int programCounter;
};

typedef struct PCB PCB;
