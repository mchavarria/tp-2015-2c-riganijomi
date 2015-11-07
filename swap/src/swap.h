#ifndef SWAP_H_
#define SWAP_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <commons/collections/list.h>
#include <stdbool.h>
#include <commons/log.h>
#include "socketServidor.h"
#include "socketCliente.h"
#include "interprete.h"
#include "configuracion.h"
#include "serializacion.h"

#define INICIAR 1
#define LEER 2
#define ESCRIBIR 3
#define FINALIZAR 4

#define SECUENCIA_NODO_RTA_SWAP_MEM "hhhs"
#define SECUENCIA_MEM_SWAP "hhhs"
/*
 * Tipo indica la operación que se realiza
 * [INICIAR | LEER | ESCRIBIR | FINALIZAR]
 * Largo indica el tamaño a recibir en el proximo mensaje
 * [ 0 | sizeof(rta) | 0 | 0 ]
 *
 * ej: | 1 - 1 - 0 |
 */
typedef struct NODO_RTA_SWAP_MEM {
	int tipo;
	int exito;
	int pagina;
	char *contenido;
} t_resp_swap_mem;

t_resp_swap_mem * nodoRespuesta;

typedef struct NODO_MEM_SWAP {
	int tipo;
	int pid;
	int pagina;
	char *contenido;
} t_nodo_mem_swap;


typedef struct nodoLibre {
	int indice;
	int tamanio;
} t_nodoLibre;

typedef struct nodoProceso {
	int idProc;
	int indice;
	int tamanio;
} t_nodoProceso;

typedef struct nodoEspera {
	int idProc;
	int tamanio;
} t_nodoEspera;

typedef struct envioPaginaSwap {
	uint32_t processID;
	char * valor;
	uint32_t numeroPagina;
	uint32_t tamanioTexto;
} __attribute__ ((packed)) t_envioPaginaSwap;

//sockets
int socketMemoria;
int socketServidor;

//Log
t_log* archivoLog;

void levantarCfgInicial();
void crearParticion();
void configurarSocketServer();
static t_nodoLibre *crearNodoLibre(int indice, int tamanio);
static t_nodoEspera *crearNodoEspera(int idProc, int cantPagProceso);
static t_nodoProceso *crearNodoProceso(int idProc, int indice, int cantPagProceso);

//Recibe un proceso, en caso de aceptarlo crea un nodoProceso y lo agrega a la lista
//En caso de rechazarlo nada jajaja
//En caso de estar fragmentando (SEMAFOOOROOO) lo pone en nodoEspera
//En todos los casos genera un MENSAJE de aviso.
void recibirProceso();

//cuando finaliza el proceso, lo elimina.
//En la lista de libres agrega el indice y el tamaño del nodo eliminado
void eliminarProceso(int pid);

void leerPaginaProceso(int idProc, int pagina);
void escribirPagina (int idProc, int pagina, char * texto);
//Condicion para el list_find en nodos libres

void estructuraRecibida(t_nodo_mem_swap * nodoMemSwap);
int recibirNodoDeMem(t_nodo_mem_swap * nodo);
void desempaquetarNodoDeMem(unsigned char *buffer,t_nodo_mem_swap * nodo);
int enviarMensajeRtaAMem(t_resp_swap_mem * nodo);
void empaquetarNodoRtaAMem(unsigned char *buffer,t_resp_swap_mem * nodo);

void escribir(t_envioPaginaSwap * nodoEscribir);
t_list * listaLibres;
t_list * listaProcesos;
t_list * listaEspera;

#endif /* SWAP_H_ */




