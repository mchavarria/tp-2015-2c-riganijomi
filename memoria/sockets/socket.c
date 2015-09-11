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

//Puede ser del cfg
#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar
#define BACKLOG 5 // Define cuantas conexiones vamos a mantener pendientes al mismo tiempo


char crearSocketServidor(char * PUERTO){

	/*
	 *  Obtiene los datos de la direccion de red y lo guarda en serverInfo.
	 *
	 */
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// No importa si uso IPv4 o IPv6
	hints.ai_flags = AI_PASSIVE;		// Asigna el address del localhost: 127.0.0.1
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	getaddrinfo(NULL, PUERTO, &hints, &serverInfo); // Notar que le pasamos NULL como IP, ya que le indicamos que use localhost en AI_PASSIVE


	/*
	 * 	Mediante socket(), obtengo el File Descriptor que me proporciona el sistema (un integer identificador).
	 *
	 */
	/* Necesitamos un socket que escuche las conecciones entrantes */
	int listeningSocket;
	listeningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
	if (listeningSocket == -1)
	{
		printf("Could not create socket");
	} else {
		puts("Socket created");
	}

	/*
	 * 	Todavia no estoy escuchando las conexiones entrantes!
	 *
	 */
	if(bind(listeningSocket,serverInfo->ai_addr, serverInfo->ai_addrlen) < 0)
	{
		//print the error message
		perror("bind failed. Error");
		return 1;
	} else {
	    puts("bind done");
	    freeaddrinfo(serverInfo); // Ya no lo vamos a necesitar
	}

	/*
	 * Solo me queda decirle que vaya y escuche!
	 *
	 */
	listen(listeningSocket, BACKLOG);		// IMPORTANTE: listen() es una syscall BLOQUEANTE.

	/*
	 * 	El sistema esperara hasta que reciba una conexion entrante...
	 * 	...
	 * 	...
	 * 	BING!!! Nos estan llamando! ¿Y ahora?
	 *
	 *	Aceptamos la conexion entrante, y creamos un nuevo socket mediante el cual nos podamos comunicar (que no es mas que un archivo).
	 *
	 *	¿Por que crear un nuevo socket? Porque el anterior lo necesitamos para escuchar las conexiones entrantes. De la misma forma que
	 *	uno no puede estar hablando por telefono a la vez que esta esperando que lo llamen, un socket no se puede encargar de escuchar
	 *	las conexiones entrantes y ademas comunicarse con un cliente.
	 *
	 *			Nota: Para que el listeningSocket vuelva a esperar conexiones, necesitariamos volver a decirle que escuche, con listen();
	 *				En este ejemplo nos dedicamos unicamente a trabajar con el cliente y no escuchamos mas conexiones.
	 *
	 */
	struct sockaddr_in addr;			// Esta estructura contendra los datos de la conexion del cliente. IP, puerto, etc.
	socklen_t addrlen = sizeof(addr);

	int socketCliente = accept(listeningSocket, (struct sockaddr *) &addr, &addrlen);
	if (socketCliente < 0)
	{
		perror("accept failed");
		return 1;
	} else {
	    puts("Connection accepted");
	}
	/*
	 * 	Ya estamos listos para recibir paquetes de nuestro cliente...
	 *
	 * 	Vamos a ESPERAR (ergo, funcion bloqueante) que nos manden los paquetes, y los imprimieremos por pantalla.
	 *
	 *	Cuando el cliente cierra la conexion, recv() devolvera 0.
	 */
	char package[PACKAGESIZE];
	printf("Antes del receive\n");
	recv(socketCliente, (void*) package, PACKAGESIZE, 0);
	printf("Despues del receive\n");
	printf("%s", package);

	close(socketCliente);
	close(listeningSocket);

	return package;
}

int crearSocketCliente(char * PUERTO, char * IP){



	/*
	 *  Obtiene los datos de la direccion de red y lo guarda en serverInfo.
	 *
	 */
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	getaddrinfo(IP, PUERTO, &hints, &serverInfo);	// Carga en serverInfo los datos de la conexion


	/*
	 *
	 * 	Obtiene un socket (un file descriptor -todo en linux es un archivo-), utilizando la estructura serverInfo que generamos antes.
	 *
	 */
	int serverSocket;
	serverSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
	if (serverSocket == -1)
	{
		printf("Could not create socket");
	} else {
		puts("Socket created");
	}
	/*
	 * 	Perfecto, ya tengo el medio para conectarme (el archivo), y ya se lo pedi al sistema.
	 * 	Ahora me conecto!
	 *
	 */
	if (connect(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen))
	{
		perror("connect failed. Error");
		return 1;
	} else {
		puts("Connected\n");
	}

	freeaddrinfo(serverInfo);	// No lo necesitamos mas

	/*
	 *	Estoy conectado! Ya solo me queda una cosa:
	 *
	 *	Enviar datos!
	 *
	 *	Vamos a crear un paquete (en este caso solo un conjunto de caracteres) de size PACKAGESIZE, que le enviare al servidor.
	 *
	 *	Aprovechando el standard immput/output, guardamos en el paquete las cosas que ingrese el usuario en la consola.
	 *	Ademas, contamos con la verificacion de que el usuario escriba "exit" para dejar de transmitir.
	 *
	 */
	int enviar = 1;
	char message[PACKAGESIZE];

	char server_reply[PACKAGESIZE];

	while(enviar){
		recv(serverSocket , server_reply , PACKAGESIZE , 0);
		puts(server_reply);
		fgets(message, PACKAGESIZE, stdin);			// Lee una linea en el stdin (lo que escribimos en la consola) hasta encontrar un \n (y lo incluye) o llegar a PACKAGESIZE.
		if (!strcmp(message,"exit\n")) enviar = 0;			// Chequeo que el usuario no quiera salir
		if (enviar) send(serverSocket, message, PACKAGESIZE, 0); 	// Solo envio si el usuario no quiere salir.
	}


	/*
	 *	Asique ahora solo me queda cerrar la conexion con un close();
	 */

	close(serverSocket);
	return 0;
}
