/*
 * planificador.h

 *
 *  Created on: 17/9/2015
 *      Author: utnso
 */
#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include <stdint.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/log.h>
#include <commons/string.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <stdlib.h>


#define PACKAGESIZE 1024

int contadorProcessID = 0;
int clientePlanificador = 0;
int servidorPlanificador = 0;
int socketCPU = -1;
sem_t semProgramas;
sem_t mutexCPU;
char package[PACKAGESIZE];
char comando[100];

void * servidor();
int consola();
char * conseguirRutaArchivo(char * programa, int socketServidor);
void agregarALista(char * programa);
void detectarComando(char * comando);
void * enviarPCBSegunFIFO();

typedef struct PCB {
	uint32_t processID;
	//int estado;
	//int programCounter;
	char contextoEjecucion[100];
} __attribute__((packed)) t_pcb;

void assert_pcb(t_pcb * pcb, int processID, char * contextoDeEjecucion);

typedef struct hilo {
	pthread_t thread;
	char m[100];;
	int  r;
} t_hilos;

t_list * listaDeProcesos;
t_list * listaDeHilos;
t_log * archivoLog;

#endif /* #endif /* CPU_H_ */
