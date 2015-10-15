#ifndef SOCKETSERVIDOR_H_
#define SOCKETSERVIDOR_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include <sys/stat.h>
#include <commons/collections/dictionary.h>
#include <commons/config.h>
#include <commons/string.h>

int socketCrearServidor(char * PUERTO, char * procesoNombre);
int socketAceptarConexion(int socket, char * procesoNombre, char * procesoCliente);

#endif /* SOCKETSERVIDOR_H_ */
