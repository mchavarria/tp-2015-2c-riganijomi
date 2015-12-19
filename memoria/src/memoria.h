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
#include "serializacion.h"

#define INICIAR 1
#define LEER 2
#define ESCRIBIR 3
#define FINALIZAR 6
#define ERRONEA 15
#define COMPACTACION 10
#define NULO 1000000
#define SECUENCIA_NODO_RTA_SWAP_MEM "hhhs"
#define SECUENCIA_MEM_SWAP "hhhs"

#define SECUENCIA_CPU_MEM "hhhs"

#define LRU 50


//CFG
char* PUERTO_SWAP;
char* IP_SWAP;
char* PUERTO_ESCUCHA;
double RETARDO_MEMORIA;
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
double comandosTotales = 0;
double aciertosTLB = 0;
int pageFaultPa = 0;
int bitIngresoFIFO = 0;
int encontradoEnTLB = 1;
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
	int tipo;
	int exito;
	int pagina;
	char *contenido;
} t_resp_swap_mem;


typedef struct NODO_MEM_SWAP {
	int tipo;
	int pid;
	int pagina;
	char *contenido;
} t_nodo_mem_swap;

typedef struct NODO_MEM {
	int pid;
	int pagina;
	int instruccion;
	char *texto;
} t_nodo_mem;

typedef struct marcosLibrs {
	uint32_t numeroMarco;
} __attribute__ ((packed)) t_marcoLibre;

typedef struct TLB{
	uint32_t processID;
	uint32_t numeroPagina;
	uint32_t marco;
} __attribute__ ((packed)) t_tlb;

void imprimirTLB();

int pageFaultLectura = 0;

typedef struct tablaPaginasProceso {
	uint32_t numeroPagina;
	uint32_t numeroMarco;
	uint32_t ingreso;
} __attribute__ ((packed)) t_tablaPaginasProceso;

typedef struct tablasPaginas {
	uint32_t processID;
	t_list * listaPaginas;
	int contadorPageFault;
	int paginasAccedidas;
} t_tablasPaginas;

typedef struct marco {
	uint32_t processID;
	char *valor;
	uint32_t numeroMarco;
	uint32_t numeroPagina;
	uint32_t bitModificacion;
	uint32_t bitLeido;
	uint32_t punteroClock;
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

t_list * listaMarcosLibres;

t_list * listaTablasPaginas;

t_list * listaMarco;

t_list * listaTLB;

sem_t mutexFlushTLB;
sem_t mutexFlushMarcos;
sem_t productorSwap;

t_resp_swap_mem * nodoRespuesta;
t_nodo_mem * nodoInstruccion;
t_nodo_mem_swap * nodoASwap;
t_resp_swap_mem * nodoRtaSwap;
void rutina (int n);
void levantarCfgInicial();
void configurarSockets();

int interpretarLinea(t_nodo_mem * nodoInst);
void inicializarTLB();
void inicializarTablaDePaginas();
void inicializarMarco();
void actualizarNodoPaginas(int indiceMarco, int processID, int numeroPagina);
void actualizarMarco(char * texto,int pid, int numeroPagina, int indiceMarco, int tipo);
int algoritmoReemplazo(int processID);
int algoritmoReemplazoFIFO(int processID);
int algoritmoReemplazoLRU(int processID);
void desasignarMarco(int processID, int marco);
void escribirMarco(int processID, int marco, char * texto, int numeroPagina,int tipo);
static t_tablaPaginasProceso * obtenerPaginaPorNumMarco(int marco, t_tablasPaginas * nodoTablasPagina);
void cargarTlb(t_nodo_mem * nodoInstruccion, t_marco * marco, int pid);
void modificarBitIngresoLRU (int pid);

void flushMarcosActivacion();
void* flushTLB();
void* flushMarcos();
void desasignarTodosLosProcesos();
void flushTLBActivacion();
void configurarSockets();
//SERIALIZACION
int recibirNodoDeCPU(t_nodo_mem * nodo);
void desempaquetarNodoInstruccion(unsigned char *buffer,t_nodo_mem * nodo);
int enviarMensajeDeNodoACPU(t_resp_swap_mem * nodo);
void empaquetarNodoRtaCPU(unsigned char *buffer,t_resp_swap_mem * nodo);
int enviarMensajeDeNodoASWAP(t_nodo_mem_swap * nodo);
void empaquetarNodoMemSWAP(unsigned char *buffer,t_nodo_mem_swap * nodo);
int recibirNodoDeRtaSwap(t_resp_swap_mem * nodo);
void desempaquetarNodoRtaSwap(unsigned char *buffer,t_resp_swap_mem * nodo);
static t_marco * detectarPageFault(t_nodo_mem * nodoInst, int numeroPagina);
static t_marco * marco_create(int numeroMarco);
static t_tlb * tlb_create();

typedef struct CPU {
	int pid;
	int socket;
} t_cpu;

t_list * listaDeCPUs;
t_list * listaSolicitudes;
t_list * listaRespuestasEnEspera;
pthread_mutex_t prioridadEspera;
sem_t cantSolicitudes;
void informarDesconexionCPU(int socketCPU);
void* monitorearSockets();
void recibirSolicitudCPU();
void agregarCPUALista(int socketCpu);
void* atenderSolicitudes();
void recibirSolicitudDeCpu(int socket, int * nbytes);
int obtenerPaginaLeeroEscribir(char * linea);
static t_marco * seleccionarMarcoVictima(int pid);
void finalizarProceso(int pid);
void modificarBitsClock (int pid, t_list * listaMarcos,int indiceMarco);
int algoritmoReemplazoClock(int processID);
void * calcularTasaAciertos();
void* recibirRespuestasCompactacion(int tiempoEspera);
int atenderRespuestaEnCola(t_nodo_mem * nodoInst);

#endif /* MEMORIA_H_ */


