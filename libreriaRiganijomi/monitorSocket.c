//Monitorea multiples sockets

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

extern sem_t sem_mem;
extern sem_t sem_sockets;
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
    servidor = socketCrearServidor(puerto);
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
                	sem_wait(&sem_sockets);
                	//ASIGNAR EL SOCKET A LA VARIABLE COMPARTIDA CON EL PADRE

					//LEVANTAR SEMAFOROOOOOO DEL PADRE

                	sem_post(&sem_mem);
                } // fin recepcion de mensajes de cliente
            } // fin nuevas conexiones
        } // fin recorrido del set
    } // fin del for(;;)
}

void monitorEliminarSocket(int socket){
	FD_CLR(socket, &coleccion); // eliminarlo de la coleccion de sockets
	socketCerrarSocket(socket);
}
