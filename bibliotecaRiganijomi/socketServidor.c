#include "socketServidor.h"

//Puede ser del cfg
#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar
#define BACKLOG 5 // Define cuantas conexiones vamos a mantener pendientes al mismo tiempo


int socketCrearServidor(char * PUERTO){

	//Obtiene los datos de la direccion de red y lo guarda en serverInfo.
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// No importa si uso IPv4 o IPv6
	hints.ai_flags = AI_PASSIVE;		// Asigna el address del localhost: 127.0.0.1
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	getaddrinfo(NULL, PUERTO, &hints, &serverInfo); // Notar que le pasamos NULL como IP, ya que le indicamos que use localhost en AI_PASSIVE


	//Mediante socket(), obtengo el File Descriptor que me proporciona el sistema (un integer identificador).

	/* Necesitamos un socket que escuche las conecciones entrantes */
	int listeningSocket;
	listeningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
	if (listeningSocket == -1)
	{
		printf("Could not create socket");
	} else {
		puts("Socket created");

		if(bind(listeningSocket,serverInfo->ai_addr, serverInfo->ai_addrlen) < 0)
		{
			//print the error message
			perror("bind failed. Error");
		} else {
			puts("bind done");
			freeaddrinfo(serverInfo); // Ya no lo vamos a necesitar
		}


			listen(listeningSocket, BACKLOG);		// IMPORTANTE: listen() es una syscall BLOQUEANTE.
	}
	return listeningSocket;
}

int socketAceptarConexion(int socket){
	struct sockaddr_in addr;			// Esta estructura contendra los datos de la conexion del cliente. IP, puerto, etc.
	int socketCliente;
	socklen_t addrlen = sizeof(addr);

	socketCliente = accept(socket, (struct sockaddr *) &addr, &addrlen);
	if (socketCliente < 0)
	{
		perror("accept failed");
	} else {
		puts("Connection accepted");
	}

	return socketCliente;
}
