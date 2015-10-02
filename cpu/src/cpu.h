#ifndef CPU_H_
#define CPU_H_

#include <stdint.h>


typedef struct NODO_MEM {
	uint32_t pid;
	char instruccion[20];
} __attribute__ ((packed)) t_nodo_mem;


typedef struct PCB {
	uint32_t processID;
	//int estado;
	//int programCounter;
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
/*
void cargarCfgs();
*/
#endif /* CPU_H_ */
