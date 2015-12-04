/*
 * planificador.c
 *
 *  Created on: 13/9/2015
 *      Author: utnso
 */
/*
 * functions.c
 *
 *  Created on: 3/9/2015
 *      Author: utnso
 */

#include "planificador.h"

int main() {
	sem_init(&semProgramas,0,0);
	sem_init(&mutexListaListo,0,1);
	levantarCfg();
	listaDeBloqueado = list_create();
	listaDeEjecutado = list_create();
	listaDeListo = list_create();
	listaDeCPUs = list_create();
	listaTiempoDeEjecucion = list_create();
	listaTiempoDeEspera = list_create();
	listaTiempoDeRespuesta = list_create();

	pthread_t thread15;
	int r15;
	char *n15="cronometro";
	r15 = pthread_create( &thread15, NULL, cronometro, (void*) n15);

	pthread_t thread4;
	int r4;
	char *n1="instruccion";
	r4 = pthread_create( &thread4, NULL, eliminaCadaMinuto, (void*) n1);



	pthread_t thread1;
	char *m1 = "monitor";
	int r1;

	/*pthread_t thread2;
	char *m2 = "despachador";
	int r2;*/

	pthread_t thread3;
	char *m3 = "consola";
	int r3;

	r1 = pthread_create( &thread1, NULL, monitorearSockets, (void*) m1);
	//ver de hacer el hilo del monitor
	//r2 = pthread_create( &thread2, NULL, enviarPCBaCPU, (void*) m2);

	r3 = pthread_create( &thread3, NULL, consola, (void*) m3);

	//Enviar PCB a CPU
	pthread_t threadPcbACpu;
	int rcp;
	//usa el valor de nodo respuesta como tiempo de entrada salida
	rcp = pthread_create( &threadPcbACpu, NULL, enviarPCBaCPU, (void*) "pcbACpu");

	pthread_join( thread3, NULL);

	return 1;
}

void * cronometro() {
	while(1) {
		usleep(1000); //1000 microsegundos es 1 milisegundo.
		timer += 0.001;
	}
}


//HACERLO POR CADA CPU CONECTADA
//INFORMARSE CON UN MONITOR CADA VEZ QUE SE CONECTA UNA
void* enviarPCBaCPU()
{
	while (list_is_empty(listaDeCPUs)) {
		;;
	}
	bool buscarCPUDisponible(t_cpu * nodoCPU) {
		return (nodoCPU->disponible == 1);
	}
	t_cpu * nodoCPU = NULL;
	while (1){
		sem_wait(&semProgramas);
		nodoCPU = NULL;
		nodoCPU = list_find(listaDeCPUs,(void*)buscarCPUDisponible);
		//Buscar una CPU disponible en la colección de CPUS donde agrega el monitor
		if (nodoCPU != NULL){
			//CPU disponible, envío el PCB
			//sem_wait(sem_CPU_conectada);
			//sleep(2);
			while (list_is_empty(listaDeListo)) {
				;;
			}
			t_pcb * nodoPCB =  list_get(listaDeListo, 0);
			nodoPCB->CPU = nodoCPU->pid;
			int err = enviarMensajeDePCBaCPU(nodoCPU->socket, nodoPCB);
			if (err <= 0){
				//Error en el envío
				printf("No se pudo enviar el PCB %d",nodoPCB->PID);
				sem_post(&semProgramas);
			} else {
				nodoCPU->pcb = nodoPCB->PID;
				//Enviado correctamente
				nodoCPU->disponible = 0;
				//saca de lista ready
				list_remove(listaDeListo, 0);
				//se agrega a la lista de ejecucion
				list_add(listaDeEjecutado,nodoPCB);

				//printf("PCB %d enviado a CPU %d \n",nodoCPU->pcb,nodoCPU->pid);

			}//Cierra el err
		} else {
			//No hay cpu disponible, vuelve a iniciar el while
			sem_post(&semProgramas);
		}
	}
}

void recibirRespuestaCPU(int socketCpu, int * nbytes){
	t_resp_cpu_plan * nodoRespuesta;
	nodoRespuesta = malloc(sizeof(t_resp_cpu_plan));
	//nodoRespuesta->respuesta = malloc(1);
	*nbytes = recibirRtadeCPU(socketCpu, nodoRespuesta);//Dereferencia del puntero para cambmiarle el valor
	actualizarNodoCpu(socketCpu);
	interpretarLinea(nodoRespuesta);
}

void actualizarNodoCpu(int socketCpu)
{
	bool buscarCPUporSocket(t_cpu * nodoCPU) {
		return (nodoCPU->socket == socketCpu);
	}
	nodoCPU=NULL;
	nodoCPU = list_find(listaDeCPUs,(void*)buscarCPUporSocket);
	nodoCPU->instruccionesLeidas++;
}

/*
 * Verifica que el programa dado esté en la PC
 */
int programaValido(char * programa){
	FILE * fp;
	int resultado = 1;
	char* mprog = malloc(strlen(programa)+8+1);
	strcpy(mprog,"/mProgs/");
	strcat(mprog,programa);
	char *dir = getcwd(NULL, 0);
	char *directorioActual = malloc(strlen(dir)+strlen(mprog)+1);
	strcpy(directorioActual,dir);
	strcat(directorioActual,mprog);
	puts(directorioActual); //BORRAR
	fp = fopen(directorioActual, "r");
	if (fp == NULL){
		resultado = 0;
	} else {
		fclose(fp);
	}
	return resultado;
}

int totalInstruccionesArchivo(char * programa){
	FILE * fp;
	char* mprog = malloc(strlen(programa)+8+1);
	strcpy(mprog,"/mProgs/");
	strcat(mprog,programa);
	char *dir = getcwd(NULL, 0);
	char *directorioActual = malloc(strlen(dir)+strlen(mprog)+1);
	strcpy(directorioActual,dir);
	strcat(directorioActual,mprog);
	fp = fopen(directorioActual, "r");

	int resultado;

	if (fp == NULL){
		resultado = 0;
	} else {
		int ch, totalLineas = 0;

		do
		{
		    ch = fgetc(fp);
		    if(ch == '\n')
		    	totalLineas++;
		} while (ch != EOF);
		// last line doesn't end with a new line!
		// but there has to be a line at least before the last line
		if(ch != '\n' && totalLineas != 0)
			totalLineas++;

		fclose(fp);
		resultado = totalLineas;
	}
	return resultado;
}

void* agregarPCBALista(char * programa) {

	t_pcb * pcb = malloc(sizeof(t_pcb));
	pcb->PID = ++contadorPID;
	strcat(programa, "\0");
	pcb->contextoEjecucion = programa;

	//Agregar nodo a la listaDeRespuesta para despues calcular ese tiempo.

	t_respuesta_clock * nodoClockRespuesta = malloc(sizeof(t_respuesta_clock));

	nodoClockRespuesta->processID = pcb->PID;
	nodoClockRespuesta->tiempoInicial = timer;

	list_add(listaTiempoDeRespuesta, nodoClockRespuesta);

	pcb->pc=0;
	pcb->estado=LISTO;
	pcb->quantum=quantumcfg;
	pcb->totalInstrucciones= totalInstruccionesArchivo(programa);
	//imprimo pcb
	int antesDeAgregar = listaDeListo->elements_count;
	//MUTEX  PARA PRIORIZAR BLOQUEADOS Y RR SOBRE NUEVOS
	sem_wait(&mutexListaListo);
	list_add(listaDeListo, pcb);
	sem_post(&mutexListaListo);
	//MUTEX
	int despuesDeAgregar = listaDeListo->elements_count;
	if (despuesDeAgregar > antesDeAgregar) {
		//sem_post(&semProgramas);
		//incrementar buffer de pcbs a consumir
		sem_post(&semProgramas);

	} else {
		perror("Lista no agregada.");
	}
	pthread_exit(NULL);
}
 /*
void crearThreadParaComando(char * comando) {
	printf("Procesos listos: %d\n", listaDeListo->elements_count);
	t_hilos* hilo = malloc(sizeof(t_hilos));
	strncpy(hilo->m, comando, sizeof(hilo->m)-1);
	hilo->m[sizeof(hilo->m)-1] = "\0";

	hilo->r = pthread_create( &hilo->thread, NULL, detectarComando, (void*) (hilo->m));

	//list_add(listaDeListo, hilo_create(hilo->thread, hilo->m, hilo->r));

}
*/
void* consola() {
	char * resultado;
	int valorPID;
	puts("Ingrese 'correr xxxx.cod' para ejecutar.");
	puts("Ingrese 'exit' para salir");
	fgets(comando, 100, stdin);
	strtok(comando, "\n");
	int continuar = 1;
	while (continuar){
		//crearThreadParaComando(comando);
		if (!strcmp(comando,"exit")) {
			continuar = 0;
			break;
		} else if (esElComando(comando, "correr")) {
			resultado = devolverParteUsable(comando, 7);
			if (programaValido(resultado)){
				pthread_t agregarPCB;
				int rpcb;
				//usa el valor de nodo respuesta como tiempo de entrada salida
				rpcb = pthread_create( &agregarPCB, NULL, agregarPCBALista, resultado );
			} else {
				perror("Programa invalido");
			}
		} else if (esElComando(comando, "ps")) {
			imprimeEstado(listaDeListo,"listos");
			imprimeEstado(listaDeBloqueado,"bloqueados");
			imprimeEstado(listaDeEjecutado,"ejecutando");
	    } else if (esElComando(comando, "cpu")) {
			imprimePorcentajeCPU();
	    } else if (esElComando(comando, "finalizar")) {
	       	valorPID = devolverParteUsableInt(comando,10);
	      	finalizarProceso(valorPID);
	    } else {
			perror("Comando no valido.");
		}
		fgets(comando, 100, stdin);
		strtok(comando, "\n");
	}
	//pasos para cerrar el programa
	puts("Cerrando el proceso Planificador...");
}



void finalizarProceso (int pidProceso)
{

		t_pcb * nodoPCB = NULL;
		nodoPCB = buscarPCBListoPorPID(pidProceso);
		if (nodoPCB != NULL){
			nodoPCB->pc = nodoPCB->totalInstrucciones - 1;
		} else {
			nodoPCB = buscarPCBEjecutandoPorPID(pidProceso);
			if (nodoPCB != NULL){
				printf("El pid %d se encuentra ejecutando\n",pidProceso);
				printf("Intente nuevamente en unos instantes\n");
			} else {
				nodoPCB = buscarBloqueado(pidProceso);
				nodoPCB->pc = nodoPCB->totalInstrucciones - 1;
			}
		}
}


void* eliminaCadaMinuto()
{
	void limpiar(t_cpu * nodoCPU) {
		float porc = porcentajeCPU(nodoCPU);
		nodoCPU->porc = porc;
		nodoCPU->instruccionesLeidas = 0;
	}
	while(1)
	{
		list_iterate(listaDeCPUs,limpiar);
		sleep(60);
	}

}


void imprimePorcentajeCPU()
{
	int i;

	if(listaDeCPUs->elements_count > 0)
	{
		 int tamanio = listaDeCPUs->elements_count;
		 for(i=0; i< tamanio ;i++)
		 {
			t_cpu * nodoCPU = list_get(listaDeCPUs, i);
			float porc = porcentajeCPU(nodoCPU);
			printf("CPU %d porcentaje progresivo actual: %3.2f\n",nodoCPU->pid,porc);
			printf("CPU %d porcentaje ultimo minuto : %3.2f\n",nodoCPU->pid,nodoCPU->porc);
		 }
	}
	else
	{
		puts("No hay CPUs conectadas.\n");
	}
}


//funcion q calcula el porcentaje de cpu usada
float porcentajeCPU(t_cpu *nodoCPU){

  float respuesta;
  if (nodoCPU->retardo != 0){
	  int instruccionesLeidas=nodoCPU->instruccionesLeidas;
	  int maximasInstrucciones = 60 / nodoCPU->retardo;
	  respuesta = (100*instruccionesLeidas / maximasInstrucciones);
  } else {
	  respuesta = 0;
  }
  return respuesta;
}

int interpretarLineaSegunRetardo(char * linea, int retardo) {
  	strtok(linea,";");
  	int valor;
  	if (esElComando(linea, "entrada-salida")) {
		valor = devolverParteUsableInt(linea,14);
	} else {
		valor = retardo;
	}
  	return valor;
}

void imprimeEstado(t_list *lista, char*estado ){
	int i;
	if(lista->elements_count > 0)
	{
		 int tamanio = lista->elements_count;
		 for(i=0; i< tamanio ;i++)
		 {
			t_pcb * nodoPCB = list_get(lista, i);
			printf("mProc %d: %s -> %s\n",nodoPCB->PID,nodoPCB->contextoEjecucion,estado);
		 }
	} else {
		printf("Lista de mProcs %s vacia.\n",estado);
	}
}

void levantarCfg(){

	archivoLog = log_create("planificador.log", "Planificador", false, 2);

	char cfgFin[] ="/planificador/src/config.cfg";//Para consola
	//char cfgFin[] ="/src/config.cfg";//Para eclipse
	char *dir = getcwd(NULL, 0);

	char *directorioActual = malloc(strlen(dir)+strlen(cfgFin)+1);

	strcat(directorioActual,dir);
	strcat(directorioActual,cfgFin);
	puts(directorioActual);
	char * puerto;
	puerto = configObtenerPuertoEscucha(directorioActual);
	strcpy(algoritmo,configObtenerAlgoritmoPlanificador(directorioActual));
	if (string_equals_ignore_case(algoritmo,"RR")){
		quantumcfg = configObtenerQuantum(directorioActual);
	}
	servidorPlanificador = socketCrearServidor(puerto,"Planificador");
}


void interpretarLinea(t_resp_cpu_plan * nodoRespuesta) {

	int PID = nodoRespuesta->PID;

    int tipoResp = nodoRespuesta->tipo;
    int exito = nodoRespuesta->exito;
    int pagRW = nodoRespuesta->pagRW;

    bool buscarPCBporPID(t_pcb * nodoPCB) {
    		return (nodoPCB->PID == PID);
    	}

    t_pcb* nodoPCB=NULL;

    while (nodoPCB == NULL) { //no sale hasta que encuentra algo
    	nodoPCB = list_find(listaDeEjecutado,(void*)buscarPCBporPID);
    }

	//actualizar el pc del nodoPCB con el valor del pc del nodo respuesta
	nodoPCB->pc = nodoRespuesta->pc;
	//printf("EL PC ES DE: %d", nodoRespuesta->pc);

	int idCPU;
	idCPU = nodoCPU->pid;
	int buscarClockDelProceso(t_ejecucion_clock * nodo) {
		return (nodo->processID == PID);
	}

    switch (tipoResp) {
    		case INICIAR:
				if (exito){
					//Se calcula el tiempo de respuesta del proceso
					t_respuesta_clock * nodoClockRespuesta = malloc(sizeof(t_respuesta_clock));

					nodoClockRespuesta = list_find(listaTiempoDeRespuesta, (void*) buscarClockDelProceso);

					list_remove_by_condition(listaTiempoDeRespuesta, (void*) buscarClockDelProceso);

					double tiempoTranscurrido =  (double)(timer - nodoClockRespuesta->tiempoInicial);

					log_info(archivoLog,"El tiempo de respuesta del proceso %d fue %g segundos de la CPU %d", PID, tiempoTranscurrido, idCPU);

					//Agregar el nodo para poder calcular despues el tiempo total de ejecucion.
					t_ejecucion_clock * nodoClockEjecucion = malloc(sizeof(t_ejecucion_clock));
					nodoClockEjecucion->tiempoInicial = timer;
					nodoClockEjecucion->processID = PID;
					list_add(listaTiempoDeEjecucion, nodoClockEjecucion);

					//el nodo cpu tendra como parametro el retardo
					nodoCPU->retardo = pagRW;
					nodoCPU->disponible = 0;
					if (nodoCPU->pid == 0){
						nodoCPU->pid = idCPU;
					}
					//el nodo cpu tendra como parametro el puntero a la siguiente instruccion
					log_info(archivoLog,"CPU %d: Proceso mProc %d (%s) creado",idCPU,PID,nodoPCB->contextoEjecucion);
				} else {
					//saco de la lista de ejecutando
					log_debug(archivoLog,"CPU %d: Proceso mProc %d (%s) fallo",idCPU,PID,nodoPCB->contextoEjecucion);
					list_remove_by_condition(listaDeEjecutado,(void*)buscarPCBporPID);
					nodoCPU->disponible = 1;
				}
			break;
    		case LEER:
    			if (exito){
					//el nodo cpu tendra como parametro el puntero a la siguiente instruccion
    				log_info(archivoLog,"CPU %d: Proceso mProc %d - pagina %d leida: %s",idCPU,PID,pagRW,nodoRespuesta->respuesta);
				} else {
					log_debug(archivoLog,"CPU %d: Proceso mProc %d - pagina %d No leida.",idCPU,PID,pagRW);
					log_debug(archivoLog,"CPU %d: Proceso mProc %d - error en asignacion/reemplazo de marcos",idCPU,PID);
					list_remove_by_condition(listaDeEjecutado,(void*)buscarPCBporPID);
					nodoCPU->disponible = 1;
					nodoCPU->pcb = 0;
				}
    		break;
    		case ESCRIBIR:
    			if (exito){
					//el nodo cpu tendra como parametro el puntero a la siguiente instruccion
    				log_info(archivoLog,"CPU %d: Proceso mProc %d - escribio: %s ",idCPU,PID,nodoRespuesta->respuesta);
				} else {
					log_debug(archivoLog,"CPU %d:Proceso mProc %d NO escribio",idCPU,PID);
					log_debug(archivoLog,"CPU %d: Proceso mProc %d - error en asignacion/reemplazo de marcos",idCPU,PID);
					list_remove_by_condition(listaDeEjecutado,(void*)buscarPCBporPID);
					nodoCPU->disponible = 1;
					nodoCPU->pcb = 0;
				}
    		break;
    		case ENTRADA_SALIDA:
				if (exito){
					//el nodo cpu tendra como parametro el puntero a la siguiente instruccion
					log_info(archivoLog,"CPU %d: Proceso mProc %d - se bloquea por entrada-salida %d",idCPU,PID,pagRW);

					list_remove_by_condition(listaDeEjecutado,(void*)buscarPCBporPID);
					nodoCPU->disponible = 1;
					nodoCPU->pcb = 0;

					tiempoBloqueo = pagRW;

					//Inicializamos el clock para calcular la cantidad de tiempo que va a estar en entrada-salida.

					t_espera_clock * nodoClockEspera = malloc(sizeof(t_espera_clock));

					nodoClockEspera->tiempoInicial = timer;
					nodoClockEspera->processID = PID;

					list_add(listaTiempoDeEspera, nodoClockEspera);

					list_add(listaDeBloqueado,nodoPCB);

					pthread_t thread_I_O;
					int r3;
					//usa el valor de nodo respuesta como tiempo de entrada salida
					r3 = pthread_create( &thread_I_O, NULL, bloquearPCB, (void*)nodoPCB );

				} else {
					log_info(archivoLog,"CPU %d: Proceso mProc %d - fallo instruccion entrada-salida %d ",idCPU,PID,pagRW);
					log_debug(archivoLog,"CPU %d: Proceso mProc %d - error en entrada/salida",idCPU,PID);
					list_remove_by_condition(listaDeEjecutado,(void*)buscarPCBporPID);
					nodoCPU->disponible = 1;
					nodoCPU->pcb = 0;
				}
				break;
    		case QUANTUM_ACABADO:
				if (exito){

					list_remove_by_condition(listaDeEjecutado,(void*)buscarPCBporPID);
					nodoCPU->disponible = 1;
					nodoCPU->pcb = 0;
					//MUTEX  PARA PRIORIZAR BLOQUEADOS Y RR SOBRE NUEVOS
					sem_wait(&mutexListaListo);
					list_add(listaDeListo, nodoPCB);
					sem_post(&mutexListaListo);
					sem_post(&semProgramas);

					log_info(archivoLog,"CPU %d: Proceso mProc %d - se bloquea por quantum acabado ",idCPU,PID);
				} else {
					//validar luego que opcion hay para fallar
					log_info(archivoLog,"CPU %d: Proceso mProc %d - fallo la instruccion por quantum",idCPU,PID);
				}
				break;
    		case FINALIZAR:
    			;
				t_ejecucion_clock * nodoClockEjecucion = malloc(sizeof(t_ejecucion_clock));
				nodoClockEjecucion = list_find(listaTiempoDeEjecucion, (void*) buscarClockDelProceso);

				list_remove_by_condition(listaTiempoDeEjecucion, (void*) buscarClockDelProceso);

				double tiempoTranscurrido =  (double)(timer - nodoClockEjecucion->tiempoInicial);

				log_info(archivoLog,"El tiempo de ejecucion del proceso %d fue de %g segundos de la CPU %d", PID, tiempoTranscurrido, idCPU);

    			if (exito){
       				log_info(archivoLog,"CPU %d: Proceso mProc %d (%s) finalizado",idCPU,PID,nodoPCB->contextoEjecucion);
				} else {
					log_info(archivoLog,"CPU %d: Proceso mProc %d (%s) no finalizado",idCPU,PID,nodoPCB->contextoEjecucion);
				}
    			nodoCPU->disponible = 1;
    			list_remove_by_condition(listaDeEjecutado,(void*)buscarPCBporPID);
    			//free(nodoPCB);
			break;
    		default:
    		    perror("mensaje erroneo.");
    	}
}


int enviarMensajeDePCBaCPU(int socketCPU, t_pcb * nodoPCB) {
	int nbytes;
	unsigned char buffer[1024];
	empaquetarPCB(buffer,nodoPCB);
	nbytes = send(socketCPU, buffer, sizeof(buffer) , 0);
	if (nbytes == 0) {
		printf("Planificador: Socket CPU %d desconectado.\n", socketCPU);
	} else if (nbytes < 0){
		printf("Planificador: Socket CPU %d envío de mensaje fallido.\n", socketCPU);
		perror("Error - Enviando mensaje");
	}
	return nbytes;
}

void empaquetarPCB(unsigned char *buffer,t_pcb * nodoPCB)
{
	unsigned int tamanioBuffer;
	/*
	t_pcb * pcb1 = malloc(sizeof(t_pcb));
	pcb1->PID = 0;
	pcb1->contextoEjecucion = malloc(strlen("programa3.cod"));
	strcpy(pcb1->contextoEjecucion,"programa3.cod");
	pcb1->pc=3;
	pcb1->estado=LISTO;
	pcb1->quantum=5;

	printf("PCB a enviar: PID: %d,  Estado: %d, PC: %d, Quantum: %d, Archivo: %s\n",
			nodoPCB->PID,nodoPCB->estado,nodoPCB->pc,nodoPCB->quantum,nodoPCB->contextoEjecucion);
	 */
	tamanioBuffer = pack(buffer,SECUENCIA_PCB,
			nodoPCB->PID,nodoPCB->CPU,nodoPCB->estado,nodoPCB->pc,nodoPCB->quantum,nodoPCB->totalInstrucciones,nodoPCB->contextoEjecucion);

	//packi16(buffer+1, tamanioBuffer); // store packet size in packet for kicks
	//printf("Tamaño del PCB serializado es %u bytes\n", tamanioBuffer);
}

void desempaquetarNodoRtaCpuPlan(unsigned char *buffer,t_resp_cpu_plan * nodoRta){
	char respuesta[50];
	unpack(buffer,SECUENCIA_NODO_RTA_CPU_PLAN,
			&nodoRta->PID,&nodoRta->idCPU,&nodoRta->tipo,&nodoRta->exito,
			&nodoRta->pagRW,&nodoRta->pc,respuesta);
	nodoRta->respuesta = malloc(sizeof(respuesta));
	strcpy(nodoRta->respuesta, respuesta);
}

int recibirRtadeCPU(int socketCPU, t_resp_cpu_plan * nodoRta){
	unsigned char buffer[1024];
	int nbytes;
	if ((nbytes = recv(socketCPU , buffer , sizeof(buffer) , 0)) < 0){
		printf("Planificador: Error recibiendo mensaje de CPU.\n");
	} else if (nbytes == 0) {
		printf("Planificador: Socket CPU desconectado.\n");
		//CERRAR LA CPU
	} else {
		desempaquetarNodoRtaCpuPlan(buffer,nodoRta);
	}
	return nbytes;
}

void* buscarBloqueado(int PID){
	bool buscarPCBporPID(t_pcb * nodoPCB) {
			return (nodoPCB->PID == PID);
		}

	t_pcb* nodoPCB=NULL;
	nodoPCB = list_find(listaDeBloqueado,(void*)buscarPCBporPID);
	return nodoPCB;
}


void* buscarPCBEjecutandoPorPID(int PID){
	bool buscarPCBporPID(t_pcb * nodoPCB) {
			return (nodoPCB->PID == PID);
		}

	t_pcb* nodoPCB=NULL;
	nodoPCB = list_find(listaDeEjecutado,(void*)buscarPCBporPID);
	return nodoPCB;
}

void* buscarPCBListoPorPID(int PID){
	bool buscarPCBporPID(t_pcb * nodoPCB) {
			return (nodoPCB->PID == PID);
		}

	t_pcb* nodoPCB=NULL;
	nodoPCB = list_find(listaDeListo,(void*)buscarPCBporPID);
	return nodoPCB;
}

void* bloquearPCB(t_pcb * nodoPCB)
{
	sleep(tiempoBloqueo);//Viene de interpretar linea y usa pagRW para dormirlo
	//se agrega a la lista de listo
	//MUTEX  PARA PRIORIZAR BLOQUEADOS Y RR SOBRE NUEVOS
	sem_wait(&mutexListaListo);
	list_add(listaDeListo,nodoPCB );
	sem_post(&semProgramas);

	bool sacarPorPID(t_pcb * nodo) {
		return (nodo->PID == nodoPCB->PID);
	}
	list_remove_by_condition(listaDeBloqueado,(void*)sacarPorPID);

	//Loguea el timepo que estuvo en entrada-salida.

	int buscarClockPorProcessID(t_espera_clock * nodo) {
		return (nodo->processID == nodoPCB->PID);
	}

	t_espera_clock * nodoClockEspera = malloc(sizeof(t_espera_clock));

	nodoClockEspera = list_find(listaTiempoDeEspera, (void*) buscarClockPorProcessID);

	double tiempoTranscurrido =  (double)(timer - nodoClockEspera->tiempoInicial);

	log_info(archivoLog,"El tiempo de espera del proceso %d fue de %d segundos.", nodoPCB->PID, tiempoBloqueo);

	//Fin de logueo de que estuvo en entrada-salida.

	sem_post(&mutexListaListo);

}

void agregarCPUALista(int socketCpu) {
	t_cpu * nodoCpu = malloc(sizeof(t_cpu));
	nodoCpu->pid = listaDeCPUs->elements_count;
	nodoCpu->socket = socketCpu;
	nodoCpu->retardo = 0;
	nodoCpu->instruccionesLeidas = 0;
	nodoCpu->porc = 0;
	nodoCpu->disponible = 1;
	printf("CPU: %d, socket: %d, agregada a la lista de CPUs.\n", nodoCpu->pid,nodoCpu->socket );
	list_add(listaDeCPUs, nodoCpu);
	//Enviar PCB a CPU
	/*
	pthread_t threadPcbACpu;
	int rcp;
	//usa el valor de nodo respuesta como tiempo de entrada salida
	rcp = pthread_create( &threadPcbACpu, NULL, enviarPCBaCPU, (void*) "pcbACpu");
	*/
}


void* monitorearSockets(){

	fd_set coleccionSockets;    // coleccion de sockets
	fd_set coleccionTemp;  // coleccionTemp de sockets temporal

    int nuevoCliente;        // socket cliente recibido
    int i;
    int colMax;
    int nbytes;

    FD_ZERO(&coleccionSockets);    // Limpia los sets de colecciones
    FD_ZERO(&coleccionTemp);// Limpia los sets de colecciones
    // agrega el servidor a la colección
	FD_SET(servidorPlanificador, &coleccionSockets);

	colMax = servidorPlanificador;
    // main loop
    for(;;) {
        coleccionTemp = coleccionSockets; // lo copio para no perderlo
        if (select(colMax+1, &coleccionTemp, NULL, NULL, NULL) == -1) {
            perror("Error - Función Select.");
        }

        // busca el socket que tiene dato por leer
        for(i = 0; i <= colMax; i++) {
            if (FD_ISSET(i, &coleccionTemp)) {
                if (i == servidorPlanificador) {
                    // El servidor tiene que aceptar un nuevo cliente
                	//Aceptamos la conexion entrante, y creamos un nuevo socket mediante el cual nos podamos comunicar.
                    nuevoCliente = socketAceptarConexion(servidorPlanificador,"Planificador","CPU");
                    if (nuevoCliente > 0) {
                        FD_SET(nuevoCliente, &coleccionSockets); // agrega el cliente al set de sockets

                        if (nuevoCliente > colMax) {    // actualiza el maximo
                            colMax = nuevoCliente;
                        }
                        int pid;
                        //la agrega a la lista de CPUs
                        agregarCPUALista(nuevoCliente);
                    }
                } else {
                	//Tengo un mensaje de algún cliente.
                	//Respuesta de alguna instrucción de CPU
                	recibirRespuestaCPU(i, &nbytes);
                	if (nbytes <= 0) {
						// connection closed
                		close(i); // bye!
						FD_CLR(i, &coleccionSockets); // remove from master set
						informarDesconexionCPU(i);
					}

                } // fin recepcion de mensajes de cliente
            } // fin isset
        } // fin recorrido del set
    } // fin del for(;;)
}

void informarDesconexionCPU(int socket)
{
	bool buscarCPUporSocket(t_cpu * nodoCPU) {
		return (nodoCPU->socket == socket);
	}
	t_cpu* nodoCPU=NULL;
	nodoCPU = list_find(listaDeCPUs,(void*)buscarCPUporSocket);
	list_remove_by_condition(listaDeCPUs,(void*)buscarCPUporSocket);
	printf("La CPU: %d ha sido desconectada.\n", nodoCPU->pid);
}

