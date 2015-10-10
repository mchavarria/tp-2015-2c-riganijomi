#include "cpu.h"

int main(void) {

	//Levanta su configuracion y se prepara para conectarse al Planificador y al ADM
	cargarCfgs();

	//Prepara el archivo log al cual va a escribir
	archivoLog = log_create("cpu.log", "CPU", false, 2);


	//Se conecta al Planificador
	socketPlanificador = socketCrearCliente(puertoPlanificador, ipPlanificador);
    socketADM = socketCrearCliente(puertoADM, ipADM);
	if (socketADM == -1) {
		log_error(archivoLog, "Memoria no se pudo conectar");

	}
	if (socketPlanificador == -1) {
	    log_error(archivoLog, "Planificador no se pudo conectar");
	}

	//Voy a recibir la pcb
	pcbProc = malloc(sizeof(t_pcb));

	while ((socketRecibirMensaje(socketPlanificador, pcbProc,sizeof(t_pcb))) > 0){

		FILE * fp;
		char * linea = NULL;
		size_t len = 0;
		ssize_t read;

		fp = fopen(pcbProc->contextoEjecucion, "r");
		if (fp == NULL){
		  exit(EXIT_FAILURE);
		}
		while (((read = getline(&linea, &len, fp)) != -1) && (continuarLeyendo)) {

		  pc++; //actualiza el pgm counter
		  //Lee linea y ejecuta
		  interpretarLinea(linea);
		  sleep(retardo);

		}

		fclose(fp);
		if (linea){
		  free(linea);
		}
	}

	  return 0;
}

void interpretarLinea(char * linea) {

    char * valor;
    if (esElComando(linea, "iniciar")) {
		//valor = devolverParteUsable(line, 8);
		instruccionIniciarProceso (linea);

	} else if (esElComando(linea, "leer")) {
		//valor = devolverParteUsable(linea, 5);
		instruccionLeerPagina (linea);

	} else if (esElComando(linea, "entrada-salida")) {
		//valor= devolverParteUsable(linea, 15);
		instruccionEntradaSalida (linea);
	} else if (esElComando(linea, "escribir")) {
		char * resultado;
		//resultado = string_substring(linea, 9, 1);
		//valor = devolverParteUsable(linea, 11);
		instruccionEscribirPagina (linea);

	} else if (esElComando(linea, "finalizar")) {
		instruccionFinalizarProceso(linea);
	} else {
		perror("comando invaaaalido");
	}
}


/*
char * obtenerDirectorio(char * nombreArchivo) {
	char directorioActual[1024];
	getcwd(directorioActual, sizeof(directorioActual));
	strcat(directorioActual, nombreArchivo);
	return directorioActual;

}*/

void instruccionIniciarProceso (char * instruccion) {

	int exito;
	nodoRespuesta->tipo = INICIAR;
	nodoRespuesta->pc = pc;
	nodoRespuesta->idCPU = getpid();
	nodoRespuesta->valor = 0;
	nodoRespuesta->PID = pcbProc->PID;

	if (socketADM > 0){
		t_nodo_mem * nodoInstruccion = malloc(sizeof(t_nodo_mem));
		nodoInstruccion->pid = pcbProc->PID;
		strcpy(nodoInstruccion->instruccion,instruccion);
		//El socket está linkeado
		if (socketEnviarMensaje(socketADM, nodoInstruccion, sizeof(t_nodo_mem)) > 0){
			//Envíe el packete.. espero respuesta
			if (socketRecibirMensaje(socketADM, exito, sizeof(exito)) > 0){
				nodoRespuesta->exito = exito;

				if (socketPlanificador > 0){
					//El socket está linkeado
					socketEnviarMensaje(socketPlanificador,nodoRespuesta,sizeof(t_resp_cpu_plan));
				}

				if (exito != 1){
					continuarLeyendo = 0;
					log_error(archivoLog, "no se pudo iniciar el proceso: %d", pcbProc->PID);
				}
			}
		} else {
			log_error(archivoLog,"error envio mensaje memoria : %d",socketADM);
		}
		free(nodoInstruccion);
	}
}

void instruccionLeerPagina (char * instruccion) {
	int exito;
	nodoRespuesta->tipo = LEER;
	nodoRespuesta->pc = pc;
	nodoRespuesta->idCPU = getpid();

	nodoRespuesta->PID = pcbProc->PID;

	if (socketADM > 0){
		t_nodo_mem * nodoInstruccion = malloc(sizeof(t_nodo_mem));
		nodoInstruccion->pid = pcbProc->PID;
		strcpy(nodoInstruccion->instruccion,instruccion);

		if (socketEnviarMensaje(socketADM, nodoInstruccion, sizeof(t_nodo_mem)) > 0){
			if (socketRecibirMensaje(socketADM, exito,sizeof(exito))> 0){
				nodoRespuesta->exito = 1;
				nodoRespuesta->valor = exito;
				if (exito){
					char respuesta[exito];
					socketRecibirMensaje(socketADM, respuesta,sizeof(respuesta));
					if (socketPlanificador > 0){
						//El socket está linkeado
						socketEnviarMensaje(socketPlanificador,nodoRespuesta,sizeof(t_resp_cpu_plan));
						socketEnviarMensaje(socketPlanificador,respuesta, sizeof(respuesta));
						int numPag = devolverParteUsable(nodoInstruccion->instruccion,5);
						socketEnviarMensaje(socketPlanificador,numPag, sizeof(numPag));
					}
				}else {//exito if
				log_error(archivoLog, "no se pudo leer el proceso: %d", pcbProc->PID);
				}
			}//rcv adm
		} else{//enviar adm
			log_error(archivoLog,"error envio mensaje memoria : %d",socketADM);
		}
		free(nodoInstruccion);
	}
}

void instruccionEscribirPagina (char * instruccion) {
	/*char * directorioActual;
	char * puerto;
	directorioActual = obtenerDirectorio("/src/config.cfg");
	puts(directorioActual);
	puerto = configObtenerPuertoMemoria(directorioActual);
	puts(puerto);*/
}

void instruccionEntradaSalida (char * tiempo) {
}

void instruccionFinalizarProceso(char * instruccion) {
	int exito;
	nodoRespuesta->tipo = FINALIZAR;
	nodoRespuesta->pc = pc;
	nodoRespuesta->idCPU = getpid();
	nodoRespuesta->valor = 0;
	nodoRespuesta->PID = pcbProc->PID;

	t_nodo_mem * nodoInstruccion = malloc(sizeof(t_nodo_mem));
	nodoInstruccion->pid = pcbProc->PID;
	strcpy(nodoInstruccion->instruccion,instruccion);
	if (socketEnviarMensaje(socketADM, nodoInstruccion, sizeof(t_nodo_mem)) > 0){
		if (socketRecibirMensaje(socketADM, exito, sizeof(exito)) > 0){
			nodoRespuesta->exito = exito;

			if (socketPlanificador > 0){
				//El socket está linkeado
				socketEnviarMensaje(socketPlanificador,nodoRespuesta,sizeof(t_resp_cpu_plan));
			}
			if (exito != 1){
				log_error(archivoLog, "no se pudo finalizar el proceso: %d", pcbProc->PID);
			}
		}
	} else {
		log_error(archivoLog,"error envio mensaje memoria: %d",socketADM);
	}
	free(nodoInstruccion);

}


void cargarCfgs() {
	int a;
	getcwd(directorioActual, sizeof(directorioActual));
	//strcat(directorioActual, "/cpu/src/config.cfg");//para consola
	strcat(directorioActual, "/src/config.cfg"); //para eclipse

	ipPlanificador = configObtenerIpPlanificador(directorioActual);
	puertoPlanificador = configObtenerPuertoPlanificador(directorioActual);
	cantidadHilos = configObtenerCantidadHilos(directorioActual);
	//a = atoi(cantidadHilos);

	ipADM = configObtenerIpADM(directorioActual);
	puertoADM = configObtenerPuertoADM(directorioActual);
	retardo = 2;//configObtenerRetardo(directorioActual);
}






