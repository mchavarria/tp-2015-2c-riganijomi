#include "swap.h"

//valores cfg
char * puertoEscucha;
char * nombreSwap;
char * cantPaginas;
char * tamanioPaginas;
char * retardoSwap;
char * retardoCompactacion;


char instruccion[20];
int nbytes;
char respuesta[30];

int main() {

	//Creación de listas
	listaLibres = list_create();
	listaProcesos = list_create();
	listaEspera = list_create();

	//carga Cfgs
	levantarCfgInicial();

	//creacion archivo particion
	crearParticion();
	for(;(socketMemoria > 0);){
		t_nodo_mem * nodoInstruccion = malloc(sizeof(t_nodo_mem));
		//sem_wait(&sem_mem);
		nbytes = socketRecibirMensaje(socketMemoria, nodoInstruccion,sizeof(t_nodo_mem));
		// tengo un mensaje de algun cliente
		if (nbytes <= 0) {
			// Error o conexion cerrada por el cliente
			if (nbytes == 0) {
				printf("servidor MEM: socket %d desconectado\n", socketMemoria);
			} else {
				perror("recepcion error");
			}
			//monitorEliminarSocket(socketCpu);
		} else {
			//Mensaje
			interpretarLinea(nodoInstruccion);
			nbytes = socketEnviarMensaje(socketMemoria, respuesta,sizeof(respuesta));
		}
	}
	/*
	//Funciona.. deberia ser a partir de un mensaje de iniciar.
	recibirProceso(1,5);
	puts("dps recibir proceso");
	leerPaginaProceso(1,1);
	puts("dps consultarParticion");

	eliminarProceso(1);
	puts("dps eliminar proceso");
	 */
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
	configurarSocketServer();
}

void crearParticion(){
	FILE *particion;
	int cantPag = 64;
	int tamPag = 4;
	int i;

	//modificar para el directorio real
	particion=fopen("swap.data","w");
	putc('A',particion);
	for (i = 2; i < cantPag * tamPag; i++){
		putc('\0',particion);
	}
	fclose(particion);

	list_add(listaLibres, crearNodoLibre(0, cantPag));

}

void configurarSocketServer(){
	//se conecta con el swap que tiene un servidor escuchando
	socketServidor = socketCrearServidor(puertoEscucha);
	if (socketServidor > 0){
		socketMemoria = socketAceptarConexion(socketServidor);
	}
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


void interpretarLinea(t_nodo_mem * nodoInstruccion) {

    int valor;
    int pid = nodoInstruccion->pid;
    if (esElComando(nodoInstruccion->instruccion, "iniciar")) {
		valor = atoi(devolverParteUsable(nodoInstruccion->instruccion, 8));
		recibirProceso(pid,valor);
	} else if (esElComando(nodoInstruccion->instruccion, "leer")) {
		valor = atoi(devolverParteUsable(nodoInstruccion->instruccion, 5));
		leerPaginaProceso(pid,valor);
	} else if (esElComando(nodoInstruccion->instruccion, "escribir")) {
		char * rta;
		rta = string_substring(nodoInstruccion->instruccion, 9, 1);
		valor = devolverParteUsable(nodoInstruccion->instruccion, 11);
		strcpy(respuesta,"escribir");
	} else if (esElComando(nodoInstruccion->instruccion, "finalizar")) {
		eliminarProceso(pid);
	} else {
		perror("comando invalido");
		strcpy(respuesta,"error-comando");
	}
}

void recibirProceso(int idProc, int cantPagProceso){

	//Salvo en la auxiliar para poder usarla en condicion
	bool condicionRecibir(t_nodoLibre * nodoLibre) {

		return (nodoLibre->tamanio >= cantPagProceso);

	}
	t_nodoLibre * nodoLibre = NULL;
	nodoLibre = list_find(listaLibres,(void*) condicionRecibir);
	printf("cant listaProc: %d \n",listaProcesos->elements_count);
	if (nodoLibre != NULL){
		//hay lugar para alojarlo
		list_add(listaProcesos, crearNodoProceso(idProc, nodoLibre->indice, cantPagProceso));
		//TODO MODIFICAR LA LISTA DE DISPONIBLES
		strcpy(respuesta,"iniciar-exito");
	} else {
		//No hay elementos libres, no puedo alojar.. Rechazo
		perror("no hay espacio para el proceso");
		strcpy(respuesta,"iniciar-error");
	}
	printf("lista procesos (elementos) : %d \n",listaProcesos->elements_count);
}

void eliminarProceso(int idProc){

	bool condicionProcAEliminar(t_nodoProceso * nodoProceso) {

		return (nodoProceso->idProc == idProc);

	}

	t_nodoProceso * nodoProceso = NULL;
	nodoProceso = list_find(listaProcesos,(void*) condicionProcAEliminar);
	if (nodoProceso != NULL){

		//encontro el nodo a eliminar
		//TODO crear el nodo libre correspondiente al espacio liberado
		list_add(listaLibres, crearNodoLibre(nodoProceso->indice, nodoProceso->tamanio));
		list_remove_by_condition(listaProcesos,(void*) condicionProcAEliminar);
		//TODO falta eliminar el nodo de memoria!!!!
		strcpy(respuesta,"finalizar-exito");
	} else {
		//no encontro el proceso indicado
		perror("no se encontró el proceso indicado");
		strcpy(respuesta,"finalizar-fallo");
	}
	printf("listaLibre (nodos): %d \n",listaLibres->elements_count);
	printf("lista procesos (elementos) : %d \n",listaProcesos->elements_count);
}

void leerPaginaProceso(int idProc, int pagina){

	int pagTam = atoi(tamanioPaginas);
	char resp[pagTam];
	int indiceProceso;
	FILE *particion;

	//buscar indice del nodoProceso "indice"
	bool condicionLeer(t_nodoProceso * nodoProceso){
				return (nodoProceso->idProc == idProc);
	}
	t_nodoProceso * nodoProceso = NULL;
	nodoProceso = list_find(listaProcesos,(void*) condicionLeer);


	if (nodoProceso != NULL){
		indiceProceso = nodoProceso->indice;
		int ubicacion = indiceProceso + (pagina * pagTam);

		//modificar para el directorio real
		//TODO cuidado con las direciones relativas
		particion=fopen("swap.data","r");

		if (particion==NULL){
			perror("No se pudo leer");
		} else {
			//Se ubica en +1 así que tiene que ser -1
			fseek(particion, SEEK_SET, ubicacion-1);
			//leer esa posicion como una lectura normal donde el tamaño total a leer es desde paginaReal hasta tamanioPagina (es el tamanio entero de la pag)
			if (fread(resp, sizeof(pagTam), 1, particion) > 0){
				//enviar mensaje
				strcat(resp,"\0");
				strcpy(respuesta,resp);

			}

			fclose(particion);
		}
	}else{

		perror("no se encontró el proceso indicado");
		strcpy(respuesta,"lectura-error");
	}

}
/*
void escribirPagina () {
	particion=fopen("swap.data","w+");
	fseek( fp, 7, SEEK_SET );
    fputs(" C Programming Language", fp);
}
*/
