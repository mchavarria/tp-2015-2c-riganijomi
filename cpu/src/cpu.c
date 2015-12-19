#include "cpu.h"

int main() {
	pthread_mutex_init(&mutexOrdenCpu, NULL);
	//Levanta su configuracion y se prepara para conectarse al Planificador y al ADM
	cargarCfgs();

	//Prepara el archivo log al cual va a escribir

	archivoLog = log_create("cpu.log", "CPU", true, 2);

	//ACA HABRÍA QUE ABRIR TANTOS HILOS COMO CPUS HAYAN
	//Y CORRER LO QUE CONTINÚA EN UNA FUNCION PARA HILO
	//crear los hilos de las CPU
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
		pthread_create(&(nodoHilosCPU->hiloCPU), NULL, cpu_func, (void*) i);
		usleep(500000);
	}
	//Esperar hilos
	for (i = 0; i < cantCPUs; i++) {
		pthread_join(nodoHilosCPU->hiloCPU, NULL );
	}
	return 1;
}


static t_hilos_CPU *hilos_create()
{
	t_hilos_CPU* new = malloc(sizeof(t_hilos_CPU));
	return new;
}

void* cpu_func() {
	int socketPlanificador = 0;

	int idCPU = process_get_thread_id();
	//Se conecta al Planificador
	pthread_mutex_lock(&mutexOrdenCpu);

	socketPlanificador = socketCrearCliente(puertoPlanificador, ipPlanificador,"CPU","Planificador");
	if (socketPlanificador == -1) {
		log_error(archivoLog, "CPU %d: Planificador no se pudo conectar", idCPU);
		exit(EXIT_FAILURE);
	}
	t_info_cpu * nodoInfoCpu = malloc(sizeof(t_info_cpu));
	nodoInfoCpu->idCPU = idCPU;
	nodoInfoCpu->retardo = retardo;
	enviarMensajeInfoCPU(socketPlanificador,nodoInfoCpu);
	//Se conecta al ADM
    int socketADM = socketCrearCliente(puertoADM, ipADM,"CPU","Memoria");
	if (socketADM == -1) {
		log_error(archivoLog, "CPU %d: Memoria no se pudo conectar", idCPU);
		exit(EXIT_FAILURE);
	}
	enviarMensajeInfoCPU(socketADM,nodoInfoCpu);
	free(nodoInfoCpu);
	pthread_mutex_unlock(&mutexOrdenCpu);
	//TODAS LAS CPUS VAN A HACER LO MISMO UNA VEZ CONECTADAS.
	//SE VAN A QUEDAR ESPERANDO ALGUN MENSAJE DEL PLANIFICADOR
	//DEBEN TENER UN SEMAFORO PARA COMPARTIR EL ADMINISTRADOR DE MEMORIA
	t_pcb * pcbProc;
	//Voy a recibir la pcb
	pcbProc = malloc(sizeof(t_pcb));
	pcbProc->contextoEjecucion = string_new();
	int nbytes;
	int imprimirLineas(char * li){
		puts(li);
	}
	while (1) {
		nbytes = recibirPCBdePlanificador(pcbProc,socketPlanificador);
		t_list * instruccionesParaEjecutar = list_create();
		int archivoValido = comprobarArchivo(pcbProc);
		if (archivoValido == 1){
			obtenerInstruccionesSegunQuantum(pcbProc,instruccionesParaEjecutar);
			//list_iterate(instruccionesParaEjecutar,(void *)imprimirLineas);
			int instruccion;
			int continuarLeyendo = 1;
			for (instruccion = 0; (instruccion < instruccionesParaEjecutar->elements_count && continuarLeyendo == 1); instruccion++){
				sleep(retardo);
				pcbProc->pc++;  //actualiza el pgm counter
				continuarLeyendo = interpretarLineaAejecutar(list_get(instruccionesParaEjecutar,instruccion),pcbProc,socketADM,socketPlanificador);
			}
			if ((pcbProc->totalInstrucciones != pcbProc->quantum) && (continuarLeyendo == 1)){
				//Debo sacarlo por QUANTUM
				sacarPorQuantum(pcbProc,socketPlanificador);
			}
		} else {
			//Archivo no valido
			notificarNoInicioPCB(pcbProc,socketPlanificador);//Avisa que no lo inicia
		}
	}
}

void obtenerInstruccionesSegunQuantum(t_pcb * pcb, t_list * lista){
	int pcNuevoInicial = 0;
	int controlQuantum;
	int pcAux = pcb->pc; //Emula ir ejecutando las isntruccioneos
	char * linea = NULL;
	size_t len = 0;
	ssize_t read;
	FILE * fp = abrirArchivo(pcb->contextoEjecucion);
	//arhico valido
	for (controlQuantum = 0; controlQuantum < pcb->quantum;){
		read = getline(&linea, &len, fp);
		if (read != -1) {
			//Hay lineas en el archivo
			if (pcNuevoInicial == pcAux){
				//Corresponde leer la linea
				//La agrega
				char * inst = malloc(strlen(linea)+1);
				strcpy(inst,linea);
				list_add(lista,inst);
				controlQuantum++;
				pcAux++;
			}
			pcNuevoInicial++;
		} else {
			//Finaliza el archivo, salgo del for
			controlQuantum = pcb->quantum;
		}
	}
	if (linea){
	  free(linea);
	}
}

int comprobarArchivo(t_pcb * pcb)
{
	int respuesta = 1;
	FILE * fp = abrirArchivo(pcb->contextoEjecucion);
	if (fp == NULL){
		printf("CPU: Archivo -%s- de PCB %d no válido", pcb->contextoEjecucion, pcb->PID);
		respuesta = 0;
	} else {
		fclose(fp);
	}
	return respuesta;
}

FILE * abrirArchivo(char *programa){
	FILE * fp;
	//Configuro el archivo para abrir el mProg
	char *mprog = malloc(strlen(programa)+8+1);
	strcpy(mprog,"/mProgs/");
	strcat(mprog,programa);

	char *dir = getcwd(NULL, 0);
	char *contextoEjecucion = malloc(strlen(dir)+strlen(mprog)+1);
	strcpy(contextoEjecucion,dir);
	strcat(contextoEjecucion,mprog);
	//Hasta aca el archivo
	fp = fopen(contextoEjecucion, "r");
	return fp;

}

void notificarNoInicioPCB(t_pcb * pcbProc,int socketPlanificador){
	t_resp_cpu_plan * nodoRtaCpuPlan;
	nodoRtaCpuPlan = malloc(sizeof(t_resp_cpu_plan));

	nodoRtaCpuPlan->PID = pcbProc->PID;
	nodoRtaCpuPlan->idCPU = idCPU;
	nodoRtaCpuPlan->tipo = INICIAR;
	nodoRtaCpuPlan->pc = pcbProc->pc;
	nodoRtaCpuPlan->pagRW = retardo;
	nodoRtaCpuPlan->respuesta = malloc(1);
	strcpy(nodoRtaCpuPlan->respuesta,"\0");
	nodoRtaCpuPlan->exito = 0;

	if (socketPlanificador > 0){
		//El socket está linkeado
		pthread_mutex_lock(&mutexOrdenCpu);
		int err = enviarMensajeRespuestaCPU(socketPlanificador,nodoRtaCpuPlan);
		pthread_mutex_unlock(&mutexOrdenCpu);

	}
	free(nodoRtaCpuPlan);
}

void sacarPorQuantum(t_pcb * pcbProc,int socketPlanificador){
	t_resp_cpu_plan * nodoRtaCpuPlan;
	nodoRtaCpuPlan = malloc(sizeof(t_resp_cpu_plan));
	int exito;
	nodoRtaCpuPlan->PID = pcbProc->PID;
	nodoRtaCpuPlan->idCPU = pcbProc->CPU;
	nodoRtaCpuPlan->tipo = QUANTUM_ACABADO;
	nodoRtaCpuPlan->pc = pcbProc->pc;
	nodoRtaCpuPlan->pagRW = 0;
	nodoRtaCpuPlan->respuesta = string_new();
	nodoRtaCpuPlan->exito = 1;

	if (socketPlanificador > 0){
		//El socket está linkeado
		pthread_mutex_lock(&mutexOrdenCpu);
		int err = enviarMensajeRespuestaCPU(socketPlanificador,nodoRtaCpuPlan);
		pthread_mutex_unlock(&mutexOrdenCpu);
		log_info(archivoLog, "CPU: %d, Proceso: %d - Instruccion: %s (%s) ", nodoRtaCpuPlan->idCPU, nodoRtaCpuPlan->PID,traducirInstruccion(nodoRtaCpuPlan->tipo),traducirExitoStatus(nodoRtaCpuPlan->exito));
	}
	free(nodoRtaCpuPlan);
}

int interpretarLineaAejecutar(char * linea,t_pcb * pcbProc,int socketADM,int socketPlanificador) {
	strtok(linea,";");
	int exito = 0;
	int continuar = 1;
	//Respuesta del CPU al Planificador
	t_resp_cpu_plan * nodoRtaCpuPlan;
	nodoRtaCpuPlan = malloc(sizeof(t_resp_cpu_plan));
	nodoRtaCpuPlan->PID = pcbProc->PID;
	nodoRtaCpuPlan->idCPU = pcbProc->CPU;
	nodoRtaCpuPlan->pc = pcbProc->pc;
	nodoRtaCpuPlan->pagRW = 0;
	nodoRtaCpuPlan->respuesta = string_new();
	//Mensaje para el ADM
	t_nodo_mem * nodoInstruccion;
	nodoInstruccion = malloc(sizeof(t_nodo_mem));
	nodoInstruccion->pid = pcbProc->PID;
	nodoInstruccion->socketCpu = 0;
	nodoInstruccion->texto = string_new();
	//El ADM me reenvia la respuesta del SWAP
	t_resp_swap_mem * nodoRta;
	nodoRta = malloc(sizeof(t_resp_swap_mem));
	nodoRta->exito = 0;//Default por si falla una comunicacion con ADM
	nodoRta->contenido = string_new();
	pthread_mutex_lock(&mutexOrdenCpu);
    if (esElComando(linea, "iniciar")) {
    	nodoInstruccion->instruccion = INICIAR;
		nodoInstruccion->pagina = devolverParteUsableInt(linea, 8);
		exito = instruccionIniciarProceso(nodoRta,nodoInstruccion,socketADM);
		if (exito != -1){
			//Se pudo comunicar bien
			if (nodoRta->tipo == COMPACTACION){
				//printf("CPU %d: El swap se compacta",process_get_thread_id());
				pthread_mutex_unlock(&mutexOrdenCpu);//libero para que otra cpu pueda interactuar con su MEM
				int nbytes;
				while ((nbytes = recibirNodoDeMEM(socketADM,nodoRta))< 0){
					//Sigue esperando su respuesta
				}
				pthread_mutex_lock(&mutexOrdenCpu);
			}
			nodoRtaCpuPlan->exito = nodoRta->exito;
		}
		nodoRtaCpuPlan->tipo = INICIAR;
		nodoRtaCpuPlan->pagRW = retardo;
		if (nodoRta->exito != 1){
			continuar = 0;
			log_error(archivoLog, "no se pudo iniciar el proceso: %d", pcbProc->PID);
		}
	} else if (esElComando(linea, "leer")) {
		nodoInstruccion->instruccion = LEER;
		nodoInstruccion->pagina = devolverParteUsableInt(linea, 5);
		exito = instruccionLeerPagina(nodoRta,nodoInstruccion,socketADM);
		if (exito != -1){
			//Se pudo comunicar bien
			if (nodoRta->tipo == COMPACTACION){
				pthread_mutex_unlock(&mutexOrdenCpu);//libero para que otra cpu pueda interactuar con su MEM
				int nbytes;
				while ((nbytes = recibirNodoDeMEM(socketADM,nodoRta))< 0){
					//Sigue esperando su respuesta
				}
				pthread_mutex_lock(&mutexOrdenCpu);
			}
			nodoRtaCpuPlan->exito = nodoRta->exito;
			strcpy(nodoRtaCpuPlan->respuesta,nodoRta->contenido);
		}
		nodoRtaCpuPlan->tipo = LEER;
		nodoRtaCpuPlan->pagRW = nodoInstruccion->pagina;
		if (nodoRta->exito != 1){
			continuar = 0;
			log_error(archivoLog, "no se pudo leer el proceso: %d, pagina %d", pcbProc->PID,nodoInstruccion->pagina);
		}

	} else if (esElComando(linea, "entrada-salida")) {
		nodoRtaCpuPlan->tipo = ENTRADA_SALIDA;
		nodoRtaCpuPlan->pagRW = devolverParteUsableInt(linea, 15);
		nodoRtaCpuPlan->exito = 1;
		continuar = 0;
	} else if (esElComando(linea, "escribir")) {
		nodoInstruccion->instruccion = ESCRIBIR;
		nodoInstruccion->pagina = valorPaginaEnEscritura(linea);
		textoAEscribir(linea,nodoInstruccion->texto);
		exito = instruccionEscribirPagina(nodoRta,nodoInstruccion,socketADM);
		if (exito != -1){
			if (nodoRta->tipo == COMPACTACION){
				pthread_mutex_unlock(&mutexOrdenCpu);//libero para que otra cpu pueda interactuar con su MEM
				int nbytes;
				while ((nbytes = recibirNodoDeMEM(socketADM,nodoRta))< 0){
					//Sigue esperando su respuesta
				}
				pthread_mutex_lock(&mutexOrdenCpu);
			}
			//Se pudo comunicar bien
			nodoRtaCpuPlan->exito = nodoRta->exito;
		}
		nodoRtaCpuPlan->tipo = ESCRIBIR;
		if (nodoRta->exito != 1){
			continuar = 0;
			log_error(archivoLog, "no se pudo escribir el proceso: %d, pagina %d, contenido %s", pcbProc->PID,nodoInstruccion->pagina,nodoInstruccion->texto);
		}

	} else if (esElComando(linea, "finalizar")) {
		nodoInstruccion->instruccion = FINALIZAR;
		nodoInstruccion->pagina = 0;
		instruccionFinalizarProceso(nodoInstruccion,socketADM);
		nodoRtaCpuPlan->tipo = FINALIZAR;
		nodoRtaCpuPlan->exito = 1;
		continuar = 0;
	} else {
		perror("comando invalido");
	}
    pthread_mutex_unlock(&mutexOrdenCpu);
    pthread_mutex_lock(&mutexOrdenCpu);
    enviarMensajeRespuestaCPU(socketPlanificador,nodoRtaCpuPlan);
	pthread_mutex_unlock(&mutexOrdenCpu);

    log_info(archivoLog, "CPU: %d, Proceso: %d - Instruccion: %s (%s) ",nodoRtaCpuPlan->idCPU,nodoRtaCpuPlan->PID,traducirInstruccion(nodoRtaCpuPlan->tipo),traducirExitoStatus(nodoRtaCpuPlan->exito));
    free(nodoRtaCpuPlan);
    free(nodoRta);
    free(nodoInstruccion);
    return continuar;
}


void textoAEscribir(char * instruccion, char * texto){
	int character = 0;
	while (!string_equals_ignore_case(string_substring(instruccion, character, 1),"\"")){
		character++;
	}
	strcpy(texto,string_substring(instruccion,++character,strlen(instruccion)));
	strtok(texto, "\"");
}

int valorPaginaEnEscritura(char * instruccion){
	int character = 0;
	while (!string_equals_ignore_case(string_substring(instruccion, character, 1),"\"")){
		character++;
	}
	int pagina = atoi(string_substring(instruccion, 9, character));
	return pagina;

}

int instruccionIniciarProceso (t_resp_swap_mem * nodoRta,t_nodo_mem * nodoInstruccion,int socketADM) {

	int err;
	int respuesta = -1;//Si el envío o la respuesta falla no debe continuar.

	if ((err = enviarMensajeDeNodoAMem(socketADM,nodoInstruccion, "")) > 0)
	{
		//Envíe el paquete.. espero respuesta
		if ((err = recibirNodoDeMEM(socketADM,nodoRta)) > 0)
		{
			respuesta = 1; //puede fallar por ejecución normal
		}
	}
	return respuesta;
}

int instruccionLeerPagina (t_resp_swap_mem * nodoRta,t_nodo_mem * nodoInstruccion,int socketADM) {


	int err;
	int respuesta = -1;
	if ((err = enviarMensajeDeNodoAMem(socketADM,nodoInstruccion,"")) > 0)
	{
		//Envíe el paquete.. espero respuesta
		if ((err = recibirNodoDeMEM(socketADM,nodoRta)) > 0)
		{
			respuesta = 1; //puede fallar por ejecución normal
		}
	}

	return respuesta;
}

int instruccionEscribirPagina (t_resp_swap_mem * nodoRta,t_nodo_mem * nodoInstruccion,int socketADM) {

	int respuesta = -1;

	int err;

	if ((err = enviarMensajeDeNodoAMem(socketADM,nodoInstruccion,nodoInstruccion->texto)) > 0)
	{
		//Envíe el paquete.. espero respuesta
		if ((err = recibirNodoDeMEM(socketADM,nodoRta)) > 0)
		{
			respuesta = 1; //puede fallar por ejecución normal
		}
	}
	return respuesta;
}

void instruccionFinalizarProceso(t_nodo_mem * nodoInstruccion, int socketADM) {
	int err;
	err = enviarMensajeDeNodoAMem(socketADM,nodoInstruccion,"");
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

int recibirPCBdePlanificador(t_pcb * nodoPCB,int socketPlanificador){
	unsigned char buffer[1024];
	int pidCpu = process_get_thread_id();
	int nbytes;
	if ((nbytes = recv(socketPlanificador , buffer , sizeof(buffer) , 0)) < 0){
		printf("CPU %d: Error recibiendo mensaje de Planificador", pidCpu);
		exit(EXIT_FAILURE);
	} else if (nbytes == 0) {
		printf("CPU %d: Socket Planificador desconectado", pidCpu);
		perror("Cerrando CPU");
		exit(EXIT_FAILURE);
		//CERRAR LA CPU
	}
	desempaquetarPCB(buffer,nodoPCB);
	return nbytes;
}



void desempaquetarPCB(unsigned char *buffer,t_pcb * nodoPCB){

	//t_pcb * pcb = malloc(sizeof(t_pcb));
	char programa[50];
	unpack(buffer,SECUENCIA_PCB,&nodoPCB->PID,&nodoPCB->CPU,&nodoPCB->estado,&nodoPCB->pc,&nodoPCB->quantum,&nodoPCB->totalInstrucciones,programa);

	nodoPCB->contextoEjecucion = programa;
}

int enviarMensajeInfoCPU(int socketPlanificador,t_info_cpu * nodoInfoCpu){
	int nbytes;
	unsigned char buffer[1024];
	empaquetarNodoInfoCPU(buffer,nodoInfoCpu);
	nbytes = send(socketPlanificador, buffer, sizeof(buffer) , 0);
	if (nbytes == 0) {
		printf("CPU: Socket Planificador %d desconectado.\n", socketPlanificador);
	} else if (nbytes < 0){
		printf("CPU: Socket Planificador %d envío de mensaje fallido.\n", socketPlanificador);
		perror("Error - Enviando mensaje");
	}
	return nbytes;
}
void empaquetarNodoInfoCPU(unsigned char *buffer,t_info_cpu * nodoInfoCpu){
	unsigned int tamanioBuffer;
	tamanioBuffer = pack(buffer,SECUENCIA_CPU_INFO,
			nodoInfoCpu->idCPU,nodoInfoCpu->retardo);
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


int enviarMensajeDeNodoAMem(int socketADM, t_nodo_mem * nodo, char * texto)
{
	int nbytes;
	int pid = process_get_thread_id();
	unsigned char buffer[1024];
	empaquetarNodoMemCPU(buffer,nodo, texto);
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

void empaquetarNodoMemCPU(unsigned char *buffer,t_nodo_mem * nodo, char * texto)
{
	unsigned int tamanioBuffer;
	tamanioBuffer = pack(buffer,SECUENCIA_CPU_MEM,nodo->pid,nodo->pagina,nodo->instruccion,nodo->socketCpu,texto);
}


int recibirNodoDeMEM(int socketADM, t_resp_swap_mem * nodo)
{
	unsigned char buffer[1024];
	int nbytes;
	int pidCpu = process_get_thread_id();
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


char * traducirInstruccion(int tipo){
	char * resultado = string_new();
	switch(tipo){
		case INICIAR:
			strcpy(resultado,"INICIAR");
			break;
		case LEER:
			strcpy(resultado,"LEER");
			break;
		case ESCRIBIR:
			strcpy(resultado,"ESCRIBIR");
			break;
		case ENTRADA_SALIDA:
			strcpy(resultado,"I/O");
			break;
		case QUANTUM_ACABADO:
			strcpy(resultado,"QUANTUM");
			break;
		case FINALIZAR:
			strcpy(resultado,"FINALIZAR");
			break;
		case ERRONEA:
			strcpy(resultado,"INST ERRONEA");
			break;
	}
	return resultado;
}


