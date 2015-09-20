#ifndef CPU_H_
#define CPU_H_


//char * obtenerDirectorio(char * nombreArchivo);

void ejecutarInstrucciones();

void instruccionIniciarProceso (int * paginas);
void instruccionLeerPagina (char * resultado);
void instruccionEscribirPagina (int * pagina, char * texto);
void instruccionEntradaSalida (int * tiempo);
void instruccionFinalizarProceso();

void cargarCfgs();

#endif /* CPU_H_ */
