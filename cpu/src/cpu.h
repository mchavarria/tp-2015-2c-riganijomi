#ifndef CPU_H_
#define CPU_H_

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <sys/stat.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/string.h>
#include <pthread.h>
#include <unistd.h>
#include <commons/log.h>
#include <sys/types.h>
#include "socketServidor.h"
#include "socketCliente.h"
#include "interprete.h"
#include "configuracion.h"
#include "serializacion.h"

#define INICIAR 1
#define LEER 2
#define ESCRIBIR 3
#define ENTRADA_SALIDA 4
#define QUANTUM_ACABADO 5
#define FINALIZAR 6
#define ERRONEA 15
//Char del orden de la Estructuras para serializar/desserializar
#define SECUENCIA_PCB "hhhhhs"
#define SECUENCIA_NODO_RTA_CPU_PLAN "hhhhhhs"
#define SECUENCIA_NODO_RTA_SWAP_MEM "hhhs"
#define SECUENCIA_CPU_MEM "hhhs"

typedef struct NODO_MEM {
	int pid;
	int pagina;
	int instruccion;
	char *texto;
} t_nodo_mem;


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

typedef struct NODO_RTA_SWAP_MEM {
	int tipo;
	int exito;
	int pagina;
	char *contenido;
} t_resp_swap_mem;


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

//Lista de Hilos de CPUs
typedef struct _t_hilos {
	pthread_t hiloCPU;
} t_hilos_CPU;

typedef struct NODO_RTA_MEM_CPU {
	uint32_t exito;
} t_resp_mem_cpu;

t_resp_cpu_plan * nodoRtaCpuPlan;
t_resp_mem_cpu * nodoRtaMemCpu;
t_resp_swap_mem * nodoRta;
t_pcb * pcbProc;
t_nodo_mem * nodoInstruccion;
t_list * listaHilosCPU;

//Log
t_log* archivoLog;

void ejecutarInstrucciones();
void cargarCfgs();

void instruccionIniciarProceso (char * instruccion);
void instruccionLeerPagina (char * instruccion);
void instruccionEscribirPagina (char * instruccion);
void instruccionEntradaSalida (char * tiempo);
void instruccionFinalizarProceso(char * instruccion);
void interpretarLinea(char * line);
void hiloCPUs();
void cpu_func();
void sacarPorQuantum();
static t_hilos_CPU *hilos_create();

//Serializacion
int recibirPCBdePlanificador(t_pcb * nodoPCB);
void notificarNoInicioPCB(t_pcb * pcbProc);
void desempaquetarPCB(unsigned char *buffer,t_pcb * nodoPCB);
int enviarMensajeRespuestaCPU(int socketPlanificador, t_resp_cpu_plan * nodoRta);
void empaquetarNodoRtaCpuPlan(unsigned char *buffer,t_resp_cpu_plan * nodoRta);
int enviarMensajeDeNodoAMem(t_nodo_mem * nodo);
void empaquetarNodoMemCPU(unsigned char *buffer,t_nodo_mem * nodo);
int recibirNodoDeMEM(t_resp_swap_mem * nodo);
void desempaquetarNodoMem(unsigned char *buffer,t_resp_swap_mem * nodo);
char * traducirInstruccion(int tipo);
int valorPaginaEnEscritura(char * instruccion);
void textoAEscribir(char * instruccion);
//Variables globales
char directorioActual[1024];
char * ipPlanificador;
char * puertoPlanificador;
char * ipADM;
char * puertoADM;
int cantidadHilos;
int retardo;

int socketADM;
int socketPlanificador = 0;
int continuarLeyendo = 1;
int pc;
int paginaInstruccion = 0;
char * texto;

#endif /* CPU_H_ */
