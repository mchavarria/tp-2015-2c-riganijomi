
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "memoria.h"
#include <semaphore.h>
#include <signal.h>

//valores cfg
char * puertoSwap;
char * ipSwap;
char * puertoEscucha;
char * maxMarcosProceso;
char * cantMarcos;
char * tamanioMarco;
char * entradasTLB;
char * TLBHabilitada;
char * retardoMemoria;
//sockets
int socketCliente;
int socketSwap;
//comandos de consola
char comandoConsola[50];
//para mensajes recibidos
char buf[1024];
int nbytes;

sem_t sem_mem;
sem_t sem_sockets;

int main () {
	sem_init(&sem_mem, 0, 0);
	sem_init(&sem_sockets, 0, 1);
	//Inicia los parametros
	levantarCfgInicial();

	pthread_t hiloMonitorSockets;
	char *arg1 = "memoria";
	int r1;

	//Tratamiento de la señan enviada por el SO
	signal(SIGINT, rutina);
	signal(SIGUSR1, rutina);

	r1 = pthread_create(&hiloMonitorSockets,NULL,monitorPrepararServidor(&sem_mem,&sem_sockets), (void *) arg1);

	for(;;){
		sem_wait(&sem_mem);
		// tengo un mensaje de algun cliente
		if ((nbytes = recv(socketCliente, buf, sizeof buf, 0)) <= 0) {
			// Error o conexion cerrada por el cliente
			if (nbytes == 0) {
				///////////////////////////
				//LOGUEAR conexion cerrada
				printf("servidor MEM: socket %d desconectado\n", socketCliente);
				///////////////////////////
				///////////////////////////
			} else {
				///////////////////////////
				perror("recepcion error");
				///////////////////////////
			}
			monitorEliminarSocket(socketCliente);
		} else {
			// Hay dato para leer. Enviarlo a alguien
			///////////////////////////
			puts(buf);
			if (socketEnviarMensaje(socketCliente, "RECIBI ALGO") == -1) {
				perror("send");
			}
			sem_post(&sem_sockets);
			///////////////////////////
			///////////////////////////
		}
	}

	return 0;
}

void levantarCfgInicial() {
	//Levanta sus puertos cfg e ip para conectarse
	char directorioActual[1024];
	getcwd(directorioActual, sizeof(directorioActual));
	strcat(directorioActual, "/src/config.cfg");

	puertoSwap = configObtenerPuertoSwap(directorioActual);
	ipSwap = configObtenerIpSwap(directorioActual);
	puertoEscucha = configObtenerPuertoEscucha(directorioActual);
	maxMarcosProceso = configObtenerMaxMarcosProceso(directorioActual);
	cantMarcos = configObtenerCantMarcos(directorioActual);
	tamanioMarco = configObtenerTamanioMarco(directorioActual);
	entradasTLB = configObtenerEntradasTLB(directorioActual);
	TLBHabilitada = configObtenerTLBHabilitada(directorioActual);
	retardoMemoria = configObtenerRetardoMemoria(directorioActual);

	configurarSocketSwap();
}

void configurarSocketSwap(){
	//se conecta con el swap que tiene un servidor escuchando
	socketSwap = socketCrearCliente(puertoSwap,ipSwap);
	printf("socket devuelto: %d",socketSwap);

	//prepara y envía un mensaje
	/*char mensaje[1024];
	strcpy(mensaje,"HOLAAMIGUEDEMIALMAAA");
	int estado = socketEnviarMensaje(socketSwap,mensaje); 	// Solo envio si el usuario no quiere salir.

	if (estado != -1){
		sleep(30);
		printf("respuesta: %s",socketRecibirMensaje(socketServidor);
	}*/
}

void rutina (int n) {
	switch (n) {
		case SIGINT:
			printf("En tu cara, no salgo nada…\n");
		break;
		case SIGUSR1:
			printf("LLEGO SIGUSR1\n");
		break;
		case SIGUSR2:
			printf("LLEGO SIGUSR2\n");
		break;
	}
}


