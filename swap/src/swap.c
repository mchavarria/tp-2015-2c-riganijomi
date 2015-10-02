#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "commons/collections/list.h"C
#include "swap.h"
#include <stdbool.h>

//valores cfg
char * puertoEscucha;
char * nombreSwap;
char * cantPaginas;
char * tamanioPaginas;
char * retardoSwap;
char * retardoCompactacion;



int main() {

	//Creación de listas
	listaLibres = list_create();
	listaProcesos = list_create();
	listaEspera = list_create();

	//carga Cfgs
	levantarCfgInicial();

	//creacion archivo particion
	crearParticion();


	//Funciona.. deberia ser a partir de un mensaje de iniciar.
	recibirProceso(1,5);
	puts("dps recibir proceso");
	leerPaginaProceso(1,1);
	puts("dps consultarParticion");

	eliminarProceso(1);
	puts("dps eliminar proceso");

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

	//modificar para el directorio real
	particion=fopen("swap.data","w");
	for (i = 1; i < cantPag * tamPag; i++){
		putc('\A',particion);
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
	printf("nodo nuevo: pid %d, indice %d, tamanio %d \n",idProc,indice,cantPagProceso);
    return nodoProceso;
}

void recibirProceso(int idProc, int cantPagProceso){

	//Salvo en la auxiliar para poder usarla en condicion
	puts("antes list_find");

	bool condicionRecibir(t_nodoLibre * nodoLibre) {

		return (nodoLibre->tamanio >= cantPagProceso);

	}

	t_nodoLibre * nodoLibre = NULL;
	nodoLibre = list_find(listaLibres,(void*) condicionRecibir);
	puts("antes");
	printf("cant listaProc: %d \n",listaProcesos->elements_count);
	if (nodoLibre != NULL){

		//hay lugar para alojarlo
		list_add(listaProcesos, crearNodoProceso(idProc, nodoLibre->indice, cantPagProceso));

	} else {
		//No hay elementos libres, no puedo alojar.. Rechazo
		perror("no hay espacio para el proceso");
	}
	puts("dsp de agregar");
	printf("lista procesos (elementos) : %d \n",listaProcesos->elements_count);
}

void eliminarProceso(int idProc){
	bool condicionProcAEliminar(t_nodoProceso * nodoProceso) {

		return (nodoProceso->idProc == idProc);

	}

	t_nodoProceso * nodoProceso = NULL;
	nodoProceso = list_find(listaProcesos,(void*) condicionProcAEliminar);
	puts("eliminado");
	if (nodoProceso != NULL){

		//encontro el nodo a eliminar
		//crear el nodo libre correspondiente al espacio liberado
		list_add(listaLibres, crearNodoLibre(nodoProceso->indice, nodoProceso->tamanio));
		list_remove_by_condition(listaProcesos,(void*) condicionProcAEliminar);
		//falta eliminar el nodo de memoria!!!!

		} else {
			//no encontro el proceso indicado
			perror("no se encontró el proceso indicado");
		}
	printf("listaLibre (nodos): %d \n",listaLibres->elements_count);
	printf("lista procesos (elementos) : %d \n",listaProcesos->elements_count);
}

void leerPaginaProceso(int idProc, int pagina){

	int contador = 0;
	FILE *particion;

	t_nodoProceso * nodoProceso = NULL;
	int indiceProceso;
	//paginaReal calculada
	//int paginaReal = indiceProceso + (pagina*tamanioPaginas);


	//buscar indice del nodoProceso "indice"
	bool condicionLeer(t_nodoProceso * nodoProceso){
				return (nodoProceso->idProc == idProc);
	}

	nodoProceso = list_find(listaProcesos,(void*) condicionLeer);


	if (nodoProceso != NULL){
		indiceProceso = nodoProceso->indice;


		//modificar para el directorio real
			particion=fopen("swap.data","r");

			if (particion==NULL){
				printf("No se pudo leer");
				return;
			}

			//leer esa posicion como una lectura normal donde el tamaño total a leer es desde paginaReal hasta tamanioPagina (es el tamanio entero de la pag)
			fread(nodoProceso, sizeof(t_nodoProceso), 1, particion);

			while(!feof(particion)){
				fseek(particion,indiceProceso,SEEK_SET);
				fread(&nodoProceso, sizeof(t_nodoProceso), 1, particion);

			 	if (contador == 0){
			 		//retornar el valor leido
			 		puts("proceso leido");
			 	}
			 	contador++;
			 }

			fclose(particion);

	}else{

		perror("no se encontró el proceso indicado");

	}

}


/*
void leerPaginaProceso(int idProc, int pagina){

	int contador = 0;
	t_nodoProceso * nodoProceso = NULL;
	int paginaReal = nodoProceso->indice + (pagina * tamanioPaginas);

	FILE *particion;

	particion=fopen("swap.data", "r");

	 if (particion==NULL){
		printf("No se pudo leer");
	 	return;
	 }

	 while(!feof(particion) || nodoProceso->idProc == idProc){
		fseek(particion, nodoProceso, SEEK_SET);
		fread(&nodoProceso, sizeof(t_nodoProceso), 1, particion);


	 	if (contador == 0){
	 		//retornar el valor leido
	 		puts("proceso leido");
	 	}
	 	contador++;
	 }


	 fclose(particion);
}*/


//armar funcion para que identifique que mensaje le llega

