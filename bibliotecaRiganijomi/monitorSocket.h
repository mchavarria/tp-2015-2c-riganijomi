#ifndef MONITORSOCKET_H_
#define MONITORSOCKET_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <semaphore.h>

#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar
#define BACKLOG 5
void *monitorPrepararServidor(void * argument);
void monitorEliminarSocket(int serverSocket);
int crearServidor(char * PUERTO);

#endif /* MONITORSOCKET_H_ */


