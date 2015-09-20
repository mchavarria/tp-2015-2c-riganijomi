#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/stat.h>

#define PACKAGESIZE 1024

/*
 * Crea un socket servidor para la solicitud
 * antes de solicitar deben definir la variable donde alojan al servidor
 * int serverSocket;
 */
int socketCrearCliente(char * PUERTO, char * IP){

	int serverSocket;
	//Obtiene los datos de la direccion de red y lo guarda en serverInfo.
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	getaddrinfo(IP, PUERTO, &hints, &serverInfo);	// Carga en serverInfo los datos de la conexion


	//Obtiene un socket utilizando la estructura serverInfo.

	serverSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
	if (serverSocket == -1)
	{
		puts("Could not create socket");
	} else {
		puts("Socket created");
		//Ahora me conecto!
		if (connect(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen))
		{
			perror("connect failed. Error");
			serverSocket = 0;
		} else {
			puts("Connected\n");
		}
	}

	freeaddrinfo(serverInfo);	// No lo necesitamos mas
	return serverSocket;
}

/*
 *	Envio de mensaje al socket servidor
 *
 *	Es necesario que envíe el mensaje y el temaño del mensaje (tamanioPaquete)
 *
 */
int socketEnviarMensaje(int serverSocket, char  * mensaje) {
	puts("antes");
	int estado = send(serverSocket, mensaje, PACKAGESIZE , 0);
	if (estado == -1){
		//-1 Indica error en el envío
		puts("error enviando mensaje al servidor");
	}
	puts("despues");
	return estado;
}

void socketRecibirMensaje(int serverSocket, char * mensaje) {

	if (recv(serverSocket , mensaje , PACKAGESIZE , 0) == -1){
		puts('error recibiendo mensaje');
	}
}

/*
 * Cierra la conexión con el servidor
 */
void socketCerrarSocket(int serverSocket) {
	if (close(serverSocket) == -1){
		//-1 Indica error en el envío
		puts('error cerrando el socket servidor');
	}
}
