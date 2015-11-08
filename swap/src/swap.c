#include "swap.h"


//valores cfg
char * puertoEscucha;
char * nombreSwap;
int cantPaginas;
int tamanioPaginas;
int retardoSwap;
int retardoCompactacion;


char instruccion[20];
int nbytes;
char respuesta[30];

int main() {

	archivoLog = log_create("swap.log", "Swap", false, 2);

	//Creación de listas
	listaLibres = list_create();
	listaProcesos = list_create();
	listaEspera = list_create();
	//carga Cfgs
	levantarCfgInicial();

	//creacion archivo particion
	crearParticion();
	for(;(socketMemoria > 0);){
		t_nodo_mem_swap * nodoMemSwap = malloc(sizeof(t_nodo_mem_swap));
		recibirNodoDeMem(nodoMemSwap);
		estructuraRecibida(nodoMemSwap);
	}
	return 1;
}

void estructuraRecibida(t_nodo_mem_swap * nodoMemSwap){
	nodoRespuesta = malloc(sizeof(t_resp_swap_mem));
	nodoRespuesta->contenido = malloc(strlen("")+1);
	strcpy(nodoRespuesta->contenido,"\0");
	switch (nodoMemSwap->tipo) {
		case INICIAR:
			nodoRespuesta->tipo = INICIAR;
			recibirProceso(nodoMemSwap->pid,nodoMemSwap->pagina);
			break;
		case LEER:
			nodoRespuesta->tipo = LEER;
			leerPaginaProceso(nodoMemSwap->pid,nodoMemSwap->pagina);
			break;
		case ESCRIBIR:
			nodoRespuesta->tipo = ESCRIBIR;
			escribirPagina(nodoMemSwap->pid,nodoMemSwap->pagina, nodoMemSwap->contenido);
		break;
		case FINALIZAR:
			nodoRespuesta->tipo = FINALIZAR;
			eliminarProceso(nodoMemSwap->pid);
		break;
	}
}

void levantarCfgInicial(){
	//Levanta sus puertos cfg e ip para conectarse
	char directorioActual[1024];
	getcwd(directorioActual, sizeof(directorioActual));
	strcat(directorioActual, "/swap/src/config.cfg");//Consola
	//strcat(directorioActual, "/src/config.cfg");//Eclipse
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
	int i;

	//modificar para el directorio real
	particion=fopen("swap.data","wb+");
	//putc('A',particion);
	for (i = 0; i < cantPaginas * tamanioPaginas; i++){
		putc('\0',particion);
	}
	fclose(particion);

	list_add(listaLibres, crearNodoLibre(0, cantPaginas));

}

void configurarSocketServer(){
	//se conecta con el swap que tiene un servidor escuchando
	socketServidor = socketCrearServidor(puertoEscucha,"Swap");
	if (socketServidor > 0){
		socketMemoria = socketAceptarConexion(socketServidor,"Swap","Memoria");
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

void recibirProceso(int idProc, int cantPagProceso)
{
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
		log_info(archivoLog, "Proceso Recibido PID: %d, Indice: %d, Tamanio: %d", idProc, nodoLibre->indice, cantPagProceso);
		//TODO MODIFICAR LA LISTA DE DISPONIBLES
		nodoRespuesta->exito = 1;
	} else {
		//No hay elementos libres, no puedo alojar.. Rechazo
		nodoRespuesta->exito = 0;
		perror("no hay espacio para el proceso");
		log_info(archivoLog, "No hay espacio para alojar el proceso PID: %d", idProc);
	}
	enviarMensajeRtaAMem(nodoRespuesta);
	printf("lista procesos (elementos) : %d \n",listaProcesos->elements_count);
}

void eliminarProceso(int pid)
{
	bool condicionProcAEliminar(t_nodoProceso * nodoProceso) {
		return (nodoProceso->idProc == pid);
	}

	t_nodoProceso * nodoProceso = NULL;
	nodoProceso = list_find(listaProcesos,(void*) condicionProcAEliminar);
	if (nodoProceso != NULL){

		//encontro el nodo a eliminar
		//TODO crear el nodo libre correspondiente al espacio liberado
		list_add(listaLibres, crearNodoLibre(nodoProceso->indice, nodoProceso->tamanio));
		log_info(archivoLog, "Proceso eliminado PID: %d, Indice: %d, Tamanio: %d \n", pid, nodoProceso->indice, nodoProceso->tamanio);
		list_remove_by_condition(listaProcesos,(void*) condicionProcAEliminar);
		//TODO falta eliminar el nodo de memoria!!!!
		nodoRespuesta->exito = 1;
	} else {
		//no encontro el proceso indicado
		nodoRespuesta->exito = 0;
		log_info(archivoLog, "No se pudo eliminar el proceso PID: %d", pid);
		perror("no se encontró el proceso indicado");
	}
	printf("listaLibre (nodos): %d \n",listaLibres->elements_count);
	printf("lista procesos (elementos) : %d \n",listaProcesos->elements_count);
	//enviarMensajeRtaAMem(nodoRespuesta);
}

void leerPaginaProceso(int idProc, int pagina){

	char leerDelArchivo[1024];
	int indiceProceso;
	FILE *particion;

	//buscar indice del nodoProceso "indice"
	bool condicionLeer(t_nodoProceso * nodoProceso){
				return (nodoProceso->idProc == idProc);
	}
	t_nodoProceso * nodoProceso = NULL;
	nodoProceso = list_find(listaProcesos,(void*) condicionLeer);
	nodoRespuesta->exito = 0;

	if (nodoProceso != NULL){
		indiceProceso = nodoProceso->indice;
		int ubicacion = indiceProceso + (pagina * tamanioPaginas);
		//modificar para el directorio real
		//TODO cuidado con las direciones relativas
		particion=fopen("swap.data","r");

		if (particion==NULL){
			perror("No se pudo leer");
		} else {
			//Se ubica en +1 así que tiene que ser -1
			fseek(particion, SEEK_SET, ubicacion-1);
			//leer esa posicion como una lectura normal donde el tamaño total a leer es desde paginaReal hasta tamanioPagina (es el tamanio entero de la pag)
			if (fread(leerDelArchivo, sizeof(tamanioPaginas), 1, particion) > 0){
				//enviar mensaje
				strcat(leerDelArchivo,"\0");
				strcpy(respuesta,leerDelArchivo);
				nodoRespuesta->exito = 1;

				if (strlen(leerDelArchivo) == 0) {
					strcpy(leerDelArchivo, "NULL");
				}
				strcpy(nodoRespuesta->contenido,leerDelArchivo);

				//log_info(archivoLog, "Lectura realizada PID: %d, Indice: %d, Tamanio: %d \n", idProc, nodoProceso->indice, tamanioPaginas);
			}

			fclose(particion);
			log_info(archivoLog, "Lectura en el SWAP: ubicacion %d, valor %s del process ID %d, de la pagina %d.", ubicacion, leerDelArchivo, idProc, pagina);
		}
	}else{

		perror("no se encontró el proceso indicado");
	}
	enviarMensajeRtaAMem(nodoRespuesta);
}


void escribirPagina (int idProc, int pagina, char * texto) {
	FILE *particion;

	particion = fopen("swap.data","r+");

	bool condicionLeer(t_nodoProceso * nodoProceso){
		return (nodoProceso->idProc == idProc);
	}
	t_nodoProceso * nodoProceso = NULL;
	nodoProceso = list_find(listaProcesos,(void*) condicionLeer);

	int ubicacion = nodoProceso->indice + (pagina * tamanioPaginas);

	int err;
	if ((err = fseek(particion, ubicacion, SEEK_SET) == 0))
	{//se ubica bien
		//fwrite((const char *)texto, strlen((const char *)texto), 1, particion);
		fputs((const char *)texto, particion);
		nodoRespuesta->exito = 1;
	} else {
		//no se puede ubicar en esa posicion
		nodoRespuesta->exito = 0;
	}

    fclose(particion);
    //enviarMensajeRtaAMem(nodoRespuesta);
    log_info(archivoLog, "Escritura en el SWAP: ubicacion %d, valor %s del process ID %d, de la pagina %d.", ubicacion, texto, idProc, pagina);
}



int recibirNodoDeMem(t_nodo_mem_swap * nodo)
{
	unsigned char buffer[1024];
	int nbytes;
	if ((nbytes = recv(socketMemoria , buffer , sizeof(buffer) , 0)) < 0){
		printf("Swap: Error recibiendo mensaje de Memoria");
	} else if (nbytes == 0) {
		printf("Swap: Socket Memoria desconectado");
		socketMemoria = 0;
	}
	desempaquetarNodoDeMem(buffer,nodo);
	return nbytes;
}



void desempaquetarNodoDeMem(unsigned char *buffer,t_nodo_mem_swap * nodo)
{
	//t_pcb * pcb = malloc(sizeof(t_pcb));
	char contenido[50];
	unpack(buffer,SECUENCIA_NODO_RTA_SWAP_MEM,&nodo->tipo,&nodo->pid,&nodo->pagina,contenido);

	nodo->contenido = contenido;
}


int enviarMensajeRtaAMem(t_resp_swap_mem * nodo)
{
	int nbytes;
	unsigned char buffer[1024];
	empaquetarNodoRtaAMem(buffer,nodo);
	nbytes = send(socketMemoria, buffer, sizeof(buffer) , 0);
	if (nbytes == 0) {
		printf("Memoria: Socket CPU desconectado.\n");
		//TODO CERRAR LA CPU
	} else if (nbytes < 0){
		printf("Memoria: Socket CPU envío de mensaje fallido.\n");
		perror("Error - Enviando mensaje");
	}
	return nbytes;
}

void empaquetarNodoRtaAMem(unsigned char *buffer,t_resp_swap_mem * nodo)
{
	unsigned int tamanioBuffer;
	tamanioBuffer = pack(buffer,SECUENCIA_NODO_RTA_SWAP_MEM,
			nodo->tipo,nodo->exito,nodo->pagina,nodo->contenido);
}


