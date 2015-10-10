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
 uint32_t largo;
 uint32_t idCPU;
 uint32_t PID;
} __attribute__ ((packed)) t_resp_cpu_plan;


typedef struct PCB {
	uint32_t PID;
	int estado;
	int pc;
	int quantum;
	char contextoEjecucion[100];
} __attribute__ ((packed)) t_pcb;


t_pcb * pcbProc;
//char * obtenerDirectorio(char * nombreArchivo);

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


/*
void cargarCfgs();
*/
#endif /* CPU_H_ */
