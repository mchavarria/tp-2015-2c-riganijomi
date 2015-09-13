#include <stdio.h>
#include <unistd.h>

/*
 * Reemplazar el contenido dentro de la función en cualquier otro main
 * Emula un servidor a la espera de un mensaje
 */
int main () {

	/*char directorioActual[1024];
	  getcwd(directorioActual, sizeof(directorioActual));
	  strcat(directorioActual, "/src/config.cfg");
	  puts(configObtenerPuertoEscucha(directorioActual));
	  */
	int socketCliente = socketCrearServidor("6667");
	printf("socket devuelto: %d \n",socketCliente);
	char mensaje[1024];
	socketRecibirMensaje(socketCliente, mensaje);
	printf("respuesta: %s \n",mensaje);
	sleep(30);
	return 0;
}
