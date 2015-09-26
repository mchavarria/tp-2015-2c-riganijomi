#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//valores cfg
char * puertoEscucha;
char * nombreSwap;
char * cantPaginas;
char * tamanioPaginas;
char * retardoSwap;
char * retardoCompactacion;


int main() {

	levantarCfgInicial();
	crearParticion();



	return 0;
}

void levantarCfgInicial(){
	//Levanta sus puertos cfg e ip para conectarse
	char directorioActual[1024];
	getcwd(directorioActual, sizeof(directorioActual));
	strcat(directorioActual, "/src/config.cfg");

	puertoEscucha = configObtenerPuertoEscucha(directorioActual);
	nombreSwap = configObtenerNombreArchivoSwap(directorioActual);
	cantPaginas = configObtenerCantPaginasSwap(directorioActual);
	tamanioPaginas = configObtenerTamPaginasSwap(directorioActual);
	retardoSwap = configObtenerRetardoSwap(directorioActual);
	retardoCompactacion = configObtenerRetardoCompactacionSwap(directorioActual);

}

void crearParticion(){
	FILE *particion;
	int cantPag = 64;
	int tamPag = 4;
	int i;
	particion=fopen("swap.data","w");
	for (i = 1; i < cantPag * tamPag; i++){
		putc('/0',particion);
	}
	fclose(particion);
}

void configurarSocketServer(){
	/*
	//conexion con Administrador de memoria
	int socketMemoria = socketCrearServidor(puertoMemoria);
	printf("socket devuelto: %d",socketMemoria);

	recv(socketMemoria,package, 1024, 0);

	puts(package);
	*/
}
