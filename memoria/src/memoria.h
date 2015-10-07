#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>
#include <signal.h>
#include <stdint.h>
#include <commons/config.h>
#include <commons/log.h>

#define INICIAR 1
#define LEER 2
#define ESCRIBIR 3
#define FINALIZAR 4

//CFG
char* PUERTO_SWAP;
char* IP_SWAP;
int PUERTO_ESCUCHA;
float RETARDO_MEMORIA;
t_config* archConfig;
//LOG
t_log* archivoLog;
//sockets
int socketCpu;
int socketSwap;
int socketServidor;

/*
 * Tipo indica la operación que se realiza
 * [INICIAR | LEER | ESCRIBIR | FINALIZAR]
 * Exito indica si fue exitoso o no
 * [ 0 | 1 ]
 * Largo indica el tamaño a recibir en el proximo mensaje
 * [ 0 | sizeof(rta) | 0 | 0 ]
 *
 * ej: | 1 - 1 - 0 |
 */
typedef struct NODO_RTA_SWAP_MEM {
	uint32_t tipo;
	uint32_t exito;
	uint32_t largo;
} __attribute__ ((packed)) t_resp_swap_mem;

typedef struct NODO_MEM {
	uint32_t pid;
	char instruccion[20];
} __attribute__ ((packed)) t_nodo_mem;

t_resp_swap_mem * nodoRespuesta;
t_nodo_mem * nodoInstruccion;
void rutina (int n);
int levantarCfgInicial(t_config* archConfig);
void configurarSockets();

void interpretarLinea(t_resp_swap_mem * nodoRespuesta);
#endif /* MEMORIA_H_ */




