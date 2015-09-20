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
	//int estado;
	//int programCounter;
	char contextoEjecucion[100];;
} t_pcb;

typedef struct hilo {
	pthread_t thread;
	char m[100];;
	int  r;
} t_hilos;

t_list * listaDeProcesos;
t_list * listaDeHilos;

#endif /* #endif /* CPU_H_ */
