#ifndef CPU_H_
#define CPU_H_

#include <stdint.h>

typedef struct PCB {
	uint32_t processID;
	//int estado;
	//int programCounter;
	char contextoEjecucion[100];
} __attribute__ ((packed)) t_pcb;

//char * obtenerDirectorio(char * nombreArchivo);

void ejecutarInstrucciones();
void cargarCfgs();

void instruccionIniciarProceso (char * paginas);
void instruccionLeerPagina (char * resultado);
void instruccionEscribirPagina (int * pagina, char * texto);
void instruccionEntradaSalida (char * tiempo);
void instruccionFinalizarProceso();
void interpretarLinea(char * line);
/*
void cargarCfgs();
*/
#endif /* CPU_H_ */
