#include "socketCliente.h"

#define PACKAGESIZE 1024

/*
 * Crea un socket servidor para la solicitud
 * antes de solicitar deben definir la variable donde alojan al servidor
 * int serverSocket;
 */
int socketCrearCliente(char * PUERTO, char * IP, char * procesoNombre, char * procesoServidor){

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
		printf("%s: No se puede crear el socket para conectar a %s\n", procesoNombre,procesoServidor);
	} else {
		printf("%s: Socket creado para conectar a %s\n", procesoNombre,procesoServidor);
		//Ahora me conecto!
		if (connect(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen))
		{
			printf("%s: Conexión fallida con %s\n", procesoNombre,procesoServidor);
			perror("Error - Conexión Fallida");
			serverSocket = 0;
		} else {
			printf("%s: Contectado con %s\n", procesoNombre,procesoServidor);
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

int socketEnviarMensaje(int serverSocket, char  * mensaje, int longitudMensaje) {
	int nbytes;
		if ((nbytes = send(serverSocket, mensaje, longitudMensaje , 0)) <= 0){
			puts("error enviando mensaje");
			// Error o conexion cerrada por el cliente
			if (nbytes == 0) {
				///////////////////////////
				//LOGUEAR conexion cerrada
				printf("servidor socket %d desconectado\n", serverSocket);
				///////////////////////////
				///////////////////////////
			} else {
				///////////////////////////
				perror("envio al servidor error");
				///////////////////////////
			}
		}
		return nbytes;
}

int socketRecibirMensaje(int serverSocket, char * mensaje, int longitudMensaje) {
	int nbytes;
	if ((nbytes = recv(serverSocket , mensaje , longitudMensaje , 0)) <= 0){
		puts("error recibiendo mensaje");
		// Error o conexion cerrada por el cliente
		if (nbytes == 0) {
			///////////////////////////
			//LOGUEAR conexion cerrada
			printf("servidor MEM: socket %d desconectado\n", serverSocket);
			///////////////////////////
			///////////////////////////
		} else {
			///////////////////////////
			perror("recepcion error");
			///////////////////////////
		}
	}
	return nbytes;
}


/*
 * Cierra la conexión con el servidor
 */
void socketCerrarSocket(int serverSocket) {
	if (close(serverSocket) == -1){
		//-1 Indica error en el envío
		puts("error cerrando el socket servidor");
	}
}
