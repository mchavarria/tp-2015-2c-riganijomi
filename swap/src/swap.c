#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "commons/collections/list.h"C
#include "swap.h"
#include <stdbool.h>

void sem_sockets(){

}
void sem_mem(){

}
//valores cfg
char * puertoEscucha;
char * nombreSwap;
char * cantPaginas;
char * tamanioPaginas;
char * retardoSwap;
char * retardoCompactacion;

int pagNecesariasAux;


int main() {



	listaLibres = list_create();
	listaProcesos = list_create();
	listaEspera = list_create();

	levantarCfgInicial();
	crearParticion();


	printf("%d",listaLibres->elements_count);
	printf("%d",listaProcesos->elements_count);
	printf("%d",listaEspera->elements_count);

	recibirProceso(1,5);

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

	list_add(listaLibres, crearNodoLibre(0, cantPag));

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

static t_nodoLibre *crearNodoLibre(int indice, int tamanio) {
	t_nodoLibre *nodoLibre = malloc(sizeof(t_nodoLibre));
	nodoLibre->indice = indice;
	nodoLibre->tamanio = tamanio;
    return nodoLibre;
}
static t_nodoEspera *crearNodoEspera(int idProc, int cantPagProceso) {
	t_nodoEspera *nodoEspera = malloc(sizeof(t_nodoEspera));
	nodoEspera->idProc = idProc;
    return nodoEspera;
}
static t_nodoProceso *crearNodoProceso(int idProc, int indice, int cantPagProceso) {
	t_nodoProceso *nodoProceso = malloc(sizeof(t_nodoProceso));
	nodoProceso->idProc = idProc;
	nodoProceso->tamanio = cantPagProceso;
	nodoProceso->indice = indice;
	printf("nodo: pid %d, indice %d, tamanio %d",idProc,indice,cantPagProceso);
    return nodoProceso;
}

void recibirProceso(int idProc, int cantPagProceso){

	//Salvo en la auxiliar para poder usarla en condicion
	pagNecesariasAux = cantPagProceso;
	puts("antes list_find");

	bool condicionRecibir(t_nodoLibre * nodoLibre) {

		return (nodoLibre->tamanio >= pagNecesariasAux);

	}

	t_nodoLibre * nodoLibre = NULL;
	nodoLibre = list_find(listaLibres,(void*) condicionRecibir);
	puts("antes");
	printf("%d",listaProcesos->elements_count);
	if (nodoLibre != NULL){

		//hay lugar para alojarlo
		list_add(listaProcesos, crearNodoProceso(idProc, nodoLibre->indice, cantPagProceso));

	} else {
		//No hay elementos libres, no puedo alojar.. Rechazo
	}
	puts("dsp de agregar");
	printf("%d",listaProcesos->elements_count);
}


