/*
 * planificador.h

 *
 *  Created on: 17/9/2015
 *      Author: utnso
 */
#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/log.h>
#include <commons/string.h>
#include <pthread.h>

#include <unistd.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "socketServidor.h"
#include "socketCliente.h"
#include "interprete.h"
#include "configuracion.h"
#include "serializacion.h"

//Char del orden de la Estructuras para serializar/desserializar
#define SECUENCIA_PCB "hhhhhs"
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

typedef struct hilo {
	pthread_t thread;
	char m[100];
	int  r;
} t_hilos;

typedef struct CPU {
	int pid;
	int socket;
	int disponible;
	int retardo;
	int pcb;
} t_cpu;

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
	int totalInstrucciones;
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
sem_t semProgramas;
sem_t mutexListaListo;
char comando[100];
int quantumcfg = 0;
char algoritmo[4];

void levantarCfg();

//Hilos principales
void* consola();
void* monitorearSockets();
void* enviarPCBaCPU();

int programaValido(char * programa);
void* agregarPCBALista(char * programa);
void agregarCPUALista(int cpu);
void interpretarLinea(t_resp_cpu_plan * nodoRespuesta);
int enviarMensajeDePCBaCPU(int socketCPU, t_pcb * nodoPCB);
void empaquetarPCB(unsigned char *buffer,t_pcb * nodoPCB);
int recibirRtadeCPU(int socketCPU, t_resp_cpu_plan * nodoRta);
void desempaquetarNodoRtaCpuPlan(unsigned char *buffer,t_resp_cpu_plan * nodoRta);
void* bloquearPCB(void *contexto);
void imprimeEstado(t_list *lista, char*estado );
void imprimePorcentajeCPU();
float porcentajeCPU(t_cpu *nodoCPU);
int interpretarLineaSegunRetardo(char * linea, int retardo);
int devolverParteUsableInt(char * linea, int desde);
int totalInstruccionesArchivo(char * programa);

void recibirRespuestaCPU(int socketCpu, int * nbytes);
void informarDesconexionCPU(int i);

void* buscarPCBEjecutandoPorPID(int PID);

t_list * listaDeListo;
t_list * listaDeBloqueado;
t_list * listaDeEjecutado;
t_list * listaDeHilos;
t_log * archivoLog;
t_list * listaDeCPUs;

#endif
