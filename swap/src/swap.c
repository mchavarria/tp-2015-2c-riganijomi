#include <stdio.h>
#include <string.h>
#include <commons/string.h>
#include <unistd.h>


int main() {

	char directorioActual[1024];
	char * puertoMemoria;
	char package[1024];

	getcwd(directorioActual, 1024);
	strcat(directorioActual, "/src/config.cfg");

	//obtener puerto memoria para conectarse
	puertoMemoria = configObtenerPuertoMemoria(directorioActual);


	//creacion archivo particion Swap
	char directorioSwap[1024];
	char * nombreArchivo;
	char comandoDD[100];

	getcwd(directorioSwap, sizeof(directorioSwap));
	strcat(directorioSwap, "/particion/");

	nombreArchivo = configObtenerNombreArchivoSwap(directorioActual);
	strcat(directorioSwap, nombreArchivo);

	char * armarStringComandoDD(char * directorio){
		strcpy(comandoDD, "dd if=/dev/zero of=");
		strcat(comandoDD, directorioSwap);
		strcat(comandoDD, " bs=512 count=1");
		return comandoDD;
	}

	system(armarStringComandoDD(directorioSwap));




	//conexion con Administrador de memoria
	int socketMemoria = socketCrearServidor(puertoMemoria);
	printf("socket devuelto: %d",socketMemoria);

	recv(socketMemoria,package, 1024, 0);

	puts(package);







	return 0;
}
