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
#include "socketServidor.h"
#include "socketCliente.h"
#include "interprete.h"
#include "configuracion.h"
#include "serializacion.h"

//Char del orden de la Estructuras para serializar/desserializar
#define SECUENCIA_PCB "hhhhs"
#define SECUENCIA_NODO_RTA_CPU_PLAN "hhhhhhs"

//estados del pcb
#define LISTO 1
#define BLOQUEADO 2
#define FINALIZADO 4
#define EJECUTANDO 3

//tipos de resuesta
#define INICIAR 1
#define LEER 2
#define ESCRIBIR 3
#define FINALIZAR 6
#define ENTRADA_SALIDA 4
#define QUANTUM_ACABADO 5

typedef struct NODO_MEM {
	uint32_t pid;
	char instruccion[20];
} __attribute__ ((packed)) t_nodo_mem;

typedef struct hilo {
	pthread_t thread;
	char m[100];
	int  r;
} t_hilos;


/* el estado puede ser:
 * 1: listo
 * 2: bloqueado
 * 3: ejecutando
 * 4: finalizado
 */
typedef struct PCB {
	int PID;
	int estado;
	int pc;
	int quantum;
	char *contextoEjecucion;
} t_pcb;


/*
 * PID nos avisa qué proceso es el que está corriendo
 * idCPU indica qué CPU está ejecutando la instrucción
 * Tipo indica la operación que se realiza
 * [INICIAR | LEER | ESCRIBIR | E/S | FINALIZAR | QUANTUM FINALIZADO ]
 * Exito indica si fue exitoso o no
 * pagRW informa qué página fue Leida o Escrita
 * pc indica cuál es la próxima instrucción a ejecutar
 * respuesta es el resultado de la página leida
 */
typedef struct NODO_RTA_CPU_PLAN {
	int PID;
	int idCPU;
	int tipo;
	int exito;
	int pagRW;
	int pc;
	char *respuesta;
} t_resp_cpu_plan;

int contadorPID = 0;
int clientePlanificador = 0;
int servidorPlanificador = 0;
int socketCPU = -1;
sem_t mutexCPU;
sem_t semProgramas;
char comando[100];
int quantumcfg = 0;

void levantarCfg();
void* consola();
int programaValido(char * programa);
void agregarALista(char * programa);
void* enviarPCBaCPU();
void interpretarLinea(t_resp_cpu_plan * nodoRespuesta);
static t_hilos *hilo_create(pthread_t thread, char * m, int  r);
int enviarMensajeDePCBaCPU(int socketCPU, t_pcb * nodoPCB);
void empaquetarPCB(unsigned char *buffer,t_pcb * nodoPCB);
int recibirRtadeCPU(int socketCPU, t_resp_cpu_plan * nodoRta);
void desempaquetarNodoRtaCpuPlan(unsigned char *buffer,t_resp_cpu_plan * nodoRta);
void imprimeEstado(t_list *lista, char*estado );
void imprimePorcentajeCPU(t_list *lista);

t_list * listaDeListo;
t_list * listaDeBloqueado;
t_list * listaDeEjecutado;
t_list * listaDeHilos;
t_log * archivoLog;

#endif
