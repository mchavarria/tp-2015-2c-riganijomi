#include <stdio.h>


int main() {

	//obtener puerto escucha para conectarse con memoria
	//Levanta sus puertos cfg e ip para conectarse y comunicarse con swap
	char directorioActual[1024];
	getcwd(directorioActual, sizeof(directorioActual));
	strcat(directorioActual, "/src/config.cfg");

	char * puertoMemoria;
	char package[1024];

	puts(directorioActual);
	puertoMemoria = configObtenerPuertoMemoria(directorioActual);


	//se conecta con la memoria
	int socketMemoria = socketCrearServidor(puertoMemoria);
	printf("socket devuelto: %d",socketMemoria);

	recv(socketMemoria,package, 1024, 0);

	puts(package);




	//creacion de archivo con comando dd
/*	char * puertoEscucha;
	char directorioActual[1024];

	getcwd(directorioActual, sizeof(directorioActual));
	strcat(directorioActual, "/src/config.cfg");
	puts(directorioActual);
	puertoEscucha = configObtenerPuertoEscucha(directorioActual);

	char * nombreArchivo;
	char directorioSwap[1024];
	getcwd(directorioSwap, sizeof(directorioSwap));
	strcat(directorioSwap, "/particion/");
	strcat(directorioSwap, nombreArchivo);
	nombreArchivo = puts(configObtenerNombreArchivoSwap(directorioActual));

	system ("dd if=/dev/zero of=/home/utnso/projects/nombreArchivo bs=512 count=1");
*/

	return 0;
}

