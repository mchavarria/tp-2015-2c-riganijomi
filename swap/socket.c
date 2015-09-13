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


#define PACKAGESIZE 1024	// size maximo del paquete a enviar
#define BACKLOG 5 // conexiones pendientes al mismo tiempo
#define IP "192.168.1.126"
#define PUERTO "6667"


int crear_socket_servidor(){


	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// No importa si uso IPv4 o IPv6
	hints.ai_flags = AI_PASSIVE;		// Asigna el address del localhost: 127.0.0.1
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	getaddrinfo(NULL, PUERTO, &hints, &serverInfo);


	/* socket que escuche las conecciones entrantes */
	int listeningSocket;
	listeningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
	if (listeningSocket == -1)
	{
		printf("Could not create socket");
	} else {
		puts("Socket created");
	}


	if(bind(listeningSocket,serverInfo->ai_addr, serverInfo->ai_addrlen) < 0)
	{
		perror("bind failed. Error");
		return 1;
	} else {
	    puts("bind done");
	    freeaddrinfo(serverInfo);
	}


	listen(listeningSocket, BACKLOG);

	/*	Acepta la conexion entrante, y crea un nuevo socket mediante el cual nos podamos comunicar */
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);

	int socketCliente = accept(listeningSocket, (struct sockaddr *) &addr, &addrlen);
	if (socketCliente < 0)
	{
		perror("accept failed");
		return 1;
	} else {
	    puts("Connection accepted");
	}


	 /* Listo para recibir paquetes del cliente	 */
	char mensaje[PACKAGESIZE];
	char package[PACKAGESIZE];

	printf("Cliente conectado. Esperando mensajes:\n");

	write(socketCliente, "Soy el swap, te doy la bienvenida!!", PACKAGESIZE);

	recv(socketCliente, (void*) package, PACKAGESIZE, 0);
	printf("%s", package);
	fgets(mensaje, PACKAGESIZE, stdin);
	write(socketCliente, mensaje, PACKAGESIZE);


	/*Cierre de conexiones*/
	close(socketCliente);
	close(listeningSocket);

	return 0;
}
