#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <string.h>


int main () {

	//Levanta sus puertos cfg e ip para conectarse y comunicarse con swap
	char directorioActual[1024];
	getcwd(directorioActual, sizeof(directorioActual));
	strcat(directorioActual, "/src/config.cfg");

	char * puertoSwap;
	char * ipSwap;

	puts(directorioActual);
	puertoSwap = configObtenerPuertoSwap(directorioActual);
	ipSwap = configObtenerIpSwap(directorioActual);

	//se conecta con el swap que tiene un servidor escuchando
	int socketSwap = socketCrearCliente(puertoSwap,ipSwap);
	printf("socket devuelto: %d",socketSwap);

	//prepara y envía un mensaje
	char mensaje[1024];
	strcpy(mensaje,"HOLAAMIGUEDEMIALMAAA");
	int estado = socketEnviarMensaje(socketSwap,mensaje); 	// Solo envio si el usuario no quiere salir.

	/*if (estado != -1){
		sleep(30);
		printf("respuesta: %s",socketRecibirMensaje(socketServidor);
	}*/
	return 0;
}

