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

#include <unistd.h>
#include <commons/log.h>
#include <sys/types.h>

#define INICIAR 1
#define LEER 2
#define ESCRIBIR 3
#define ENTRADA_SALIDA 4
#define QUANTUM_ACABADO 5
#define FINALIZAR 6

typedef struct NODO_MEM {
	uint32_t pid;
	char instruccion[20];
} __attribute__ ((packed)) t_nodo_mem;


/*
 * Tipo indica la operación que se realiza
 * [INICIAR | LEER | ESCRIBIR | E/S | FINALIZAR | QUANTUM FINALIZADO ]
 * Exito indica si fue exitoso o no
 * [ 0 | 1 ]
 * Largo indica el tamaño a recibir en el proximo mensaje (Sólo en LEER)
 * Largo indica dónde quedó el PC cuando sale por E/S o QUANTUM finalizado
 * idCPU indica qué CPU está ejecutando la instrucción
 * PID nos avisa qué proceso es el que está corriendo
 *
 * ej: | 1 - 1 - 0 |
 */
typedef struct NODO_RTA_CPU_PLAN {
 uint32_t tipo;
 uint32_t exito;
 uint32_t valor;
 uint32_t idCPU;
 uint32_t PID;
 uint32_t pc;
} __attribute__ ((packed)) t_resp_cpu_plan;

t_resp_cpu_plan * nodoRespuesta;

typedef struct PCB {
	uint32_t PID;
	uint32_t estado;
	uint32_t pc;
	uint32_t quantum;
	char contextoEjecucion[100];
} __attribute__ ((packed)) t_pcb;


t_pcb * pcbProc;
//char * obtenerDirectorio(char * nombreArchivo);

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

//Variables globales
char directorioActual[1024];
char * ipPlanificador;
char * puertoPlanificador;
char * ipADM;
char * puertoADM;
char * cantidadHilos;
int retardo;

int socketADM;
int socketPlanificador = 0;
int continuarLeyendo = 1;
int pc;

/*
void cargarCfgs();
*/
#endif /* CPU_H_ */
