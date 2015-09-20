#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include "memoria.h"

int socketCliente = 0;
char * puertoSwap;
char * ipSwap;
char * puertoEscucha;
sem_t hilo1Mutex, hilo2Mutex;
int value;


int main () {
	pthread_t hiloMonitorSockets;
	char *arg1 = "thr1";
	int r1;

	//Inicia el semaforo del hilo1
	sem_init(&hilo1Mutex, 0, 0);
	//Inicia el semaforo del hilo2
	sem_init(&hilo2Mutex, 0, 0);

	//Inicia los parametros
	levantarCfgInicial();

	r1 = pthread_create(&hiloMonitorSockets,NULL,monitorPrepararServidor(puertoEscucha,&hilo1Mutex,&hilo2Mutex,&socketCliente), (void *) arg1);

	char buf[1024];    // tamanio del mensaje
	int nbytes;
	while(1){
		sem_wait(&hilo1Mutex);
		puts("paso el semaforo");
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
				///////////////////////////
				///////////////////////////
			}
			sem_post(&hilo2Mutex);
	}
	//Cosas de la MEM

	return 0;
}

void levantarCfgInicial() {
	//Levanta sus puertos cfg e ip para conectarse
	char directorioActual[1024];
	getcwd(directorioActual, sizeof(directorioActual));
	strcat(directorioActual, "/src/config.cfg");

	puts(directorioActual);
	puertoSwap = configObtenerPuertoSwap(directorioActual);
	puts(puertoSwap);
	ipSwap = configObtenerIpSwap(directorioActual);
	puts(ipSwap);
	puertoEscucha = configObtenerPuertoEscucha(directorioActual);
	puts(puertoEscucha);
}

/*
//se conecta con el swap que tiene un servidor escuchando
	int socketSwap = socketCrearCliente(puertoSwap,ipSwap);
	printf("socket devuelto: %d",socketSwap);

	//prepara y envía un mensaje
	char mensaje[1024];
	strcpy(mensaje,"HOLAAMIGUEDEMIALMAAA");
	int estado = socketEnviarMensaje(socketSwap,mensaje); 	// Solo envio si el usuario no quiere salir.

	if (estado != -1){
		sleep(30);
		printf("respuesta: %s",socketRecibirMensaje(socketServidor);
	}*/
