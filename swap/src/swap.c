#include "swap.h"

int main() {

	sem_init(&mutexAtendiendoEspera,0,1);
	archivoLog = log_create("swap.log", "Swap", false, 2);

	//Creación de listas
	listaLibres = list_create();
	listaProcesos = list_create();
	listaEspera = list_create();
	listaMetricas = list_create();
	//carga Cfgs
	levantarCfgInicial();
	t_nodo_mem_swap * nodoMemSwap = malloc(sizeof(t_nodo_mem_swap));
	nodoMemSwap->contenido = malloc(1);
	//creacion archivo particion
	crearParticion();
	for(;(socketMemoria > 0);){
		recibirNodoDeMem(nodoMemSwap);
		estructuraRecibida(nodoMemSwap);
	}
	return 1;
}

void estructuraRecibida(t_nodo_mem_swap * nodoMemSwap){
	nodoRespuesta = malloc(sizeof(t_resp_swap_mem));
	nodoRespuesta->contenido = malloc(1);
	strcpy(nodoRespuesta->contenido,"\0");
	switch (nodoMemSwap->tipo) {
		case INICIAR:
			nodoRespuesta->tipo = INICIAR;
			sem_wait(&mutexAtendiendoEspera);
			recibirProceso(nodoMemSwap->pid,nodoMemSwap->pagina);
			sem_post(&mutexAtendiendoEspera);

			break;
		case LEER:
			nodoRespuesta->tipo = LEER;
			leerPaginaProceso(nodoMemSwap->pid,nodoMemSwap->pagina);
			usleep(retardoSwap * 1000000);
			break;
		case ESCRIBIR:
			escribirPagina(nodoMemSwap->pid,nodoMemSwap->pagina, nodoMemSwap->contenido);
			usleep(retardoSwap * 1000000);
		break;
		case FINALIZAR:
			puts("Entro");
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
	if (hayFragmentacion == 0){
		bool condicionRecibir(t_nodoLibre * nodoLibre) {

			return (nodoLibre->tamanio >= cantPagProceso);

		}
		t_nodoLibre * nodoLibre = NULL;
		nodoLibre = list_find(listaLibres,(void*) condicionRecibir);
		if (nodoLibre != NULL){
			list_add(listaMetricas, metricas_create(idProc));
			//hay lugar para alojarlo
			list_add(listaProcesos, crearNodoProceso(idProc, nodoLibre->indice, cantPagProceso));
			log_info(archivoLog, "Proceso Recibido PID: %d, Indice: %d, Tamanio: %d", idProc, nodoLibre->indice, cantPagProceso);
			nodoLibre->indice = nodoLibre->indice + cantPagProceso;
			nodoLibre->tamanio = nodoLibre->tamanio - cantPagProceso;
			nodoRespuesta->exito = 1;
		} else {
			hayFragmentacion = fragmentacionExterna(cantPagProceso);
			if (hayFragmentacion){
				//necesito compactar
				list_add(listaEspera, crearNodoEspera(idProc, cantPagProceso));
				compactarSwap(cantPagProceso);
			} else {
				//No hay elementos libres, no puedo alojar.. Rechazo
				nodoRespuesta->exito = 0;
				perror("no hay espacio para el proceso");
				log_info(archivoLog, "No hay espacio para alojar el proceso PID: %d", idProc);
			}
		}
	} else {
		//Esta fragmentando pongo en Espera
		list_add(listaEspera, crearNodoEspera(idProc, cantPagProceso));
	}
	if (hayFragmentacion == 0){
		enviarMensajeRtaAMem(nodoRespuesta);
	}
}

void atenderProcesosEnEspera(){
	sem_wait(&mutexAtendiendoEspera);

	int i;
	int cantidad = listaEspera->elements_count;
	for (i = 1; i == cantidad; i++){
		t_nodoEspera * nodoEspera = NULL;
		nodoEspera = list_get(listaEspera, i);
		recibirProceso(nodoEspera->idProc,nodoEspera->tamanio);
		list_remove(listaEspera, i);
		free(nodoEspera);
	}
	sem_post(&mutexAtendiendoEspera);

}

void eliminarProceso(int pid)
{
	bool condicionProcAEliminar(t_nodoProceso * nodoProceso) {
		return (nodoProceso->idProc == pid);
	}

	t_nodoProceso * nodoProceso = NULL;
	nodoProceso = list_find(listaProcesos,(void*) condicionProcAEliminar);
	if (nodoProceso != NULL){
		t_metricas * metricas = malloc(sizeof(t_metricas));
		metricas = buscarMetricas(pid);
		log_info(archivoLog, "PID: %d, Cantidad de paginas leidas: %d \n", pid, metricas->paginasLeidas);
		log_info(archivoLog, "PID: %d, Cantidad de paginas escritas: %d \n", pid, metricas->paginasEscritas);
		//encontro el nodo a eliminar
		list_add(listaLibres, crearNodoLibre(nodoProceso->indice, nodoProceso->tamanio));
		detectarHuecosContiguos(nodoProceso->indice, nodoProceso->tamanio);
		log_info(archivoLog, "Proceso eliminado PID: %d, Indice: %d, Tamanio: %d \n", pid, nodoProceso->indice, nodoProceso->tamanio);
		list_remove_by_condition(listaProcesos,(void*) condicionProcAEliminar);
		nodoRespuesta->exito = 1;
		free(nodoProceso);
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

int fragmentacionExterna(int tam){
	int i;
	int resp = 0;
	int tamanio = 0;
	t_nodoLibre * nodoLibre = NULL;

	for (i = 1; i == listaLibres->elements_count; i++){
		nodoLibre = list_get(listaLibres, i);
		tamanio = tamanio + nodoLibre->tamanio;

	}
	if (tamanio >= tam){
		resp = 1;
	}
	return resp;
}

void compactarSwap(int cantPagProceso){

	pthread_t thread1;
	char *m1 = "compactar";
	int r1;
	r1 = pthread_create(&thread1, NULL, compactacion, (void*) m1);
}

void* compactacion(){

	int i;
	int elementos = listaLibres->elements_count;
	for (i = 1; i < elementos; i++){
		desplazarYcompactar(i);
	}
	usleep(retardoCompactacion * 1000000);
	hayFragmentacion = 0;
	atenderProcesosEnEspera();
}

void desplazarYcompactar(int indice){

	t_nodoLibre * nodoLibre1 = NULL;
	t_nodoProceso * nodoProceso = NULL;
	nodoLibre1 = list_get(listaLibres,indice);
	int tamCalc;
	tamCalc = nodoLibre1->indice + nodoLibre1->tamanio;
	bool condicionLibre(t_nodoLibre * nodoLibre) {

		return (nodoLibre->indice >= tamCalc);

	}
	nodoProceso = list_find(listaProcesos,(void*) condicionLibre);
	nodoProceso->indice = nodoLibre1->indice;
	nodoLibre1->indice = tamCalc;
	nodoLibre1->tamanio = nodoProceso->tamanio - nodoLibre1->tamanio;
	t_nodoLibre * nodoLibre2 = NULL;
	nodoLibre2 = list_find(listaLibres,(void*) condicionLibre);
	if (nodoLibre2 != NULL){
		nodoLibre2->indice = nodoLibre1->indice;
		nodoLibre2->tamanio = nodoLibre1->tamanio + nodoLibre2->tamanio;
		list_remove(listaLibres, indice);
		free(nodoLibre1);
		//Modifica el indice del elemento encontrado

	} else {
		tamCalc = nodoLibre1->indice + nodoLibre1->tamanio;
		//se llama de vuelta
		desplazarYcompactar(indice);
	}

}

void detectarHuecosContiguos(int indice, int tamanio){

	int valor;
	bool condicionNodoAEliminar(t_nodoLibre * nodoLibre) {
		return (nodoLibre->indice == valor);
	}

	bool condicionBusqueda(t_nodoLibre * nodoLibre) {

		return (nodoLibre->indice == valor);
	}

	t_nodoLibre * nodoLibre1 = NULL;
	t_nodoLibre * nodoLibre2 = NULL;
	valor = indice;
	nodoLibre1 = list_find(listaLibres,(void*) condicionBusqueda);
	valor = indice + tamanio;
	nodoLibre2 = list_find(listaLibres,(void*) condicionBusqueda);
	if (nodoLibre1 != NULL && nodoLibre2 != NULL){
		nodoLibre1->tamanio = nodoLibre1->tamanio + nodoLibre2->tamanio;
		valor = nodoLibre2->indice;
		list_remove_by_condition(listaLibres,(void*) condicionNodoAEliminar);
		free(nodoLibre2);
	}

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
		t_metricas * metricas;
		metricas = buscarMetricas(idProc);
		metricas->paginasLeidas++;
		indiceProceso = nodoProceso->indice;
		int ubicacion = indiceProceso + (tamanioPaginas*pagina);
		//modificar para el directorio real
		//TODO cuidado con las direciones relativas
		particion=fopen("swap.data","r");

		if (particion==NULL){
			perror("No se pudo leer");
		} else {

			fseek(particion, ubicacion, SEEK_SET);
			//leer esa posicion como una lectura1 normal donde el tamaño total a leer es desde paginaReal hasta tamanioPagina (es el tamanio entero de la pag)
			if (fread(leerDelArchivo, tamanioPaginas, 1, particion) > 0){
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

	int ubicacion = nodoProceso->indice + (tamanioPaginas*pagina);
	char * tamTexto = malloc(sizeof("")+1);
	int err;
	if ((err = fseek(particion, ubicacion, SEEK_SET) == 0))
	{//se ubica bien
		//fwrite((const char *)texto, strlen((const char *)texto), 1, particion);
		strncpy(tamTexto,texto,tamanioPaginas);
		strcat(tamTexto,"\0");
		fputs((const char *)tamTexto, particion);
	} else {
		//no se puede ubicar en esa posicion
	}

    fclose(particion);
    //enviarMensajeRtaAMem(nodoRespuesta);
	t_metricas * metricas;
	metricas = buscarMetricas(idProc);
	metricas->paginasEscritas++;
    log_info(archivoLog, "Escritura en el SWAP: ubicacion %d, valor %s del process ID %d, de la pagina %d.", ubicacion, tamTexto, idProc, pagina);

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
	nodo->contenido = malloc(sizeof(tamanioPaginas)+1);
	strcpy(nodo->contenido, contenido);
	strcat(nodo->contenido, "\0");
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

static t_metricas * metricas_create(int pid) {
	t_metricas *new = malloc(sizeof(t_metricas));
    new->idProc = pid;
    new->paginasEscritas = 0;
    new->paginasLeidas = 0;
    return new;
}

t_metricas * buscarMetricas(int processID){

	int devolverTablaMetricas(t_metricas * nodo){
		return (nodo->idProc == processID);
	}

	t_metricas * nodoMetricas = NULL;
	nodoMetricas = list_find(listaMetricas, (void *) devolverTablaMetricas);

	return nodoMetricas;
}
