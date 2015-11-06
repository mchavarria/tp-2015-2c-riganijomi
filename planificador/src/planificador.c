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

	pthread_join( thread3, NULL);

	return 1;
}

//HACERLO POR CADA CPU CONECTADA
//INFORMARSE CON UN MONITOR CADA VEZ QUE SE CONECTA UNA
void* enviarPCBaCPU()
{
	sem_wait(&semProgramas);
	bool buscarCPUDisponible(t_cpu * nodoCPU) {
		return (nodoCPU->disponible == 1);
	}
	t_cpu * nodoCPU = NULL;
	nodoCPU = list_find(listaDeCPUs,(void*)buscarCPUDisponible);
	//Buscar una CPU disponible en la colección de CPUS donde agrega el monitor
	if (nodoCPU != NULL){
		//CPU disponible, envío el PCB
		//sem_wait(sem_CPU_conectada);
		t_pcb * nodoPCB =  list_get(listaDeListo, 0);
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
			list_add(listaDeEjecutado,nodoPCB );
		}//Cierra el err
	} else {
		//No hay cpu disponible, vuelve a iniciar el while
		sem_post(&semProgramas);
	}
}

void recibirRespuestaCPU(int socketCpu, int * nbytes){
	t_resp_cpu_plan * nodoRespuesta;
	nodoRespuesta = malloc(sizeof(t_resp_cpu_plan));
	*nbytes = recibirRtadeCPU(socketCpu, nodoRespuesta);//Dereferencia del puntero para cambmiarle el valor
	interpretarLinea(nodoRespuesta);
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
		//Enviar PCB a CPU
		pthread_t threadPcbACpu;
		int rcp;
		//usa el valor de nodo respuesta como tiempo de entrada salida
		rcp = pthread_create( &threadPcbACpu, NULL, enviarPCBaCPU, (void*) "pcbACpu");
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
	    } else {
			perror("Comando no valido.");
		}
		fgets(comando, 100, stdin);
		strtok(comando, "\n");
	}
	//pasos para cerrar el programa
	puts("Cerrando el proceso Planificador...");
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
			printf("CPU %d porcentaje: %3.2f\n",nodoCPU->pid,porc);
		 }
	}
	else
	{
		puts("No hay CPUs conectadas.\n");
	}
}


//funcion q calcula el porcentaje de cpu usada
float porcentajeCPU(t_cpu *nodoCPU){
  int total=0;
  int usado=0;
  float respuesta;
  if(nodoCPU->disponible==1)
  {
	  respuesta = 0;
  } else {
	  t_pcb* nodoPCB = buscarPCBEjecutandoPorPID(nodoCPU->pcb);

	  FILE * fp;
	  int pc = 0;
	  char * linea = NULL;
	  size_t len = 0;
	  ssize_t read;

	  //Configuro el archivo para abrir el mProg
	  char *mprog = malloc(strlen(nodoPCB->contextoEjecucion)+8+1);
	  strcpy(mprog,"/mProgs/");
	  strcat(mprog,nodoPCB->contextoEjecucion);

	  char *dir = getcwd(NULL, 0);
	  char *contextoEjecucion = malloc(strlen(dir)+strlen(mprog)+1);
	  strcpy(contextoEjecucion,dir);
	  strcat(contextoEjecucion,mprog);
	  //Hasta aca el arlchivo

	  fp = fopen(contextoEjecucion, "r");

	  while (((read = getline(&linea, &len, fp)) != -1))
	  {
		  total = total + interpretarLineaSegunRetardo(linea,nodoCPU->retardo);
		  if (pc < nodoPCB->pc){
		     pc++;
		  }
	  }

	  fclose(fp);
	  if (linea){
	      free(linea);
	  }
      respuesta = usado * 100 / total;
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

	//char cfgFin[] ="/planificador/src/config.cfg";//Para consola
	char cfgFin[] ="/src/config.cfg";//Para eclipse
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

	int PID = nodoRespuesta ->PID;
	int idCPU = nodoRespuesta ->idCPU;
    int tipoResp = nodoRespuesta->tipo;
    int exito = nodoRespuesta->exito;
    int pc = nodoRespuesta->pc;
    int pagRW = nodoRespuesta->pagRW;

    bool buscarPCBporPID(t_pcb * nodoPCB) {
    		return (nodoPCB->PID == PID);
    	}

    t_pcb* nodoPCB=NULL;
	nodoPCB = list_find(listaDeEjecutado,(void*)buscarPCBporPID);

	bool buscarCPUporPid(t_cpu * nodoCPU) {
	    		return (nodoCPU->pcb == PID);
	    	}
	t_cpu * nodoCPU = NULL;
	nodoCPU = list_find(listaDeCPUs,(void*)buscarCPUporPid);//Para actualizar si está disponible o no

    switch (tipoResp) {
    		case INICIAR:
				if (exito){
					//el nodo cpu tendra como parametro el retardo
					nodoCPU->retardo = pagRW;
					nodoCPU->pcb = PID;
					if (nodoCPU->pid == 0){
						nodoCPU->pid = idCPU;
					}
					//el nodo cpu tendra como parametro el puntero a la siguiente instruccion
					log_info(archivoLog,"CPU %d: Proceso mProc %d (%s) creado",idCPU,PID,nodoPCB->contextoEjecucion);
				} else {
					//saco de la lista de ejecutando
					log_debug(archivoLog,"CPU %d: Proceso mProc %d (%s) fallo",idCPU,PID,nodoPCB->contextoEjecucion);
					list_remove_by_condition(listaDeEjecutado,(void*)buscarPCBporPID);
					free(nodoPCB);
					nodoCPU->disponible = 1;
				}
			break;
    		case LEER:
    			if (exito){
					//el nodo cpu tendra como parametro el puntero a la siguiente instruccion
    				log_info(archivoLog,"CPU %d: Proceso mProc %d - pagina %d leida: %s",idCPU,PID,pagRW,nodoRespuesta->respuesta);
				} else {
					log_debug(archivoLog,"CPU %d: Proceso mProc %d - pagina %d No leida.",idCPU,PID,pagRW);
				}
    		break;
    		case ESCRIBIR:
    			if (exito){
					//el nodo cpu tendra como parametro el puntero a la siguiente instruccion
    				log_info(archivoLog,"CPU %d: Proceso mProc %d - escribio ",idCPU,PID);
				} else {
					log_debug(archivoLog,"CPU %d:Proceso mProc %d NO escribio",idCPU,PID);
				}
    		break;
    		case ENTRADA_SALIDA:
				if (exito){
					//el nodo cpu tendra como parametro el puntero a la siguiente instruccion
					log_info(archivoLog,"CPU %d: Proceso mProc %d - se bloquea por entrada-salida %d",idCPU,PID,pagRW);

					list_remove_by_condition(listaDeEjecutado,(void*)buscarPCBporPID);
					nodoCPU->disponible = 1;
					nodoCPU->pcb = 0;
					pthread_t thread_I_O;
					int r3;
					//usa el valor de nodo respuesta como tiempo de entrada salida
					r3 = pthread_create( &thread_I_O, NULL, bloquearPCB, nodoRespuesta );
				} else {
					log_info(archivoLog,"CPU %d: Proceso mProc %d - fallo instruccion entrada-salida %d ",idCPU,PID,pagRW);
				}
				break;
    		case QUANTUM_ACABADO:
				if (exito){
					//el nodo cpu tendra como parametro el puntero a la siguiente instruccion
					t_pcb* nodoPCB=NULL;
					nodoPCB = list_find(listaDeEjecutado,(void*)buscarPCBporPID);
					//actualizar el pc del nodoPCB con el valor del pc del nodo respuesta
					nodoPCB->pc = pc;
					list_remove_by_condition(listaDeEjecutado,(void*)buscarPCBporPID);

					//MUTEX  PARA PRIORIZAR BLOQUEADOS Y RR SOBRE NUEVOS
					sem_wait(&mutexListaListo);
					list_add(listaDeListo, nodoPCB);
					sem_post(&mutexListaListo);

					nodoCPU->disponible = 1;
					nodoCPU->pcb = 0;
					sem_post(&semProgramas);
					//Enviar PCB a CPU
					pthread_t threadPcbACpu;
					int rcp;
					//usa el valor de nodo respuesta como tiempo de entrada salida
					rcp = pthread_create( &threadPcbACpu, NULL, enviarPCBaCPU, (void*) "pcbACpu");

					log_info(archivoLog,"CPU %d: Proceso mProc %d - se bloquea por quantum acabado ",idCPU,PID);
				} else {
					//validar luego que opcion hay para fallar
					log_info(archivoLog,"CPU %d: Proceso mProc %d - fallo la instruccion por quantum",idCPU,PID);
				}
				break;
    		case FINALIZAR:
    			if (exito){
       				log_info(archivoLog,"CPU %d: Proceso mProc %d (%s) finalizado",idCPU,PID,nodoPCB->contextoEjecucion);
				} else {
					log_info(archivoLog,"CPU %d: Proceso mProc %d (%s) no finalizado",idCPU,PID,nodoPCB->contextoEjecucion);
				}
    			nodoCPU->disponible = 1;
    			list_remove_by_condition(listaDeEjecutado,(void*)buscarPCBporPID);
    			free(nodoPCB);
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
			nodoPCB->PID,nodoPCB->estado,nodoPCB->pc,nodoPCB->quantum,nodoPCB->totalInstrucciones,nodoPCB->contextoEjecucion);

	//packi16(buffer+1, tamanioBuffer); // store packet size in packet for kicks
	//printf("Tamaño del PCB serializado es %u bytes\n", tamanioBuffer);
}

void desempaquetarNodoRtaCpuPlan(unsigned char *buffer,t_resp_cpu_plan * nodoRta){
	char respuesta[50];
	unpack(buffer,SECUENCIA_NODO_RTA_CPU_PLAN,
			&nodoRta->PID,&nodoRta->idCPU,&nodoRta->tipo,&nodoRta->exito,
			&nodoRta->pagRW,&nodoRta->pc,respuesta);

	nodoRta->respuesta = respuesta;
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

void* buscarPCBEjecutandoPorPID(int PID){
	bool buscarPCBporPID(t_pcb * nodoPCB) {
			return (nodoPCB->PID == PID);
		}

	t_pcb* nodoPCB=NULL;
	nodoPCB = list_find(listaDeEjecutado,(void*)buscarPCBporPID);
	return nodoPCB;
}

void* bloquearPCB(void *contexto) {
	t_resp_cpu_plan * nodoRespuesta = contexto;

	t_pcb* nodoPCB = buscarPCBEjecutandoPorPID(nodoRespuesta->PID);

	//UTILIZA EL MUTEX PARA AGREGAR A LISTO NUEVAMENTE
	list_add(listaDeBloqueado,nodoPCB);

	sleep(nodoRespuesta->pagRW);//Viene de interpretar linea y usa pagRW para dormirlo
	//se agrega a la lista de listo
	//MUTEX  PARA PRIORIZAR BLOQUEADOS Y RR SOBRE NUEVOS
	sem_wait(&mutexListaListo);
	list_add(listaDeListo,nodoPCB );
	sem_post(&mutexListaListo);

	//Enviar PCB a CPU
	pthread_t threadPcbACpu;
	int rcp;
	//usa el valor de nodo respuesta como tiempo de entrada salida
	rcp = pthread_create( &threadPcbACpu, NULL, enviarPCBaCPU, (void*) "pcbACpu");
}

void agregarCPUALista(int socketCpu) {
	t_cpu * nodoCpu = malloc(sizeof(t_cpu));
	nodoCpu->pid = listaDeCPUs->elements_count;
	nodoCpu->socket = socketCpu;
	nodoCpu->retardo = 0;
	nodoCpu->disponible = 1;
	printf("CPU: %d, socket: %d, agregada a la lista de CPUs.\n", nodoCpu->pid,nodoCpu->socket );
	list_add(listaDeCPUs, nodoCpu);
	//Enviar PCB a CPU
	pthread_t threadPcbACpu;
	int rcp;
	//usa el valor de nodo respuesta como tiempo de entrada salida
	rcp = pthread_create( &threadPcbACpu, NULL, enviarPCBaCPU, (void*) "pcbACpu");
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

void informarDesconexionCPU(int socketCPU)
{
	bool buscarCPUporSocket(t_cpu * nodoCPU) {
		return (nodoCPU->socket == socketCPU);
	}
	t_cpu* nodoCPU=NULL;
	nodoCPU = list_find(listaDeCPUs,(void*)buscarCPUporSocket);
	list_remove_by_condition(listaDeCPUs,(void*)buscarCPUporSocket);
	printf("La CPU: %d ha sido desconectada.\n", nodoCPU->pid);
}

