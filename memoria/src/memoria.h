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
#include <stdbool.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include "socketServidor.h"
#include "socketCliente.h"
#include "interprete.h"
#include "configuracion.h"

#define INICIAR 1
#define LEER 2
#define ESCRIBIR 3
#define FINALIZAR 4
#define NULO 1000000

//CFG
char* PUERTO_SWAP;
char* IP_SWAP;
char* PUERTO_ESCUCHA;
float RETARDO_MEMORIA;
int TAMANIO_MARCO;
int CANTIDAD_MARCOS;
int MAXIMO_MARCOS_POR_PROCESO;
int ENTRADAS_TLB;
int MAXIMO_MARCOS_POR_PROCESO;
char TLB_HABILITADA[3];
char ALGORITMO_REEMPLAZO[50];
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

typedef struct marcosLibrs {
	uint32_t numeroMarco;
} __attribute__ ((packed)) t_marcoLibre;

typedef struct TLB{
	uint32_t processID;
	uint32_t numeroPagina;
	uint32_t marco;
} __attribute__ ((packed)) t_tlb;

typedef struct tablaPaginasProceso {
	uint32_t numeroPagina;
	uint32_t numeroMarco;
	uint32_t ingreso;
} __attribute__ ((packed)) t_tablaPaginasProceso;

typedef struct tablasPaginas {
	uint32_t processID;
	t_list * listaPaginas;
} __attribute__ ((packed)) t_tablasPaginas;

typedef struct marco {
	uint32_t processID;
	char * valor;
	uint32_t numeroMarco;
	uint32_t numeroPagina;
	uint32_t bitModificacion;
	uint32_t bitLeido;
} __attribute__ ((packed)) t_marco;

typedef struct envioPaginaSwap {
	uint32_t processID;
	char * valor;
	uint32_t numeroPagina;
	uint32_t tamanioTexto;
} __attribute__ ((packed)) t_envioPaginaSwap;

typedef struct pedirPaginaSwap {
	uint32_t processID;
	uint32_t numeroPagina;
} __attribute__ ((packed)) t_pedirPaginaSwap;

typedef struct eliminarPaginaSwap {
	uint32_t processID;
	char * valor;
} __attribute__ ((packed)) t_eliminarPaginaSwap;

typedef struct iniciarSwap {
	uint32_t processID;
	uint32_t cantidadPaginas;
} __attribute__ ((packed)) t_iniciarSwap;

typedef struct decidirEstructuraSwap {
	uint32_t mensaje;
} __attribute__ ((packed)) t_decidirEstructuraSwap;

typedef struct respuestaCPU {
	uint32_t mensaje;
} __attribute__ ((packed)) t_respuestaCPU;

t_list * listaMarcosLibres;

t_list * listaTablasPaginas;

t_list * listaMarco;

t_list * listaTLB;

sem_t mutexFlushTLB;
sem_t mutexFlushMarcos;

t_resp_swap_mem * nodoRespuesta;
t_nodo_mem * nodoInstruccion;
void rutina (int n);
//int levantarCfgInicial(t_config* ar[]chConfig);
void configurarSockets();

int interpretarLinea(t_nodo_mem * nodoInstruccion);
void interpretarRespuestaSwap(t_resp_swap_mem * nodoRespuesta);
void inicializarTLB();
int inicializarTablaDePaginas();
void inicializarMarco();
void actualizarNodoPaginas(int indiceMarco, int processID, int numeroPagina);
void actualizarMarco(char * texto,int pid, int numeroPagina, char * paginaSwap, int indiceMarco);
int algoritmoReemplazo(int processID);
int algoritmoReemplazoFIFO(int processID);
void desasignarMarco(int processID, int marco);
void escribirMarco(int processID, int marco, char * texto, int numeroPagina);
static t_tablaPaginasProceso * obtenerPaginaPorNumMarco(int marco, t_tablasPaginas * nodoTablasPagina);
int valorPagina(char * instruccion);
void cargarTlb(t_nodo_mem * nodoInstruccion, t_marco * marco);
#endif /* MEMORIA_H_ */


