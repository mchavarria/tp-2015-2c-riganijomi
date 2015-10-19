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
	sem_init(&mutexCPU, 0, 0);
	sem_init(&semProgramas,0,0);
	levantarCfg();

	listaDeBloqueado = malloc(50000);
	listaDeBloqueado = list_create();
	listaDeEjecutado = malloc(50000);
	listaDeEjecutado = list_create();
	listaDeListo = malloc(50000);
	listaDeListo = list_create();

	pthread_t thread1;
	char *m1 = "consola";
	int r1;

	pthread_t thread2;
	char *m2 = "despachador";
	int r2;

	r1 = pthread_create( &thread1, NULL, &consola, (void*) m1);

	r2 = pthread_create( &thread2, NULL, &enviarPCBaCPU, (void*) m2);

	pthread_join( thread1, NULL);
	pthread_join( thread2, NULL);
	return 1;
}

void* enviarPCBaCPU() {

	sem_wait(&mutexCPU);
	while(1) {
		sem_wait(&semProgramas);

		if (listaDeListo->elements_count > 0) {
			//sem_wait(sem_CPU_conectada);
			t_pcb * nodoPCB =  list_get(listaDeListo, 0);
			int err = enviarMensajeDePCBaCPU(socketCPU, nodoPCB);
			if (err <= 0){
				//Error en el envío
				printf("No se pudo enviar el PCB %d",nodoPCB->PID);
			} else {
				//Enviado correctamente
				//saca de lista ready
				list_remove(listaDeListo, 0);
				//se agrega a la lista de ejecucion
				list_add(listaDeEjecutado,nodoPCB );

				t_resp_cpu_plan * nodoRespuesta;
				int nbytes;
				nodoRespuesta = malloc(sizeof(t_resp_cpu_plan));
				while ((nbytes = recibirRtadeCPU(socketCPU, nodoRespuesta) > 0)){
					interpretarLinea(nodoRespuesta);

				}//fin while recibir rta
			}//Cierra el err
		}//cierra if cantidad de elementos
	}//cierra while
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

void agregarALista(char * programa) {

	t_pcb * pcb = malloc(sizeof(t_pcb));
	pcb->PID = ++contadorPID;
	strcat(programa, "\0");
	pcb->contextoEjecucion = programa;


	pcb->pc=0;
	pcb->estado=LISTO;
	pcb->quantum=quantumcfg;
	//imprimo pcb
	int antesDeAgregar = listaDeListo->elements_count;
	list_add(listaDeListo, pcb);
	int despuesDeAgregar = listaDeListo->elements_count;
	if (despuesDeAgregar > antesDeAgregar) {
		//sem_post(&semProgramas);
		//incrementar buffer de pcbs a consumir
		sem_post(&semProgramas);
	} else {
		perror("Lista no agregada.");
	}


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
				agregarALista(resultado);
			} else {
				perror("Programa invalido");
			}
		} else {
			perror("Comando no valido.");
		}
		fgets(comando, 100, stdin);
		strtok(comando, "\n");
	}
	//pasos para cerrar el programa
	puts("Cerrando el proceso Planificador...");
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
    quantumcfg = configObtenerQuantum(directorioActual);
	servidorPlanificador = socketCrearServidor(puerto,"Planificador");
	puts("Cfg iniciada... esperando CPU...");
	socketCPU = socketAceptarConexion(servidorPlanificador,"Planificador","CPU");
	if (socketCPU > 0)	{
		sem_post(&mutexCPU);
	}
}


void interpretarLinea(t_resp_cpu_plan * nodoRespuesta) {

	int PID = nodoRespuesta ->PID;
	int idCPU = nodoRespuesta ->idCPU;
    int tipoResp = nodoRespuesta->tipo;
    int exito = nodoRespuesta->exito;
    int pagRW = nodoRespuesta->pagRW;
    int pc = nodoRespuesta->pc;

    bool buscarPorPID(t_pcb * nodoPCB) {
    		return (nodoPCB->PID == PID);
    	}

    t_pcb* nodoPCB=NULL;
	nodoPCB = list_find(listaDeEjecutado,(void*)buscarPorPID);


    switch (tipoResp) {
    		case INICIAR:
				if (exito){
					log_info(archivoLog,"CPU %d: Proceso mProc %d (%s) creado",idCPU,PID,nodoPCB->contextoEjecucion);
				} else {
					//saco de la lista de ejecutando
					log_debug(archivoLog,"CPU %d: Proceso mProc %d (%s) fallo",idCPU,PID,nodoPCB->contextoEjecucion);
					list_remove_by_condition(listaDeEjecutado,(void*)buscarPorPID);
					free(nodoPCB);
				}
			break;
    		case LEER:
    			if (exito){
    				log_info(archivoLog,"CPU %d: Proceso mProc %d - pagina %d leida: %s",idCPU,PID,pagRW,nodoRespuesta->respuesta);
				} else {
					log_debug(archivoLog,"CPU %d: Proceso mProc %d - pagina %d No leida.",idCPU,PID,pagRW);
				}
    		break;
    		case ESCRIBIR:
    			//No necesita avisarle al cpu
				//TODO Solo loguear y actualizar las estructuras necesarias
    			if (exito){
    				log_info(archivoLog,"CPU %d: Proceso mProc %d - escribio ",idCPU,PID);
				} else {
					log_debug(archivoLog,"CPU %d:Proceso mProc %d NO escribio",idCPU,PID);
				}
    		break;
    		case ENTRADA_SALIDA:
    		   	//No necesita avisarle al cpu
				//TODO Solo loguear y actualizar las estructuras necesarias
				if (exito){
					log_info(archivoLog,"CPU %d: Proceso mProc %d - se bloquea por entrada-salida ",idCPU,PID);
					list_remove_by_condition(listaDeEjecutado,(void*)buscarPorPID);
					list_add(listaDeBloqueado,nodoPCB);
					//usar el valor de nodo respuesta como tiempo de entrada salida
				} else {
					log_info(archivoLog,"CPU %d: Proceso mProc %d - fallo instruccion entrada-salida ",idCPU,PID);
				}
				break;
    		case QUANTUM_ACABADO:
				if (exito){
					t_pcb* nodoPCB=NULL;
					nodoPCB = list_find(listaDeEjecutado,(void*)buscarPorPID);
					//actualizar el pc del nodoPCB con el valor del pc del nodo respuesta
					nodoPCB->pc = nodoRespuesta->pc;
					list_remove_by_condition(listaDeEjecutado,(void*)buscarPorPID);
					list_add(listaDeListo,nodoPCB);
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
    			list_remove_by_condition(listaDeEjecutado,(void*)buscarPorPID);
    			free(nodoPCB);
			break;
    		default:
    		    perror("mensaje erroneo.");
    	}
}


static t_hilos *hilo_create(pthread_t thread, char * m, int  r) {
	t_hilos *nuevo = malloc(sizeof(t_hilos));

	nuevo->thread = thread;
	nuevo->r = r;
	strncpy(nuevo->m, m, sizeof(nuevo->m)-1);
	nuevo->m[sizeof(nuevo->m)-1] = "\0";
    return nuevo;
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
			nodoPCB->PID,nodoPCB->estado,nodoPCB->pc,nodoPCB->quantum,nodoPCB->contextoEjecucion);

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
		printf("Planificador: Error recibiendo mensaje de CPU");
	} else if (nbytes == 0) {
		printf("Planificador: Socket CPU desconectado");
		//CERRAR LA CPU
	}
	desempaquetarNodoRtaCpuPlan(buffer,nodoRta);
	return nbytes;
}

