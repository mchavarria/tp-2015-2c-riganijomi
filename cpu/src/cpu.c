#include "cpu.h"

int main() {

	//Levanta su configuracion y se prepara para conectarse al Planificador y al ADM
	cargarCfgs();

	//Prepara el archivo log al cual va a escribir
	archivoLog = log_create("cpu.log", "CPU", false, 2);

	//ACA HABRÍA QUE ABRIR TANTOS HILOS COMO CPUS HAYAN
	//Y CORRER LO QUE CONTINÚA EN UNA FUNCION PARA HILO

	//Se conecta al Planificador
	socketPlanificador = socketCrearCliente(puertoPlanificador, ipPlanificador,"CPU","Planificador");
	if (socketPlanificador == -1) {
		log_error(archivoLog, "Planificador no se pudo conectar");
		return -1;
	}

    socketADM = socketCrearCliente(puertoADM, ipADM,"CPU","Memoria");
	if (socketADM == -1) {
		log_error(archivoLog, "Memoria no se pudo conectar");
		return -1;
	}

	//TODAS LAS CPUS VAN A HACER LO MISMO UNA VEZ CONECTADAS.
	//SE VAN A QUEDAR ESPERANDO ALGUN MENSAJE DEL PLANIFICADORS
	//DEBEN TENER UN SEMAFORO PARA COMPARTIR EL ADMINISTRADOR DE MEMORIA
	int continuarLeyendo = 1;


	//Voy a recibir la pcb
	pcbProc = malloc(sizeof(t_pcb));
	pcbProc->PID = 0;
	int nbytes;
	while ((nbytes = recibirPCBdePlanificador(pcbProc)) > 0){

		FILE * fp;
		char * linea = NULL;
		size_t len = 0;
		ssize_t read;

		//Configuro el archivo para abrir el mProg
		char *mprog = malloc(strlen(pcbProc->contextoEjecucion)+8+1);
		strcpy(mprog,"/mProgs/");
		strcat(mprog,pcbProc->contextoEjecucion);

		char *dir = getcwd(NULL, 0);
		char *contextoEjecucion = malloc(strlen(dir)+strlen(mprog)+1);
		strcpy(contextoEjecucion,dir);
		strcat(contextoEjecucion,mprog);
		//Hasta aca el arlchivo

		fp = fopen(contextoEjecucion, "r");

		if (fp == NULL){
			printf("CPU: Archivo -%s- de PCB %d no válido", pcbProc->contextoEjecucion, pcbProc->PID);
			perror("Archivo no válido.");
		} else {
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
		} // If archivo leido
	} //if pcb != NULL

	  return 0;
}

void interpretarLinea(char * linea) {
	strtok(linea,";");
	nodoRtaCpuPlan = malloc(sizeof(t_resp_cpu_plan));
    if (esElComando(linea, "iniciar")) {
		instruccionIniciarProceso (linea);

	} else if (esElComando(linea, "leer")) {
		instruccionLeerPagina (linea);

	} else if (esElComando(linea, "entrada-salida")) {
		instruccionEntradaSalida (linea);
	} else if (esElComando(linea, "escribir")) {
		instruccionEscribirPagina (linea);

	} else if (esElComando(linea, "finalizar")) {
		instruccionFinalizarProceso(linea);
	} else {
		perror("comando invalido");
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
	nodoRtaCpuPlan->PID = pcbProc->PID;
	nodoRtaCpuPlan->idCPU = getpid();
	nodoRtaCpuPlan->tipo = INICIAR;
	nodoRtaCpuPlan->pc = pc;
	nodoRtaCpuPlan->pagRW = 0;
	char resp[1];
	nodoRtaCpuPlan->respuesta = resp;
	nodoRtaCpuPlan->exito = 0;

	if (socketADM > 0){
		t_nodo_mem * nodoInstruccion = malloc(sizeof(t_nodo_mem));
		nodoInstruccion->pid = pcbProc->PID;
		strcpy(nodoInstruccion->instruccion,instruccion);
		//El socket está linkeado
		if (socketEnviarMensaje(socketADM, nodoInstruccion, sizeof(t_nodo_mem)) > 0){
			//Envíe el packete.. espero respuesta
			if (socketRecibirMensaje(socketADM, exito, sizeof(exito)) > 0){
				nodoRtaCpuPlan->exito = exito;

				if (socketPlanificador > 0){
					//El socket está linkeado
					int err = enviarMensajeRespuestaCPU(socketPlanificador,nodoRtaCpuPlan);
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
	nodoRtaCpuPlan->PID = pcbProc->PID;
	nodoRtaCpuPlan->idCPU = getpid();
	nodoRtaCpuPlan->tipo = LEER;
	nodoRtaCpuPlan->pc = pc;
	int numPag = devolverIntInstruccion(instruccion,5);
	nodoRtaCpuPlan->pagRW = numPag;

	if (socketADM > 0){
		t_nodo_mem * nodoInstruccion = malloc(sizeof(t_nodo_mem));
		nodoInstruccion->pid = pcbProc->PID;
		strcpy(nodoInstruccion->instruccion,instruccion);

		if (socketEnviarMensaje(socketADM, nodoInstruccion, sizeof(t_nodo_mem)) > 0){
			if (socketRecibirMensaje(socketADM, exito,sizeof(exito))> 0){
				if (exito){
					nodoRtaCpuPlan->exito = 1;
					char respuesta[exito];
					socketRecibirMensaje(socketADM, respuesta,sizeof(respuesta));
					if (socketPlanificador > 0){
						//El socket está linkeado
						strcpy(nodoRtaCpuPlan->respuesta,respuesta);
					}
				}else {
					nodoRtaCpuPlan->exito = 0;
					char * respuesta = "\0";
					nodoRtaCpuPlan->respuesta = respuesta;
					log_error(archivoLog, "no se pudo leer el proceso: %d", pcbProc->PID);
				}//exito if
				int err = enviarMensajeRespuestaCPU(socketPlanificador, nodoRtaCpuPlan);
			}//rcv adm
		} else{
			log_error(archivoLog,"error envio mensaje memoria : %d",socketADM);
		}//enviar adm
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
	//en el pagRW poner el valor del Tiempo de E/S
}

void instruccionFinalizarProceso(char * instruccion) {
	int exito;
	nodoRtaCpuPlan->tipo = FINALIZAR;
	nodoRtaCpuPlan->pc = pc;
	nodoRtaCpuPlan->idCPU = getpid();
	strcpy(nodoRtaCpuPlan->respuesta,"");
	nodoRtaCpuPlan->PID = pcbProc->PID;

	t_nodo_mem * nodoInstruccion = malloc(sizeof(t_nodo_mem));
	nodoInstruccion->pid = pcbProc->PID;
	strcpy(nodoInstruccion->instruccion,instruccion);
	if (socketEnviarMensaje(socketADM, nodoInstruccion, sizeof(t_nodo_mem)) > 0){
		if (socketRecibirMensaje(socketADM, exito, sizeof(exito)) > 0){
			nodoRtaCpuPlan->exito = exito;

			if (socketPlanificador > 0){
				//El socket está linkeado
				socketEnviarMensaje(socketPlanificador,nodoRtaCpuPlan,sizeof(t_resp_cpu_plan));
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
	strcat(directorioActual, "/cpu/src/config.cfg");//para consola
	//strcat(directorioActual, "/src/config.cfg"); //para eclipse

	ipPlanificador = configObtenerIpPlanificador(directorioActual);
	puertoPlanificador = configObtenerPuertoPlanificador(directorioActual);
	cantidadHilos = configObtenerCantidadHilos(directorioActual);
	//a = atoi(cantidadHilos);

	ipADM = configObtenerIpADM(directorioActual);
	puertoADM = configObtenerPuertoADM(directorioActual);
	retardo = 2;//configObtenerRetardo(directorioActual);
}

int recibirPCBdePlanificador(t_pcb * nodoPCB){
	unsigned char buffer[1024];
	int pidCpu = getpid();
	int nbytes;
	if ((nbytes = recv(socketPlanificador , buffer , sizeof(buffer) , 0)) < 0){
		printf("CPU %d: Error recibiendo mensaje de Planificador", pidCpu);
	} else if (nbytes == 0) {
		printf("CPU %d: Socket Planificador desconectado", pidCpu);
		//CERRAR LA CPU
	}
	desempaquetarPCB(buffer,nodoPCB);
	return nbytes;
}



void desempaquetarPCB(unsigned char *buffer,t_pcb * nodoPCB){

	//t_pcb * pcb = malloc(sizeof(t_pcb));
	char programa[50];
	unpack(buffer,SECUENCIA_PCB,&nodoPCB->PID,&nodoPCB->estado,&nodoPCB->pc,&nodoPCB->quantum,programa);

	nodoPCB->contextoEjecucion = programa;
	/*
	 * printf("PCB recibido: PID: %d,  Estado: %d, PC: %d, Quantum: %d, Archivo: %s\n",
			nodoPCB->PID,nodoPCB->estado,nodoPCB->pc,nodoPCB->quantum,nodoPCB->contextoEjecucion);

	 EJEMPLO DE ENVIO / RECEPCION PARA PROBAR EN MAIN
	 t_pcb * pcb1 = malloc(sizeof(t_pcb));
	pcb1->PID = 0;
	pcb1->contextoEjecucion = malloc(strlen("programa3.cod"));
	strcpy(pcb1->contextoEjecucion,"programa3.cod");
	pcb1->pc=3;
	pcb1->estado=LISTO;
	pcb1->quantum=5;
	unsigned char buffer[1024];
	empaquetarPCB(buffer,pcb1);
	puts("pcb enviado.");

	t_pcb * pcb = malloc(sizeof(t_pcb));
	desempaquetarPCB(buffer,pcb);
	 */
}

int enviarMensajeRespuestaCPU(int socketPlanificador, t_resp_cpu_plan * nodoRta) {
	int nbytes;
	unsigned char buffer[1024];
	empaquetarNodoRtaCpuPlan(buffer,nodoRta);
	nbytes = send(socketPlanificador, buffer, sizeof(buffer) , 0);
	if (nbytes == 0) {
		printf("CPU: Socket Planificador %d desconectado.\n", socketPlanificador);
	} else if (nbytes < 0){
		printf("CPU: Socket Planificador %d envío de mensaje fallido.\n", socketPlanificador);
		perror("Error - Enviando mensaje");
	}
	return nbytes;
}

void empaquetarNodoRtaCpuPlan(unsigned char *buffer,t_resp_cpu_plan * nodoRta){
	unsigned int tamanioBuffer;

	tamanioBuffer = pack(buffer,SECUENCIA_NODO_RTA_CPU_PLAN,
			nodoRta->PID,nodoRta->idCPU,nodoRta->tipo,nodoRta->exito,
			nodoRta->pagRW,nodoRta->pc,nodoRta->respuesta);

	//packi16(buffer+1, tamanioBuffer); // store packet size in packet for kicks
}




