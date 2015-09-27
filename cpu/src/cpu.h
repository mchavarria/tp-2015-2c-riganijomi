#ifndef CPU_H_
#define CPU_H_

typedef struct PCB {
	int processID;
	//int estado;
	//int programCounter;
	char contextoEjecucion[100];
} __attribute__ ((packed)) t_pcb;

//char * obtenerDirectorio(char * nombreArchivo);

void ejecutarInstrucciones();

void instruccionIniciarProceso (char * paginas);
void instruccionLeerPagina (char * resultado);
void instruccionEscribirPagina (int * pagina, char * texto);
void instruccionEntradaSalida (char * tiempo);
void instruccionFinalizarProceso();
/*
void cargarCfgs();
*/
#endif /* CPU_H_ */
