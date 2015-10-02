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

typedef struct NODO_MEM {
	uint32_t pid;
	char instruccion[20];
} __attribute__ ((packed)) t_nodo_mem;

void rutina (int n);
int levantarCfgInicial(t_config* archConfig);
void configurarSockets();

void interpretarLinea(t_nodo_mem * nodoInstruccion);
#endif /* MEMORIA_H_ */




