#include "cpu.h"

int main() {

	//Levanta su configuracion y se prepara para conectarse al Planificador y al ADM
	cargarCfgs();

	//Prepara el archivo log al cual va a escribir
	archivoLog = log_create("cpu.log", "CPU", false, 2);

	//ACA HABRÍA QUE ABRIR TANTOS HILOS COMO CPUS HAYAN
	//Y CORRER LO QUE CONTINÚA EN UNA FUNCION PARA HILO
	hiloCPUs();


	return EXIT_SUCCESS;
}

void hiloCPUs() {
	//crear los hilos de las CPU
	//pthread_mutex_lock(&mutexCPUs);
	char *m1 = "cpu1";

	int i;
	listaHilosCPU = list_create();
	int cantCPUs = cantidadHilos;
	for (i = 0; i < cantCPUs; i++) {
		list_add(listaHilosCPU, hilos_create());
	}

	//crear Hilos
	t_hilos_CPU * nodoHilosCPU;
	for (i = 0; i < cantCPUs; i++) {
		nodoHilosCPU =  list_get(listaHilosCPU, i);
		pthread_create(&(nodoHilosCPU->hiloCPU), NULL, cpu_func, (void*) m1);
	}
	//pthread_mutex_unlock(&mutexCPU);
	//Esperar hilos
	for (i = 0; i < cantCPUs; i++) {
		pthread_join(nodoHilosCPU->hiloCPU, NULL );
	}
}

static t_hilos_CPU *hilos_create()
{
	t_hilos_CPU* new = malloc(sizeof(t_hilos_CPU));
	return new;
}

void cpu_func() {

	int idCPU = getpid();
	//Se conecta al Planificador
	socketPlanificador = socketCrearCliente(puertoPlanificador, ipPlanificador,"CPU","Planificador");
	if (socketPlanificador == -1) {
		log_error(archivoLog, "Planificador no se pudo conectar");
		exit(EXIT_FAILURE);
	} else {
		log_info(archivoLog, "CPU conectada al Planificador, idCpu: %d", idCPU);
	}

	//Se conecta al ADM
    socketADM = socketCrearCliente(puertoADM, ipADM,"CPU","Memoria");
	if (socketADM == -1) {
		log_error(archivoLog, "Memoria no se pudo conectar");
		exit(EXIT_FAILURE);
	} else {
		log_info(archivoLog, "CPU conectada a Memoria, idCpu: %d", idCPU);
	}

	//TODAS LAS CPUS VAN A HACER LO MISMO UNA VEZ CONECTADAS.
	//SE VAN A QUEDAR ESPERANDO ALGUN MENSAJE DEL PLANIFICADOR
	//DEBEN TENER UN SEMAFORO PARA COMPARTIR EL ADMINISTRADOR DE MEMORIA
	continuarLeyendo = 1;


	//Voy a recibir la pcb
	pcbProc = malloc(sizeof(t_pcb));
	pcbProc->PID = 0;
	int nbytes;
	int pcNuevoInicial = 0;
	int controlQuantum = 1;
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
		//Hasta aca el archivo

		fp = fopen(contextoEjecucion, "r");

		if (fp == NULL){
			printf("CPU: Archivo -%s- de PCB %d no válido", pcbProc->contextoEjecucion, pcbProc->PID);
			perror("Archivo no válido.");
		} else {
			while (((read = getline(&linea, &len, fp)) != -1) && (continuarLeyendo)) {

				if (pcNuevoInicial++ >= pcbProc->pc)
				{
					if (pcbProc->quantum == 0){
						//FIFO
						pcbProc->pc++;  //actualiza el pgm counter
						//Lee linea y ejecuta
						interpretarLinea(linea);
						sleep(retardo);
					} else {
						//RR
						if (controlQuantum++ < pcbProc->quantum){
							pcbProc->pc++;  //actualiza el pgm counter
							//Lee linea y ejecuta
							interpretarLinea(linea);
							sleep(retardo);
						}
					}
				}

				if ((pcbProc->quantum != 0) && (controlQuantum > pcbProc->quantum)){
					sacarPorQuantum();
					continuarLeyendo = 0;
				}
			}//while

			fclose(fp);
			if (linea){
			  free(linea);
			}
		}//fp != null
		pcNuevoInicial = 0;
		controlQuantum = 1;
		continuarLeyendo = 1;
	}//while rcv
}

void sacarPorQuantum(){

	nodoRtaCpuPlan = malloc(sizeof(t_resp_cpu_plan));

	int exito;
	nodoRtaCpuPlan->PID = pcbProc->PID;
	nodoRtaCpuPlan->idCPU = getpid();
	nodoRtaCpuPlan->tipo = QUANTUM_ACABADO;
	nodoRtaCpuPlan->pc = pcbProc->pc;
	nodoRtaCpuPlan->pagRW = 0;
	nodoRtaCpuPlan->respuesta = malloc(strlen("frula2")+1);
	strcpy(nodoRtaCpuPlan->respuesta,"\0");
	nodoRtaCpuPlan->exito = 1;

	if (socketPlanificador > 0){
		//El socket está linkeado
		int err = enviarMensajeRespuestaCPU(socketPlanificador,nodoRtaCpuPlan);
		log_info(archivoLog, "Instruccion: %d, Proceso: %d, CPU: %d, exito: %d ", nodoRtaCpuPlan->tipo,nodoRtaCpuPlan->PID, nodoRtaCpuPlan->idCPU, nodoRtaCpuPlan->exito);
	}
}

void interpretarLinea(char * linea) {
	strtok(linea,";");
	nodoRtaCpuPlan = malloc(sizeof(t_resp_cpu_plan));
	nodoInstruccion = malloc(sizeof(t_nodo_mem));
	nodoRta = malloc(sizeof(t_resp_swap_mem));
    if (esElComando(linea, "iniciar")) {
		instruccionIniciarProceso (linea);

	} else if (esElComando(linea, "leer")) {
		instruccionLeerPagina (linea);

	} else if (esElComando(linea, "entrada-salida")) {
		continuarLeyendo = 0;
		instruccionEntradaSalida (linea);
	} else if (esElComando(linea, "escribir")) {
		instruccionEscribirPagina (linea);

	} else if (esElComando(linea, "finalizar")) {
		continuarLeyendo = 0;
		instruccionFinalizarProceso(linea);
	} else {
		perror("comando invalido");
	}
    free(nodoRtaCpuPlan);
    free(nodoRta);
    free(nodoInstruccion);
}

void instruccionIniciarProceso (char * instruccion) {

	nodoRtaCpuPlan->PID = pcbProc->PID;
	nodoRtaCpuPlan->idCPU = getpid();
	nodoRtaCpuPlan->tipo = INICIAR;
	nodoRtaCpuPlan->pc = pcbProc->pc;
	nodoRtaCpuPlan->pagRW = retardo;
	nodoRtaCpuPlan->respuesta = malloc(strlen("frula2")+1);
	strcpy(nodoRtaCpuPlan->respuesta,"\0");
	nodoRtaCpuPlan->exito = 0;

	int err;
	nodoInstruccion->pid = pcbProc->PID;
	nodoInstruccion->instruccion = malloc(strlen("\0")+1);
	strcpy(nodoInstruccion->instruccion,instruccion);

	if ((err = enviarMensajeDeNodoAMem(nodoInstruccion)) > 0)
	{
		//Envíe el paquete.. espero respuesta
		nodoRta->contenido = malloc(strlen("\0")+1);
		strcpy(nodoRta->contenido,"\0");
		if ((err = recibirNodoDeMEM(nodoRta)) > 0)
		{
			nodoRtaCpuPlan->exito = nodoRta->exito;
			enviarMensajeRespuestaCPU(socketPlanificador,nodoRtaCpuPlan);

			if (nodoRta->exito != 1){
				continuarLeyendo = 0;
				log_error(archivoLog, "no se pudo iniciar el proceso: %d", pcbProc->PID);
			}
			log_info(archivoLog, "Instruccion: %d, Proceso: %d, CPU: %d, exito: %d ", nodoRtaCpuPlan->tipo,nodoRtaCpuPlan->PID, nodoRtaCpuPlan->idCPU, nodoRtaCpuPlan->exito);
		}
	}
}

void instruccionLeerPagina (char * instruccion) {
	nodoRtaCpuPlan->PID = pcbProc->PID;
	nodoRtaCpuPlan->idCPU = getpid();
	nodoRtaCpuPlan->tipo = LEER;
	nodoRtaCpuPlan->pc = pcbProc->pc;
	int numPag = devolverIntInstruccion(instruccion,5);
	nodoRtaCpuPlan->pagRW = numPag;
	nodoRtaCpuPlan->respuesta = malloc(strlen("\0")+1);
	strcpy(nodoRtaCpuPlan->respuesta,"\0");

	nodoInstruccion->pid = pcbProc->PID;
	strcpy(nodoInstruccion->instruccion,instruccion);
	int err;
	err = enviarMensajeDeNodoAMem(nodoInstruccion);

	nodoRta->contenido = malloc(strlen("\0")+1);
	strcpy(nodoRta->contenido,"\0");
	err = recibirNodoDeMEM(nodoRta);
	nodoRtaCpuPlan->exito = nodoRta->exito;
	strcpy(nodoRtaCpuPlan->respuesta,nodoRta->contenido);
	if (!nodoRta->exito){
		continuarLeyendo = 0;
		log_error(archivoLog, "no se pudo leer el proceso: %d", pcbProc->PID);
	}
	err = enviarMensajeRespuestaCPU(socketPlanificador, nodoRtaCpuPlan);
	log_info(archivoLog, "Instruccion: %d, Proceso: %d, CPU: %d, exito: %d ", nodoRtaCpuPlan->tipo,nodoRtaCpuPlan->PID, nodoRtaCpuPlan->idCPU, nodoRtaCpuPlan->exito);
}

void instruccionEscribirPagina (char * instruccion) {

	int exito;
	nodoRtaCpuPlan->PID = pcbProc->PID;
	nodoRtaCpuPlan->idCPU = getpid();
	nodoRtaCpuPlan->tipo = ESCRIBIR;
	nodoRtaCpuPlan->pc = pcbProc->pc;
	int numPag = devolverIntInstruccion(instruccion,9);
	nodoRtaCpuPlan->pagRW = numPag;
	nodoRtaCpuPlan->respuesta = malloc(strlen("\0")+1);
	strcpy(nodoRtaCpuPlan->respuesta,"\0");

	nodoInstruccion->pid = pcbProc->PID;
	strcpy(nodoInstruccion->instruccion,instruccion);
	int err;
	err = enviarMensajeDeNodoAMem(nodoInstruccion);

	nodoRta->contenido = malloc(strlen("\0")+1);
	strcpy(nodoRta->contenido,"\0");
	err = recibirNodoDeMEM(nodoRta);
	nodoRtaCpuPlan->exito = nodoRta->exito;
	enviarMensajeRespuestaCPU(socketPlanificador,nodoRtaCpuPlan);

	if (nodoRta->exito != 1){
		continuarLeyendo = 0;
	}//exito if
	log_info(archivoLog, "Instruccion: %d, Proceso: %d, CPU: %d, exito: %d ", nodoRtaCpuPlan->tipo,nodoRtaCpuPlan->PID, nodoRtaCpuPlan->idCPU, nodoRtaCpuPlan->exito);
}

void instruccionEntradaSalida (char * instruccion) {

	int exito;
	nodoRtaCpuPlan->PID = pcbProc->PID;
	nodoRtaCpuPlan->idCPU = getpid();
	nodoRtaCpuPlan->tipo = ENTRADA_SALIDA;
	nodoRtaCpuPlan->pc = pcbProc->pc;
	nodoRtaCpuPlan->pagRW = devolverParteUsableInt(instruccion, 15);
	nodoRtaCpuPlan->respuesta = malloc(strlen("")+1);
	strcpy(nodoRtaCpuPlan->respuesta,"\0");
	nodoRtaCpuPlan->exito = 1;

	int err = enviarMensajeRespuestaCPU(socketPlanificador,nodoRtaCpuPlan);
	log_info(archivoLog, "Instruccion: %d, Proceso: %d, CPU: %d, exito: %d ", nodoRtaCpuPlan->tipo,nodoRtaCpuPlan->PID, nodoRtaCpuPlan->idCPU, nodoRtaCpuPlan->exito);
}

void instruccionFinalizarProceso(char * instruccion) {
	int exito;
	nodoRtaCpuPlan->tipo = FINALIZAR;
	nodoRtaCpuPlan->pc = pcbProc->pc;
	nodoRtaCpuPlan->idCPU = getpid();
	nodoRtaCpuPlan->respuesta = malloc(strlen("\0")+1);
	strcpy(nodoRtaCpuPlan->respuesta,"\0");
	nodoRtaCpuPlan->PID = pcbProc->PID;
	nodoRtaCpuPlan->exito = 1;

	nodoInstruccion->pid = pcbProc->PID;
	nodoInstruccion->instruccion = malloc(strlen("\0")+1);
	strcpy(nodoInstruccion->instruccion,instruccion);
	int err;

	err = enviarMensajeDeNodoAMem(nodoInstruccion);

	err = enviarMensajeRespuestaCPU(socketPlanificador, nodoRtaCpuPlan);

	log_info(archivoLog, "Instruccion: %d, Proceso: %d, CPU: %d, exito: %d ", nodoRtaCpuPlan->tipo,nodoRtaCpuPlan->PID, nodoRtaCpuPlan->idCPU, nodoRtaCpuPlan->exito);
}


void cargarCfgs() {
	getcwd(directorioActual, sizeof(directorioActual));
	strcat(directorioActual, "/cpu/src/config.cfg");//para consola
	//strcat(directorioActual, "/src/config.cfg"); //para eclipse

	ipPlanificador = configObtenerIpPlanificador(directorioActual);
	puertoPlanificador = configObtenerPuertoPlanificador(directorioActual);
	cantidadHilos = configObtenerCantidadHilos(directorioActual);
	ipADM = configObtenerIpADM(directorioActual);
	puertoADM = configObtenerPuertoADM(directorioActual);
	retardo = configObtenerRetardoCPU(directorioActual);
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
	unpack(buffer,SECUENCIA_PCB,&nodoPCB->PID,&nodoPCB->estado,&nodoPCB->pc,&nodoPCB->quantum,&nodoPCB->totalInstrucciones,programa);

	nodoPCB->contextoEjecucion = programa;
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
}


int enviarMensajeDeNodoAMem(t_nodo_mem * nodo)
{
	int nbytes;
	int pid = getpid();
	unsigned char buffer[100];
	empaquetarNodoMemCPU(buffer,nodo);
	nbytes = send(socketADM, buffer, sizeof(buffer) , 0);
	if (nbytes == 0) {
		printf("CPU %d: Socket Memoria %d desconectado.\n", pid, socketADM);
		//TODO CERRAR LA CPU
	} else if (nbytes < 0){
		printf("CPU %d: Socket Memoria %d envío de mensaje fallido.\n", pid, socketADM);
		perror("Error - Enviando mensaje");
	}
	return nbytes;
}

void empaquetarNodoMemCPU(unsigned char *buffer,t_nodo_mem * nodo)
{
	unsigned int tamanioBuffer;
	tamanioBuffer = pack(buffer,SECUENCIA_CPU_MEM,nodo->pid,nodo->instruccion);
}


int recibirNodoDeMEM(t_resp_swap_mem * nodo)
{
	unsigned char buffer[1024];
	int nbytes;
	int pidCpu = getpid();
	if ((nbytes = recv(socketADM , buffer , sizeof(buffer) , 0)) < 0){
		printf("CPU %d: Error recibiendo mensaje de Memoria", pidCpu);
	} else if (nbytes == 0) {
		printf("CPU %d: Socket Memoria desconectado", pidCpu);
		//TODO CERRAR LA CPU
	}
	desempaquetarNodoMem(buffer,nodo);
	return nbytes;
}



void desempaquetarNodoMem(unsigned char *buffer,t_resp_swap_mem * nodo)
{
	//t_pcb * pcb = malloc(sizeof(t_pcb));
	char programa[50];
	unpack(buffer,SECUENCIA_NODO_RTA_SWAP_MEM,&nodo->tipo,&nodo->exito,&nodo->pagina,programa);

	nodo->contenido = programa;
}



