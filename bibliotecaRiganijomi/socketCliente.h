#ifndef SOCKETCLIENTE_H_
#define SOCKETCLIENTE_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/stat.h>

int socketCrearCliente(char * PUERTO, char * IP, char * procesoNombre, char * procesoServidor);
int socketEnviarMensaje(int serverSocket, char * mensaje,int longitud);
int socketRecibirMensaje(int serverSocket, char * mensaje, int longitud);
void socketCerrarSocket(int serverSocket);


#endif /* SOCKETCLIENTE_H_ */
