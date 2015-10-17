//Monitorea multiples sockets

#include "monitorSocket.h"

fd_set coleccion;    // coleccion de sockets
fd_set coleccionTemp;  // coleccionTemp de sockets temporal

void  *monitorPrepararServidor(void *argumento ){
	char *puerto = (char *) argumento;

    int colMax;        // maximo num de sockets

    int servidor;     // socket Servidor
    int nuevoCliente;        // socket cliente recibido
    struct sockaddr_storage remoteaddr; // direccion del cliente
    socklen_t addrlen;
    int i;

    FD_ZERO(&coleccion);    // Limpia los sets de colecciones
    FD_ZERO(&coleccionTemp);

    ///////////////////////////
    ///////////////////////////
    //Crear el socket servidor
    servidor = crearServidor(puerto);
    ///////////////////////////
    ///////////////////////////

    // agrega el servidor a la colección
    FD_SET(servidor, &coleccion);

    // actualiza el num max de la cantidad
    colMax = servidor;

    // main loop
    for(;;) {
        coleccionTemp = coleccion; // lo copio para no perderlo
        if (select(colMax+1, &coleccionTemp, NULL, NULL, NULL) == -1) {
            perror("select");
            ///////////////////////////
            //LOGUEAR EL ERROR
            ///////////////////////////
        }

        // busca el socket que tiene dato por leer
        for(i = 0; i <= colMax; i++) {
            if (FD_ISSET(i, &coleccionTemp)) {
                if (i == servidor) {
                    // El servidor tiene que aceptar un nuevo cliente
                	//Aceptamos la conexion entrante, y creamos un nuevo socket mediante el cual nos podamos comunicar.
					addrlen = sizeof remoteaddr;
                    nuevoCliente = accept(servidor,
                        (struct sockaddr *)&remoteaddr,
                        &addrlen);

                    if (nuevoCliente == -1) {
                        perror("accept");
                        ///////////////////////////
                        ///////////////////////////
                        //LOGUEAR ERROR
                        ///////////////////////////
                        ///////////////////////////
                    } else {
                        FD_SET(nuevoCliente, &coleccion); // agrega el cliente al set de sockets

                        if (nuevoCliente > colMax) {    // actualiza el maximo
                            colMax = nuevoCliente;
                        }

                    }
                } else {

                	//ASIGNAR EL SOCKET A LA VARIABLE COMPARTIDA CON EL PADRE

					//LEVANTAR SEMAFOROOOOOO DEL PADRE

                } // fin recepcion de mensajes de cliente
            } // fin nuevas conexiones
        } // fin recorrido del set
    } // fin del for(;;)
}


int crearServidor(char * PUERTO){

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

void monitorEliminarSocket(int socket){
	FD_CLR(socket, &coleccion); // eliminarlo de la coleccion de sockets
	if (close(socket) == -1){
			//-1 Indica error en el envío
			puts("error cerrando el socket");
		}
}
