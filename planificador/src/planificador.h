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

//estados del pcb
#define LISTO 1
#define BLOQUEADO 2
#define FINALIZADO 4
#define EJECUTANDO 3

#define PACKAGESIZE 1024
//tipos de resuesta
#define INICIAR 1
#define LEER 2
#define ESCRIBIR 3
#define FINALIZAR 6
#define ENTRADA_SALIDA 4
#define QUANTUM_ACABADO 5



int contadorPID = 0;
int clientePlanificador = 0;
int servidorPlanificador = 0;
int socketCPU = -1;
sem_t semProgramas;
sem_t mutexCPU;
char package[PACKAGESIZE];
char comando[100];
int quantumcfg = 0;
char nombreArchivo[100];

void * servidor();
int consola();
char * conseguirRutaArchivo(char * programa, int socketServidor);
void agregarALista(char * programa);
void detectarComando(char * comando);
void * enviarPCBSegunFIFO();
/* el estado puede ser:
 * 1: listo
 * 2: bloqueado
 * 3: ejecutando
 * 4: finalizado
 */
typedef struct PCB {
	uint32_t PID;
	uint32_t estado;
	uint32_t pc;
	uint32_t quantum;
	char contextoEjecucion[100];
} __attribute__((packed)) t_pcb;

void assert_pcb(t_pcb * pcb, int PID, char * contextoDeEjecucion);

typedef struct NODO_MEM {
	uint32_t pid;
	char instruccion[20];
} __attribute__ ((packed)) t_nodo_mem;

typedef struct NODO_RTA_CPU_PLAN {
	uint32_t tipo;
	uint32_t exito;
	uint32_t valor;
	uint32_t idCPU;
	uint32_t PID;
	uint32_t pc;

} __attribute__ ((packed)) t_resp_cpu_plan;


void interpretarLinea(t_resp_cpu_plan * nodoRespuesta);

typedef struct hilo {
	pthread_t thread;
	char m[100];
	int  r;
} t_hilos;

t_list * listaDeListo;
t_list * listaDeBloqueado;
t_list * listaDeEjecutado;
t_list * listaDeHilos;
t_log * archivoLog;

#endif /* #endif /* CPU_H_ */
