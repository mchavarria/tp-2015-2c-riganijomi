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
	int continuar = 1;
	//carga Cfgs
	levantarCfgInicial();

	//creacion archivo particion
	crearParticion();
	for(;(socketMemoria > 0) && continuar;){
		t_nodo_mem * nodoInstruccion = malloc(sizeof(t_nodo_mem));
		t_decidirEstructuraSwap * decidirEstructuraSwap = malloc(sizeof(t_decidirEstructuraSwap));
		recv(socketMemoria, decidirEstructuraSwap,sizeof(t_decidirEstructuraSwap), 0);
		estructuraRecibida(decidirEstructuraSwap->mensaje);
	}
}

void estructuraRecibida(int mensaje){
	t_iniciarSwap * iniciarSwap = malloc(sizeof(t_iniciarSwap));
	t_pedirPaginaSwap * nodoPedirPaginaSwap = malloc(sizeof(t_pedirPaginaSwap));
	t_envioPaginaSwap * envioSwap = malloc(sizeof(t_envioPaginaSwap));
	t_eliminarPaginaSwap * nodoEliminar = malloc(sizeof(t_eliminarPaginaSwap));
	char finalizarError [10] ;
	switch (mensaje) {
		case 0:
			recv(socketMemoria, iniciarSwap,sizeof(t_iniciarSwap), 0);
			iniciar(iniciarSwap);
		break;
		case 1:
			recv(socketMemoria, nodoPedirPaginaSwap,sizeof(t_pedirPaginaSwap), 0);
			leer(nodoPedirPaginaSwap);
		break;
		case 2:
			recv(socketMemoria, envioSwap,sizeof(t_envioPaginaSwap), 0);
			envioSwap->valor = malloc(envioSwap->tamanioTexto + 1);
			recv(socketMemoria, envioSwap->valor, envioSwap->tamanioTexto, 0);
			strcpy(envioSwap->valor, string_substring(envioSwap->valor, 0, envioSwap->tamanioTexto));
			strcat(envioSwap->valor, "\0");
			escribir(envioSwap);
		break;
		case 3:
			recv(socketMemoria, nodoEliminar,sizeof(t_eliminarPaginaSwap), 0);
			finalizar(nodoEliminar);
		break;
	}
}

void iniciar(t_iniciarSwap * nodoIniciar){
	recibirProceso(nodoIniciar->processID,nodoIniciar->cantidadPaginas);
}

void leer(t_pedirPaginaSwap * nodoLeer){
	leerPaginaProceso(nodoLeer->processID,nodoLeer->numeroPagina);
}

void escribir(t_envioPaginaSwap * nodoEscribir){
	escribirPagina(nodoEscribir->processID, nodoEscribir->numeroPagina, nodoEscribir->valor);
}

void finalizar(t_eliminarPaginaSwap * nodoEliminar){
	eliminarProceso(nodoEliminar);
}

void levantarCfgInicial(){
	//Levanta sus puertos cfg e ip para conectarse
	char directorioActual[1024];
	getcwd(directorioActual, sizeof(directorioActual));
	strcat(directorioActual, "/swap/src/config.cfg");
	//strcat(directorioActual, "/src/config.cfg");
	t_config * archConfig = malloc(sizeof(t_config));
	archConfig = config_create(directorioActual);

	puertoEscucha = config_get_string_value(archConfig, "PUERTO_ESCUCHA");
	nombreSwap = configObtenerNombreArchivoSwap(directorioActual);
	tamanioPaginas = config_get_int_value(archConfig, "TAMANIO_PAGINA");
	cantPaginas = config_get_int_value(archConfig, "CANTIDAD_PAGINAS");
	retardoSwap = config_get_int_value(archConfig, "RETARDO_SWAP");
	retardoCompactacion = config_get_int_value(archConfig, "RETARDO_COMPACTACION");
	configurarSocketServer();
}

void crearParticion(){
	FILE *particion;
	int i;

	//modificar para el directorio real
	particion=fopen("swap.data","wb+");
	//putc('A',particion);
	for (i = 2; i < cantPaginas * tamanioPaginas; i++){
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

void recibirProceso(int idProc, int cantPagProceso){

	nodoRespuesta = malloc(sizeof(t_resp_swap_mem));
	nodoRespuesta->tipo = INICIAR;
	nodoRespuesta->largo = 0;//No devuelde nada mas
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
		//nbytes = socketEnviarMensaje(socketMemoria, nodoRespuesta,sizeof(t_resp_swap_mem));
		send(socketMemoria, nodoRespuesta, sizeof(t_resp_swap_mem), 0);

	} else {
		//No hay elementos libres, no puedo alojar.. Rechazo
		nodoRespuesta->exito = 0;
		perror("no hay espacio para el proceso");
		log_info(archivoLog, "No hay espacio para alojar el proceso PID: %d", idProc);
		send(socketMemoria, nodoRespuesta,sizeof(t_resp_swap_mem), 0);

	}
	printf("lista procesos (elementos) : %d \n",listaProcesos->elements_count);
}

void eliminarProceso(t_eliminarPaginaSwap * nodoEliminar){

	nodoRespuesta->tipo = FINALIZAR;
	nodoRespuesta->largo = 0;//No devuelve nada mas
	bool condicionProcAEliminar(t_nodoProceso * nodoProceso) {
		return (nodoProceso->idProc == nodoEliminar->processID);
	}

	t_nodoProceso * nodoProceso = NULL;
	nodoProceso = list_find(listaProcesos,(void*) condicionProcAEliminar);
	if (nodoProceso != NULL){

		//encontro el nodo a eliminar
		//TODO crear el nodo libre correspondiente al espacio liberado
		list_add(listaLibres, crearNodoLibre(nodoProceso->indice, nodoProceso->tamanio));
		log_info(archivoLog, "Proceso eliminado PID: %d, Indice: %d, Tamanio: %d \n", nodoEliminar->processID, nodoProceso->indice, nodoProceso->tamanio);
		list_remove_by_condition(listaProcesos,(void*) condicionProcAEliminar);
		//TODO falta eliminar el nodo de memoria!!!!
		nodoRespuesta->exito = 1;
		nbytes = socketEnviarMensaje(socketMemoria, nodoRespuesta,sizeof(t_resp_swap_mem));
	} else {
		//no encontro el proceso indicado
		nodoRespuesta->exito = 0;
		nbytes = socketEnviarMensaje(socketMemoria, nodoRespuesta,sizeof(t_resp_swap_mem));
		log_info(archivoLog, "No se pudo eliminar el proceso PID: %d", nodoEliminar->processID);
		perror("no se encontró el proceso indicado");
		strcpy(respuesta,"finalizar-fallo");
	}
	printf("listaLibre (nodos): %d \n",listaLibres->elements_count);
	printf("lista procesos (elementos) : %d \n",listaProcesos->elements_count);
}

void leerPaginaProceso(int idProc, int pagina){

	sleep(retardoSwap);

	char leerDelArchivo[1024];
	int indiceProceso;
	FILE *particion;

	//buscar indice del nodoProceso "indice"
	bool condicionLeer(t_nodoProceso * nodoProceso){
				return (nodoProceso->idProc == idProc);
	}
	t_nodoProceso * nodoProceso = NULL;
	nodoProceso = list_find(listaProcesos,(void*) condicionLeer);


	if (nodoProceso != NULL){
		//if (nodoProceso->tamanio <= (pagina * tamanioPaginas)) {

			indiceProceso = nodoProceso->indice;
			int ubicacion = indiceProceso + (pagina * tamanioPaginas);

			//modificar para el directorio real
			//TODO cuidado con las direciones relativas
			particion=fopen("swap.data","r");

			if (particion==NULL){
				perror("No se pudo leer");
			} else {
				//Se ubica en +1 así que tiene que ser -1
				fseek(particion, ubicacion, SEEK_SET);
				//leer esa posicion como una lectura normal donde el tamaño total a leer es desde paginaReal hasta tamanioPagina (es el tamanio entero de la pag)
				if (fread(leerDelArchivo, tamanioPaginas, 1, particion) > 0){
					//enviar mensaje
					strcat(leerDelArchivo,"\0");
					strcpy(respuesta,leerDelArchivo);
					nodoRespuesta->tipo = LEER;
					nodoRespuesta->exito = 1;
					nodoRespuesta->largo = sizeof(respuesta);

					if (strlen(leerDelArchivo) == 0) {
						strcpy(leerDelArchivo, "NULL");
					}
					send(socketMemoria, nodoRespuesta, sizeof(t_resp_swap_mem),0);
					send(socketMemoria, leerDelArchivo, nodoRespuesta->largo, 0);
					//log_info(archivoLog, "Lectura realizada PID: %d, Indice: %d, Tamanio: %d \n", idProc, nodoProceso->indice, tamanioPaginas);
				}

				fclose(particion);
				log_info(archivoLog, "Lectura en el SWAP: ubicacion %d, valor %s del process ID %d, de la pagina %d.", ubicacion, leerDelArchivo, idProc, pagina);
			}
/**		} else {
			log_error(archivoLog, "La pagina %d del proceso %d es invalida", pagina, idProc);
			nodoRespuesta->tipo = LEER;
			nodoRespuesta->exito = 0;
			nodoRespuesta->largo = 0;
			send(socketMemoria, nodoRespuesta,sizeof(t_resp_swap_mem),0);

		}
*/
	}else{
		perror("no se encontró el proceso indicado");
		nodoRespuesta->tipo = LEER;
		nodoRespuesta->exito = 0;
		nodoRespuesta->largo = 0;
		send(socketMemoria, nodoRespuesta,sizeof(t_resp_swap_mem),0);
	}
}


void escribirPagina (int idProc, int pagina, char * texto) {

	sleep(retardoSwap);

	FILE *particion;

	particion = fopen("swap.data","r+");

	bool condicionLeer(t_nodoProceso * nodoProceso){
		return (nodoProceso->idProc == idProc);
	}
	t_nodoProceso * nodoProceso = NULL;
	nodoProceso = list_find(listaProcesos,(void*) condicionLeer);

	t_resp_swap_mem * nodoRespuesta = malloc(sizeof(t_resp_swap_mem));
	nodoRespuesta->tipo=ESCRIBIR;

//	if (nodoProceso->tamanio < (pagina * tamanioPaginas)) {
		int ubicacion = nodoProceso->indice + (pagina * tamanioPaginas);

		fseek(particion, ubicacion, SEEK_SET);

		//fwrite((const char *)texto, strlen((const char *)texto), 1, particion);
		fputs((const char *)texto, particion);

		fclose(particion);

		log_info(archivoLog, "Escritura en el SWAP: ubicacion %d, valor %s del process ID %d, de la pagina %d.", ubicacion, texto, idProc, pagina);

		nodoRespuesta->exito =1;
/**
	}

else {
		nodoRespuesta->exito =0;
	}
*/
	send(socketMemoria, nodoRespuesta, sizeof(t_resp_swap_mem), 0);
}

