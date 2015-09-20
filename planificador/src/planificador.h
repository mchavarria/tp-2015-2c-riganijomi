/*
 * planificador.h

 *
 *  Created on: 17/9/2015
 *      Author: utnso
 */
#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_


void servidor();
int consola();
char * conseguirRutaArchivo(char * programa, int socketServidor);
void agregarALista(char * programa);
void detectarComando(char * comando);

typedef struct PCB {
	int processID;
	int estado;
	int programCounter;
	char contextoEjecucion[512];
} t_pcb;

t_list * listaDeProcesos;

#endif /* #endif /* CPU_H_ */
