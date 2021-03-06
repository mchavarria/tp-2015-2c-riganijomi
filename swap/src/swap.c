#include "swap.h"

int main() {

	archivoLog = log_create("swap.log", "Swap", false, 2);
	pthread_mutex_init(&ordenAtencion, NULL);
	pthread_mutex_init(&mutexListaEspera,NULL);
	//Creación de listas
	listaLibres = list_create();
	listaProcesos = list_create();
	listaEspera = list_create();
	listaMetricas = list_create();
	//carga Cfgs
	levantarCfgInicial();
	t_nodo_mem_swap * nodoMemSwap = malloc(sizeof(t_nodo_mem_swap));
	nodoMemSwap->contenido = string_new();
	//creacion archivo particion
	crearParticion();
	for(;(socketMemoria > 0);){
		recibirNodoDeMem(nodoMemSwap);
		pthread_mutex_lock(&ordenAtencion);
		estructuraRecibida(nodoMemSwap);
		pthread_mutex_unlock(&ordenAtencion);
	}
	return 1;
}

void estructuraRecibida(t_nodo_mem_swap * nodoMemSwap){
	if (hayFragmentacion == 0) {
		//opero tranquilo
		nodoRespuesta = malloc(sizeof(t_resp_swap_mem));
		nodoRespuesta->contenido = string_new();
		switch (nodoMemSwap->tipo) {
			case INICIAR:
				nodoRespuesta->tipo = INICIAR;
				recibirProceso(nodoMemSwap);
				break;
			case LEER:
				nodoRespuesta->tipo = LEER;
				leerPaginaProceso(nodoMemSwap);
				usleep(retardoSwap * 1000000);
				break;
			case ESCRIBIR:
				escribirPagina(nodoMemSwap);
				usleep(retardoSwap * 1000000);
				break;
			case FINALIZAR:
				nodoRespuesta->tipo = FINALIZAR;
				eliminarProceso(nodoMemSwap->pid);
				break;
		}
	} else {
		printf("PID %d: Solicitud %s agregada en Lista de Espera.\n",nodoMemSwap->pid,traducirTipoInstruccion(nodoMemSwap->tipo));
		log_info(archivoLog,"PID %d: Solicitud agregada en Lista de Esepra.\n",nodoMemSwap->pid);
		//Tengo que agregar a la lista de pedidos
		pthread_mutex_lock(&mutexListaEspera);
		agregarAListaDeEspera(nodoMemSwap);
		pthread_mutex_unlock(&mutexListaEspera);
		if (nodoMemSwap->tipo != FINALIZAR){
			//Debo avisar que está en compactacion
			nodoRespuesta->tipo = COMPACTACION;
			nodoRespuesta->pagina = retardoCompactacion;
			enviarMensajeRtaAMem(nodoRespuesta);
		}
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

static t_nodoProceso *crearNodoProceso(int idProc, int indice, int cantPagProceso) {
	t_nodoProceso *nodoProceso = malloc(sizeof(t_nodoProceso));
	nodoProceso->idProc = idProc;
	nodoProceso->tamanio = cantPagProceso;
	nodoProceso->indice = indice;
    return nodoProceso;
}

void recibirProceso(t_nodo_mem_swap * nodoMemSwap)
{
	int idProc = nodoMemSwap->pid;
	int cantPagProceso = nodoMemSwap->pagina;
	int respuestaAgregar;
	//Salvo en la auxiliar para poder usarla en condicion
	if (hayFragmentacion == 0){
		respuestaAgregar = agregarProcesoEnSwap(idProc,cantPagProceso);
		if (respuestaAgregar == 1){
			nodoRespuesta->exito = 1;
		} else {
			hayFragmentacion = fragmentacionExterna(cantPagProceso);
			if (hayFragmentacion == 1){
				printf("PID %d: Fallo iniciar en Swap por fragmentacion Externa.\n", idProc);
				//necesito compactar
				printf("PID %d: Solicitud %s agregada en Lista de Esepra.\n",nodoMemSwap->pid,traducirTipoInstruccion(nodoMemSwap->tipo));
				log_info(archivoLog,"PID %d: Solicitud agregada en Lista de Esepra.\n",nodoMemSwap->pid);
				pthread_mutex_lock(&mutexListaEspera);
				agregarAListaDeEspera(nodoMemSwap);
				pthread_mutex_unlock(&mutexListaEspera);

				log_info(archivoLog, "Se COMPACTA para recibir el proceso PID: %d", idProc);
				pthread_t thread1;
				char *m1 = "compactar";
				int r1;
				r1 = pthread_create(&thread1, NULL, compactacion, (void*) m1);
				/*printf("PID %d: Intentando iniciar nuevamente.\n", idProc);
				respuestaAgregar = agregarProcesoEnSwap(idProc,cantPagProceso);*/
			}
		}
	}

	if (respuestaAgregar == 0 && hayFragmentacion == 0) {
		//No hay elementos libres, no puedo alojar.. Rechazo
		nodoRespuesta->exito = 0;
		printf("PID %d: No hay espacio para el proceso.\n", idProc);
		log_info(archivoLog, "PID %d: No hay espacio para el proceso.", idProc);
	}
	if (hayFragmentacion == 1){
		nodoRespuesta->tipo = COMPACTACION;
		nodoRespuesta->pagina = retardoCompactacion;
	}

	enviarMensajeRtaAMem(nodoRespuesta);
}

void agregarAListaDeEspera(t_nodo_mem_swap * nodoMemSwap){
	t_nodo_mem_swap * nodo = malloc(sizeof(t_nodo_mem_swap));
	nodo->contenido = string_new();
	nodo->pagina = nodoMemSwap->pagina;
	nodo->pid = nodoMemSwap->pid;
	nodo->tipo = nodoMemSwap->tipo;
	strcpy(nodo->contenido,nodoMemSwap->contenido);
	list_add(listaEspera, nodo);
}

int agregarProcesoEnSwap(int idProc, int cantPagProceso){

	int respuesta = 0;
	bool condicionRecibir(t_nodoLibre * nodoLibre) {
		return (nodoLibre->tamanio >= cantPagProceso);
	}
	t_nodoLibre * nodoLibre = NULL;
	nodoLibre = list_find(listaLibres,(void*) condicionRecibir);
	if (nodoLibre != NULL){
		list_add(listaMetricas, metricas_create(idProc));
		//hay lugar para alojarlo
		list_add(listaProcesos, crearNodoProceso(idProc, nodoLibre->indice, cantPagProceso));
		log_info(archivoLog, "PID %d: Total Paginas %d. Recibido y alojado en Indice %d", idProc,cantPagProceso , nodoLibre->indice);
		puts("----------------------");
		printf("PID %d: Total Paginas %d. Recibido y alojado en Indice %d\n", idProc,cantPagProceso , nodoLibre->indice);
		puts("----------------------");
		nodoLibre->tamanio = nodoLibre->tamanio - cantPagProceso;
		int indiceRecienOcupado (t_nodoLibre * nodo) {
			return (nodo->indice == nodoLibre->indice);
		}
		if (nodoLibre->tamanio == 0) {
			list_remove_by_condition(listaLibres, (void *) indiceRecienOcupado);
		} else {
			nodoLibre->indice = nodoLibre->indice + cantPagProceso;
		}
		respuesta = 1;
	}
	return respuesta;
}
/*
 * Al no responderle a memoria por estar fragmentando
 * todavia no llegan nuevos pedidos
 * hasta que termine
void atenderProcesosEnEspera(){
	sem_wait(&mutexAtendiendoEspera);

	int i;
	int cantidad = listaEspera->elements_count;
	for (i = 0; i < cantidad; i++){
		t_nodoEspera * nodoEspera = NULL;
		nodoEspera = list_get(listaEspera, 0);
		recibirProceso(nodoEspera->idProc,nodoEspera->tamanio);
		list_remove(listaEspera, 0);
		//free(nodoEspera);
	}
	sem_post(&mutexAtendiendoEspera);

}
*/
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
		log_info(archivoLog, "PID %d: Cantidad de paginas leidas: %d.", pid, metricas->paginasLeidas);
		log_info(archivoLog, "PID %d: Cantidad de paginas escritas: %d.", pid, metricas->paginasEscritas);
		//encontro el nodo a eliminar
		limpiarPaginas(nodoProceso);
		list_add(listaLibres, crearNodoLibre(nodoProceso->indice, nodoProceso->tamanio));
		detectarHuecosContiguos(nodoProceso->indice, nodoProceso->tamanio);
		log_info(archivoLog, "PID %d: Eliminado de Swap.", pid);
		puts("----------------------");
		printf("PID %d: Eliminado de Swap.\n", pid);
		puts("----------------------");
		list_remove_by_condition(listaProcesos,(void*) condicionProcAEliminar);
		nodoRespuesta->exito = 1;
		//free(nodoProceso);
	} else {
		//no encontro el proceso indicado
		nodoRespuesta->exito = 0;
		log_info(archivoLog, "PID %d: No se pudo eliminar de Swap.", pid);
	}
	//imprimirEstadosListas();

	//enviarMensajeRtaAMem(nodoRespuesta);
}

void imprimirEstadosListas(){
	printf("Libres: %d elementos.\n",listaLibres->elements_count);
	int totalTamanio=0;
	void imprimirLibres(t_nodoLibre * nodo)
	{
		printf("Indice %d, paginas libres: %d\n", nodo->indice,nodo->tamanio);
		totalTamanio += nodo->tamanio;
	}
	list_iterate(listaLibres,(void *) imprimirLibres);
	printf("Procesos: %d activos.\n",listaProcesos->elements_count);
	totalTamanio = 0;
	void imprimirNodos(t_nodoProceso * nodo)
	{
		printf("PID: %d, indice %d, cantidad de paginas: %d\n", nodo->idProc, nodo->indice,nodo->tamanio);
		totalTamanio += nodo->tamanio;
	}
	list_iterate(listaProcesos,(void *) imprimirNodos);
}

void limpiarPaginas(t_nodoProceso * nodoProceso){
	int i;
	int ubicacion = (nodoProceso->indice * tamanioPaginas);
	for (i = 0; i < nodoProceso->tamanio; i++){
		//Elimina el contenido de donde estaba antes el proceso.
		FILE * p3=fopen("swap.data","r+");
		int j=0;
		for (j= 0; j < tamanioPaginas; j++){
			fseek(p3, ubicacion+j, SEEK_SET);
			putc('\0', p3);
		}
		fclose(p3);
	}
}

int fragmentacionExterna(int tam){
	int i;
	int resp = 0;
	int tamanio = 0;
	t_nodoLibre * nodoLibre = NULL;

	for (i = 0; i < listaLibres->elements_count; i++){
		nodoLibre = list_get(listaLibres, i);
		tamanio = tamanio + nodoLibre->tamanio;

	}
	if (tamanio >= tam){
		resp = 1;
	}
	return resp;
}

void* compactacion(){

	//aviso a memoria que espere
	int i;
	puts("Compactando particion");
	usleep(retardoCompactacion * 1000000);
	int elementos = listaLibres->elements_count;
	for (i = 0; i < elementos; i++){
		desplazarYcompactar(0);
	}
	listaLibres = list_create();
	int indiceLibre = 0;
	int tamanio = 0;
	void libre(t_nodoProceso * nodo) {
		if (indiceLibre < nodo->indice) {
			indiceLibre = nodo->indice + nodo->tamanio;
		}
		tamanio += nodo->tamanio;
	}
	list_iterate(listaProcesos, (void *)libre);
	list_add(listaLibres, crearNodoLibre(indiceLibre, (cantPaginas - tamanio)));
	puts("Compactacion finalizada");
	//imprimirEstadosListas();
	//Atiendo los nodos en espera y despues libero.
	pthread_mutex_lock(&ordenAtencion);
	hayFragmentacion = 0;
	atenderProcesosEnEspera();
	pthread_mutex_unlock(&ordenAtencion);

}

void atenderProcesosEnEspera(){
	int i;
	int cant = listaEspera->elements_count;
	printf("Atendiendo %d solicitudes en espera.\n",cant);
	for (i = 0; i < cant; i++){
		pthread_mutex_lock(&mutexListaEspera);
		t_nodo_mem_swap * nodo = list_get(listaEspera,i);
		estructuraRecibida(nodo);
		pthread_mutex_unlock(&mutexListaEspera);
	}
	listaEspera = list_create();
	puts("Finaliza atencion de solicitudes en espera");
}

void desplazarYcompactar(int indice){

	t_nodoLibre * nodoLibre1 = NULL;
	t_nodoProceso * nodoProceso = NULL;
	nodoLibre1 = list_get(listaLibres,indice);
	int tamCalc;
	tamCalc = nodoLibre1->indice + nodoLibre1->tamanio;
	bool condicionAlLadoDeLibre(t_nodoProceso * nodoP) {

		return (nodoP->indice >= tamCalc);

	}
	nodoProceso = list_find(listaProcesos,(void*) condicionAlLadoDeLibre);

	while (nodoProceso != NULL){
		//DESPLAZAMIENTO CONTENIDO ARCHIVO
			char leerDelArchivo[1024];
			int ubicacion = (nodoProceso->indice * tamanioPaginas);
			int nuevaUbicacion = nodoLibre1->indice * tamanioPaginas;
			int i=0;
			for (i = 0; i < nodoProceso->tamanio; i++){
				//LEER
				FILE * particion=fopen("swap.data","r");
				fseek(particion, ubicacion, SEEK_SET);
				fread(leerDelArchivo, tamanioPaginas, 1, particion);
				fclose(particion);
				//Elimina el contenido de donde estaba antes el proceso.
				FILE * p3=fopen("swap.data","r+");
				int j=0;
				for (j= 0; j < tamanioPaginas; j++){
					fseek(p3, ubicacion+j, SEEK_SET);
					putc('\0', p3);
				}
				nuevaUbicacion = nuevaUbicacion + (tamanioPaginas*i);
				fclose(p3);
				ubicacion = ubicacion + (tamanioPaginas*i);
				//ESCRIBIR
				FILE * p2=fopen("swap.data","r+");
				fseek(p2, nuevaUbicacion, SEEK_SET);
				fputs((const char *)leerDelArchivo, p2);
				//nuevaUbicacion = nuevaUbicacion + (tamanioPaginas*i);
				fclose(p2);

			}
			//DESPLAZAMIENTO CONTENIDO ARCHIVO
			nodoProceso->indice = nodoLibre1->indice;
			nodoLibre1->indice = nodoProceso->indice + nodoProceso->tamanio;
			//reinicia el while
			tamCalc = nodoLibre1->indice + nodoLibre1->tamanio;
			nodoProceso = list_find(listaProcesos,(void*) condicionAlLadoDeLibre);
	}
	t_nodoLibre * nodoLibre2 = NULL;
	bool condicionLibreContiguo(t_nodoLibre * nodoL) {

		return (nodoL->indice >= (nodoLibre1->indice + nodoLibre1->tamanio));

	}
	nodoLibre2 = list_find(listaLibres,(void*) condicionLibreContiguo);
	if (nodoLibre2 != NULL) {
		nodoLibre2->indice = nodoLibre1->indice;
		nodoLibre2->tamanio = nodoLibre1->tamanio + nodoLibre2->tamanio;
		list_remove(listaLibres, indice);
	}
	//free(nodoLibre1);
	//Modifica el indice del elemento encontrado

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
		//free(nodoLibre2);
	}

}

void leerPaginaProceso(t_nodo_mem_swap * nodoMemSwap){

	int idProc = nodoMemSwap->pid;
	int pagina = nodoMemSwap->pagina;
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
		int ubicacion = (indiceProceso * tamanioPaginas) + (tamanioPaginas*pagina);
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
			log_info(archivoLog, "PID %d: Pagina %d leida en indice %d, contenido -%s-.",idProc,pagina, ubicacion,leerDelArchivo);
		}
	}else{

		perror("no se encontró el proceso indicado");
	}
	enviarMensajeRtaAMem(nodoRespuesta);
}


void escribirPagina(t_nodo_mem_swap * nodoMemSwap) {
	int idProc = nodoMemSwap->pid;
	int pagina = nodoMemSwap->pagina;
	char * texto = nodoMemSwap->contenido;
	FILE *particion;

	particion = fopen("swap.data","r+");

	bool condicionLeer(t_nodoProceso * nodoProceso){
		return (nodoProceso->idProc == idProc);
	}
	t_nodoProceso * nodoProceso = NULL;
	nodoProceso = list_find(listaProcesos,(void*) condicionLeer);

	int ubicacion = (nodoProceso->indice * tamanioPaginas) + (tamanioPaginas*pagina);
	//char * tamTexto = malloc(sizeof("")+1);
	int err;
	if ((err = fseek(particion, ubicacion, SEEK_SET) == 0))
	{//se ubica bien
		//fwrite((const char *)texto, strlen((const char *)texto), 1, particion);
		/*
		strncpy(tamTexto,texto,tamanioPaginas);
		strcat(tamTexto,"\0");
		fputs((const char *)tamTexto, particion);
		*/
		fputs((const char *)texto, particion);
	} else {
		//no se puede ubicar en esa posicion
	}

    fclose(particion);
    //enviarMensajeRtaAMem(nodoRespuesta);
	t_metricas * metricas;
	metricas = buscarMetricas(idProc);
	metricas->paginasEscritas++;
    log_info(archivoLog, "PID %d: Pagina %d escrita en indice %d, valor -%s-.",idProc,pagina, ubicacion, texto);

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

char * traducirTipoInstruccion(int tipo){
	char * instruccion = string_new();
	switch (tipo) {
		case INICIAR:
			strcpy(instruccion,"INICIAR");
			break;
		case LEER:
			strcpy(instruccion,"LEER");
			break;
		case ESCRIBIR:
			strcpy(instruccion,"ESCRIBIR");
			break;
		case FINALIZAR:
			strcpy(instruccion,"FINALIZAR");
			break;
	}
	return instruccion;
}
