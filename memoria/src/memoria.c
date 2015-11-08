#include "memoria.h"

int main(int argc, char* argv[]) {
	/*
	pthread_t thread1;
	char *m1 = "";
	int r1;

	r1 = pthread_create(&thread1,NULL, flushTLB, (void *) m1);

	pthread_t thread2;
	char *m2 = "";
	int r2;

	r2 = pthread_create(&thread2,NULL, flushMarcos, (void *) m2);

	signal(SIGINT, flushTLBActivacion);
	signal(SIGUSR1, flushMarcosActivacion);*/
	//signal(SIGUSR2, dumpMemoria);
	sem_init(&mutexRespuestaSwap,0,0);
	listaTablasPaginas = list_create();
	listaMarco = list_create();
	listaTLB = list_create();
	listaDeCPUs = list_create();
	listaSolicitudes = list_create();
	/*
	const char *programa1[9];
	programa1[0] = "iniciar 9";
	programa1[1] = "escribir 7 'nicolas'";
	programa1[2] = "escribir 1 'juan'";
	programa1[3] = "escribir 2 'florencia'";
	programa1[4] = "escribir 3 'pepe'";
	programa1[5] = "escribir 4 'joaquin'";
	programa1[6] = "leer 1";
	programa1[7] = "leer 4";
	programa1[8] = "finalizar";*/


	levantarCfgInicial();
	//ESTRUCTURAS
	inicializarTLB();
	inicializarMarco();

	/*int i=0;
	for (i = 0; i < 10; i++) {
		t_nodo_mem * mem = malloc(sizeof(t_nodo_mem));
		strcpy(mem->instruccion, programa1[i]);
		mem->pid = 1234;
		interpretarLinea(mem);
	}*/


	//sem_init(&sem_mem, 0, 0);
	//sem_init(&sem_sockets, 0, 1);
	//Inicia los parametros

	//Tratamiento de la señan enviada por el SO

	signal(SIGINT, rutina);
	signal(SIGUSR1, rutina);
	signal(SIGUSR2, rutina);

	pthread_t monitor;
	char *m3 = "monitor";
	int r3;

	r3 = pthread_create( &monitor, NULL, monitorearSockets, (void*) m3);
	//r1 = pthread_create(&hiloMonitorSockets,NULL,monitorPrepararServidor(&sem_mem,&sem_sockets), (void *) arg1);

	pthread_t atenderSol;
	char *m2 = "atenderSol";
	int r1;

	r1 = pthread_create( &atenderSol, NULL, atenderSolicitudes, (void*) m2);
	pthread_join(atenderSol,NULL);
	return 1;
}

static t_marco * marco_create(int processID, char * valor, int numeroMarco, int numeroPagina, int bitModificacion) {
	t_marco *new = malloc(sizeof(t_marco));
    new->processID = processID;
    new->numeroMarco = numeroMarco;
    new->numeroPagina = numeroPagina;
    new->bitModificacion = bitModificacion;
    new->valor = malloc(sizeof(valor));
    strcpy(new->valor, valor);
    return new;
}

static t_marco * tlb_create(int processID, int numeroPagina, int marco) {
	t_tlb * new = malloc(sizeof(t_tlb));
    new->processID = processID;
    new->numeroPagina = numeroPagina;
    new->marco = marco;
    return new;
}

void inicializarTLB() {
	int i = 0;
	for (i=0; i<ENTRADAS_TLB; i++) {
		list_add(listaTLB, tlb_create(NULO,0,0));
	}
}


void inicializarMarco() {
	uint32_t i = 0;
	//o * nodoMarco;
	//listaMarco = malloc(sizeof(t_marco) * CANTIDAD_MARCOS);
	//nodoMarco = malloc(sizeof(t_marco));
	int numeroMarco;
	int processID;
	char * valor;
	int presencia;
	int numeroPagina;
	int bitModificacion;
	for(i=0; i<CANTIDAD_MARCOS; i++) {
		numeroMarco = i;
		processID = 0;
		valor = malloc(sizeof("NULL"));
		strcpy(valor, "NULL");
		strcat(valor, "\0");
		presencia = 0;
		numeroPagina = 0;
		bitModificacion = 0;
		list_add(listaMarco, marco_create(NULO, valor, numeroMarco, numeroPagina, bitModificacion));
	}
	/*int * array = malloc(CANTIDAD_MARCOS * TAMANIO_MARCO);
	memoria = &array[0];
	for(; i<CANTIDAD_MARCOS; i++) {
		*(memoria+(TAMANIO_MARCO* i)) = malloc(TAMANIO_MARCO);
		strcpy(*(memoria+(TAMANIO_MARCO * i)), "NULL");
		strcat(*(memoria+(TAMANIO_MARCO * i)), "\0");
		printf("%s\n, ", memoria[TAMANIO_MARCO * i]);
	}*/
}


void levantarCfgInicial()
{
	char cfgFin[] ="/memoria/src/config.cfg";//Consola
	//char cfgFin[] ="/src/config.cfg";

	char *dir = getcwd(NULL, 0);

	char *directorioActual = malloc(strlen(dir)+strlen(cfgFin)+1);

	strcpy(directorioActual,dir);
	strcat(directorioActual,cfgFin);

	int resultado = 1;

	puts(directorioActual);

	archConfig = malloc(sizeof(t_config));
	archivoLog = log_create("memoria.log", "Memoria", false, 2);//Eclipse
	archConfig = config_create(directorioActual);

	int largo = strlen(config_get_string_value(archConfig, "PUERTO_SWAP"));
	PUERTO_SWAP=malloc(largo + 1);
	memset(PUERTO_SWAP,'\0',largo + 1);
	PUERTO_SWAP=config_get_string_value(archConfig,"PUERTO_SWAP");

	largo = strlen(config_get_string_value(archConfig, "IP_SWAP"));
	IP_SWAP=malloc(largo + 1);
	memset(IP_SWAP,'\0',largo + 1);
	IP_SWAP = config_get_string_value(archConfig, "IP_SWAP");

	largo = strlen(config_get_string_value(archConfig, "PUERTO_ESCUCHA"));
	PUERTO_ESCUCHA=malloc(largo + 1);
	memset(PUERTO_ESCUCHA,'\0',largo + 1);
	PUERTO_ESCUCHA=config_get_string_value(archConfig,"PUERTO_ESCUCHA");

	RETARDO_MEMORIA =config_get_int_value(archConfig,"RETARDO_MEMORIA");

	CANTIDAD_MARCOS = config_get_long_value(archConfig,"CANTIDAD_MARCOS");

	TAMANIO_MARCO = config_get_long_value(archConfig,"TAMANIO_MARCO");

	MAXIMO_MARCOS_POR_PROCESO = config_get_long_value(archConfig,"MAXIMO_MARCOS_POR_PROCESO");

	ENTRADAS_TLB = config_get_long_value(archConfig,"ENTRADAS_TLB");
	strcpy(TLB_HABILITADA,config_get_string_value(archConfig,"TLB_HABILITADA"));
	strcpy(ALGORITMO_REEMPLAZO,config_get_string_value(archConfig,"ALGORITMO_REEMPLAZO"));

	free(directorioActual);
}


void rutina (int n) {
	switch (n) {
		case SIGINT:
			printf("En tu cara, no salgo nada…\n");
			log_info(archivoLog,"ingreso el mensaje rutina SIGINT: %d",SIGINT);
		break;
		case SIGUSR1:
			printf("LLEGO SIGUSR1\n");
			log_info(archivoLog,"ingreso el mensaje rutina SIGUSR1: %d",SIGUSR1);
		break;
		case SIGUSR2:
			printf("LLEGO SIGUSR2\n");
			log_info(archivoLog,"ingreso el mensaje rutina SIGUSR2: %d",SIGUSR2);
		break;
	}
}

static t_marcoLibre * marcoLibre_create(int numeroMarco) {
	t_marcoLibre * new = malloc(sizeof(t_marcoLibre));

	new->numeroMarco = numeroMarco;

	return new;
}

static t_tablaPaginasProceso * nodoTablaPaginaProceso_create(int numeroMarco, int numeroPagina, int ingreso) {
	t_tablaPaginasProceso * new = malloc(sizeof(t_tablaPaginasProceso));

	new->numeroMarco = numeroMarco;
	new->numeroPagina = numeroPagina;
	new->ingreso = ingreso;

	return new;
}

int inicializarTablaDePaginas(int cantidadPaginas, int pid) {
	t_tablasPaginas * nodoTablaPaginas = malloc(sizeof(t_tablasPaginas));
	nodoTablaPaginas->processID = pid;
	nodoTablaPaginas->listaPaginas = list_create();
	int i = 0;
	t_tablaPaginasProceso * nodoPaginasProceso;
	for (i=0; i<cantidadPaginas; i++) {
		nodoPaginasProceso = malloc(sizeof(t_tablaPaginasProceso));
		list_add(nodoTablaPaginas->listaPaginas, nodoTablaPaginaProceso_create(NULO, i, NULO));
		free(nodoPaginasProceso);
	}
	list_add(listaTablasPaginas, nodoTablaPaginas);
}

void assert_valorTLB(t_tlb * nodoTLB, int processID, int numeroPagina, int fueModificado, int marco) {
	processID = nodoTLB->processID;
	numeroPagina = nodoTLB->numeroPagina;
	marco = nodoTLB->marco;
}

int buscarEnTLB(int processID, int numeroPagina) {
	int devolverValorDeTLB(t_tlb * nodo) {
		return (nodo->processID == processID && nodo->numeroPagina == numeroPagina);
	}

	t_tlb * nodoTLB = malloc(sizeof(t_tlb));

	nodoTLB = NULL;

	nodoTLB = list_find(listaTLB, (void *) devolverValorDeTLB);

	if (nodoTLB != NULL) {
		return nodoTLB->marco;
	}
	return NULO;
}

void assert_valor(t_marco * nodoMarco, int processID, char * valor) {
	processID = nodoMarco->processID;
	valor = nodoMarco->valor;
}

void assert_valorTablaPag(t_tablasPaginas * nodoTablasPagina, t_list * listaPaginas) {
	listaPaginas = nodoTablasPagina->listaPaginas;
}

void assert_valorPagina(t_tablaPaginasProceso * nodoTablasPaginaProceso, int marco) {
	marco = nodoTablasPaginaProceso->numeroMarco;
}

static t_tablaPaginasProceso * obtenerPagina(int numeroPagina, t_tablasPaginas * nodoTablasPagina) {

	int devolverPaginasProceso(t_tablaPaginasProceso * nodo){
		return (nodo->numeroPagina == numeroPagina);
	}

	t_tablaPaginasProceso * nodoTablasPaginaProceso = NULL;
	nodoTablasPaginaProceso = list_find(nodoTablasPagina->listaPaginas, (void *) devolverPaginasProceso);

	return nodoTablasPaginaProceso;
}

static t_tablaPaginasProceso * obtenerPaginaPorNumMarco(int marco, t_tablasPaginas * nodoTablasPagina) {

	int devolverPaginasProceso(t_tablaPaginasProceso * nodo){
		return (nodo->numeroMarco == marco);
	}

	t_tablaPaginasProceso * nodoTablasPaginaProceso = NULL;
	nodoTablasPaginaProceso = list_find(nodoTablasPagina->listaPaginas, (void *) devolverPaginasProceso);

	return nodoTablasPaginaProceso;
}

static t_tablasPaginas * buscarTablaPaginas(int processID){

	int devolverTablaProceso(t_tablasPaginas * nodo){
		return (nodo->processID == processID);
	}

	t_tablasPaginas * nodoTablaDeProceso = NULL;
	nodoTablaDeProceso = list_find(listaTablasPaginas, (void *) devolverTablaProceso);

	return nodoTablaDeProceso;
}

int buscarEnMarcos(int processID, int numeroPagina) {
	t_marco * marco = malloc(sizeof(t_marco));


	int devolverValorMarco(t_marco * nodo) {
		return ((nodo->numeroPagina == numeroPagina) && (nodo->processID == processID));
	}

	marco = list_find(listaMarco, (void *) devolverValorMarco);

	return marco->valor;

}

int cantidadMarcosAsignados(int processID) {

	t_tablasPaginas * tablaDeProceso = malloc(sizeof(t_tablasPaginas));

	tablaDeProceso = buscarTablaPaginas(processID);

	int contarMarcosAsignados(t_tablaPaginasProceso * nodo) {
		return (nodo->numeroMarco != NULO);
	}
	int valor = list_count_satisfying(tablaDeProceso->listaPaginas, (void*) contarMarcosAsignados);
	return valor;
}

int obtenerUnMarcoLibre(t_list * lista) {

	int marcoLibre(t_marco * nodo) {
		return (nodo->processID == NULO);
	}
	t_marco * nodoMarco = NULL;
	nodoMarco = (list_find(listaMarco, (void*) marcoLibre));

	return (nodoMarco->numeroMarco);

}


//Se usar para LEER u ESCRIBIR, busca un marco en TLB, luego lo busca en TABLA DE PAGINAS.
//Si es necesario envia/pide a swap.
static t_marco * detectarPageFault(t_nodo_mem * nodoInstruccion, int numeroPagina) {
	int resultadoBusqueda;
	bool devolverValor(t_marco * nodo) {
		return (nodo->numeroMarco == resultadoBusqueda);
	}
	t_tablasPaginas * tablaDeProceso;
	t_tablaPaginasProceso * nodoPagina;
	int indiceMarco;
	t_marco * nodoMarco = malloc(sizeof(t_marco));
	nodoMarco = NULL;
	resultadoBusqueda = buscarEnTLB(nodoInstruccion->pid, numeroPagina);
	if (resultadoBusqueda == NULO) {
		//numeroPagina = devolverParteUsableInt(nodoInstruccion->instruccion, 5);
		tablaDeProceso = buscarTablaPaginas(nodoInstruccion->pid);
		nodoPagina = obtenerPagina(numeroPagina, tablaDeProceso);
		resultadoBusqueda = nodoPagina->numeroMarco;
		if (resultadoBusqueda == NULO)
		{
			//PAGE FAULT
		}//Encontrado en la tabla de paginas del proceso
	}//Encontrado en TLB
	nodoMarco = list_find(listaMarco, (void*) devolverValor);
	return nodoMarco;
}

int algoritmoReemplazo(int processID){
	int marco;
	if (string_equals_ignore_case(ALGORITMO_REEMPLAZO, "FIFO")) {
		marco = algoritmoReemplazoFIFO(processID);
	}

	return marco;
}

int algoritmoReemplazoFIFO(int processID) {
	bool ordenarParaFIFO(t_tablaPaginasProceso * nodo1, t_tablaPaginasProceso * nodo2) {
		return (nodo1->ingreso < nodo2->ingreso);
	}
	t_tablasPaginas * tablaDeProceso = NULL;

	tablaDeProceso = buscarTablaPaginas(processID);
	list_sort(tablaDeProceso->listaPaginas, (void*) ordenarParaFIFO);

	t_tablaPaginasProceso * nodoPagina = list_get(tablaDeProceso->listaPaginas, 0);

	int numMarcoPa = nodoPagina->numeroMarco;
	//actualizarNodoPaginas(NULO, processID, nodoPagina->numeroPagina);

	int conMarcosAsignados(t_tablaPaginasProceso * nodo) {
		return (nodo->ingreso != NULO);
	}
	t_list * listaPaginasAlgoritmo = list_filter(tablaDeProceso->listaPaginas, (void*) conMarcosAsignados);
	int i;
	for (i=1; i<listaPaginasAlgoritmo->elements_count; i++) {
		t_tablaPaginasProceso * nodoPagina2 = list_get(listaPaginasAlgoritmo, i);
		nodoPagina2->ingreso--;
	}
	actualizarNodoPaginas(NULO, processID, nodoPagina->numeroPagina);

	return numMarcoPa;
}

void actualizarMarco(char * texto,int pid, int numeroPagina, char * paginaSwap, int indiceMarco, int tipo)
{
		escribirMarco(pid, indiceMarco, texto, numeroPagina,tipo);
		actualizarNodoPaginas(indiceMarco, pid, numeroPagina);
}

void desasignarMarco(int processID, int marco) {
	t_tablasPaginas * tablaDeProceso;
	t_tablaPaginasProceso * nodoPagina;

	tablaDeProceso = buscarTablaPaginas(processID);

	nodoPagina = obtenerPaginaPorNumMarco(marco, tablaDeProceso);

	nodoPagina->ingreso = NULO;
	nodoPagina->numeroMarco = NULO;
}

void actualizarNodoPaginas(int indiceMarco, int processID, int numeroPagina) {
	t_tablasPaginas * tablaDeProceso;
	t_tablaPaginasProceso * nodoPagina;

	tablaDeProceso = buscarTablaPaginas(processID);

	nodoPagina = obtenerPagina(numeroPagina, tablaDeProceso);

	int contarMarcosAsignados(t_tablaPaginasProceso * nodo) {
			return (nodo->numeroMarco != NULO);
		}
	int valor = list_count_satisfying(tablaDeProceso->listaPaginas, (void*) contarMarcosAsignados);

	nodoPagina->ingreso = valor;
	nodoPagina->numeroMarco = indiceMarco;
}

int interpretarLinea() {

    t_marco * marco = malloc(sizeof(t_marco));
    marco = NULL;
    //Envio a SWAP
    nodoASwap = malloc(sizeof(t_nodo_mem_swap));
	nodoASwap->pid = pid;
	nodoASwap->contenido = malloc(strlen("")+1);
	strcpy(nodoASwap->contenido,"\0");

	//Respuesta a CPU
	nodoRtaSwap = malloc(sizeof(t_resp_swap_mem));

    if (esElComando(linea, "iniciar"))
    {
    	int cantidadPaginasProceso;
		cantidadPaginasProceso = devolverParteUsableInt(linea, 8);

    	nodoASwap->tipo = INICIAR;
    	nodoRtaSwap->tipo = INICIAR;
    	nodoASwap->pagina = cantidadPaginasProceso;
    	enviarMensajeDeNodoASWAP(nodoASwap);
		recibirNodoDeRtaSwap(nodoRtaSwap);
		if (nodoRtaSwap->exito){
			inicializarTablaDePaginas(cantidadPaginasProceso, pid);
		}
    	enviarMensajeDeNodoACPU(nodoRtaSwap);
		log_info(archivoLog, "Proceso ID %d, cantidad de paginas %d. Inicio: %d", pid, cantidadPaginasProceso,nodoRtaSwap->exito);
	} else if (esElComando(linea, "finalizar")) {
		nodoASwap->tipo = FINALIZAR;
		nodoRtaSwap->tipo = FINALIZAR;
		bool procesoActual(t_tablasPaginas * nodo) {
			return (nodo->processID == pid);
		}
		list_remove_by_condition(listaTablasPaginas, (void *) procesoActual);
		nodoASwap->pagina = 0;
		enviarMensajeDeNodoASWAP(nodoASwap);
		log_info(archivoLog, "Se ha finalizado el proceso con ID: %d.", pid);
		puts("finalizado");
	} else {
		int pagina = obtenerPaginaLeeroEscribir(linea);
		marco = detectarPageFault(nodoInstruccion,pagina);
		if (marco == NULL)
		{//Hay page fault
			nodoASwap->tipo = LEER;
			nodoASwap->pagina = pagina;
			enviarMensajeDeNodoASWAP(nodoASwap);
			sem_wait(&mutexRespuestaSwap);
			tamanioSwapMjeGlobal = recibirNodoDeRtaSwap(nodoRtaSwap);
			seleccionarMarcoVictima(marco);
			marco->valor = nodoRtaSwap->contenido;
			marco->bitLeido = 1;
			marco->numeroPagina = pagina;
			marco->processID = nodoInstruccion->pid;
		}
		if (esElComando(linea, "leer"))
		{
				nodoRtaSwap->tipo = LEER;
				cargarTlb(nodoInstruccion, marco);
				strcpy(nodoRtaSwap->contenido,marco->valor);
				log_info(archivoLog, "Se ha leido una pagina: process ID %d, pagina leida %d con el valor '%s'", pid, pagina, marco->valor);
				enviarMensajeDeNodoACPU(nodoRtaSwap);
				sleep(RETARDO_MEMORIA);
		} else if (esElComando(linea, "escribir")) {
			nodoRtaSwap->tipo = ESCRIBIR;
			nodoRtaSwap->exito = 1;
			char * texto;
			texto = malloc(strlen(devolverParteUsable(linea, posicionComilla(linea))));
			strcpy(texto, devolverParteUsable(linea, posicionComilla(linea)));
			strtok(texto, "\"");
			char ** textoSinComillas = string_split(texto, "\"");
			strcat(texto, "\0");
			strcpy(marco->valor,texto);
			cargarTlb(nodoInstruccion, marco);
			log_info(archivoLog, "Se ha escrito una pagina: process ID %d, pagina escrita %d con el valor '%s'", pid, pagina, texto);
			enviarMensajeDeNodoACPU(nodoRtaSwap);
			sleep(RETARDO_MEMORIA);
		}
	}
    free(nodoRtaSwap);
    free(nodoASwap);
    free(marco);

    return 1;
}

int obtenerPaginaLeeroEscribir(char * linea)
{
	int pagina;
	if (esElComando(linea, "leer")){
		pagina = devolverParteUsableInt(linea, 5);
	} else {
		pagina = valorPagina(linea);
	}
	return pagina;
}

void seleccionarMarcoVictima(t_marco * marco)
{
	int indiceMarco;
	if (cantidadMarcosAsignados(nodoInstruccion->pid) < MAXIMO_MARCOS_POR_PROCESO)
	{
		indiceMarco = obtenerUnMarcoLibre(listaMarco);
	} else if (cantidadMarcosAsignados(nodoInstruccion->pid) == MAXIMO_MARCOS_POR_PROCESO) {
		//ejecuta algoritmo de reemplazo FIFO.
		//elimina numero de marco en la pagina victima
		int marco = algoritmoReemplazo(nodoInstruccion->pid);
		desasignarMarco(nodoInstruccion->pid,marco);
		//TODO guardar en swap en caso de estar modificado
	}
	if (indiceMarco < 0) {
		//finaliza proceso.
		/*nodoASwap->tipo = FINALIZAR;
		nodoASwap->pid = nodoInstruccion->pid;
		enviarMensajeDeNodoASWAP(nodoASwap);
		nodoRtaSwap->exito = 0;*/
	} else {
		marco = list_get(listaMarco,indiceMarco);
	}
 }

void cargarTlb(t_nodo_mem * nodoInstruccion, t_marco * marco){
	int devolverNodoTLBLibre(t_tlb * nodo) {
		return (nodo->processID == NULO);
	}

	t_tlb * nodoTLB = malloc(sizeof(t_tlb));
	nodoTLB = list_find(listaTLB, (void *) devolverNodoTLBLibre);

	if(nodoTLB == NULL){
		nodoTLB = list_get(listaTLB, 0);
	}
	nodoTLB->processID = nodoInstruccion->pid;
	nodoTLB->numeroPagina = marco->numeroPagina;
	nodoTLB->marco = marco->numeroMarco;

}

int valorPagina(char * instruccion){
	int character = 0;
	while (!string_equals_ignore_case(string_substring(instruccion, character, 1),"\'")){
		character++;
	}
	int pagina = atoi(string_substring(instruccion, 9, character));
	return pagina;
}

int posicionComilla(char * instruccion){
	int character = 0;
	while (!string_equals_ignore_case(string_substring(instruccion, character, 1),"\"")){
		character++;
	}
	return character;
}

void escribirMarco(int processID, int marco, char * texto, int numeroPagina,int tipo){
	t_marco * ptrMarco = NULL;
	ptrMarco = list_get(listaMarco, marco);
	ptrMarco->processID = processID;
	if (ptrMarco != NULL){
		if (LEER == tipo) {
			//es lectura
			ptrMarco->bitLeido = 1;
			if (texto != ""){
				strcpy(ptrMarco->valor,texto);
			}
			//list_replace(listaMarco, marco, marco_create(processID, texto, marco, numeroPagina, 0));
		} else {
			//es escritura
			ptrMarco->bitLeido = 1;
			ptrMarco->bitModificacion = 1;
			strcpy(ptrMarco->valor,texto);
			//list_replace(listaMarco, marco, marco_create(processID, texto, marco, numeroPagina, 1));
		}
	}
}

void flushMarcosActivacion() {
	sem_post(&mutexFlushMarcos);
}

void* flushTLB() {
	sem_wait(&mutexFlushTLB);
	inicializarTLB();
}

void* flushMarcos() {
	sem_wait(&mutexFlushMarcos);
	inicializarMarco();
	desasignarTodosLosProcesos();
}

void desasignarTodosLosProcesos() {
	int i;
	int j;
	for (i=0; i<listaTablasPaginas->elements_count; i++) {
		t_tablasPaginas * nodoTabla = list_get(listaTablasPaginas, i);
		for (j=0; nodoTabla->listaPaginas; j++) {
			t_tablaPaginasProceso * nodoPagina = list_get(nodoTabla->listaPaginas, j);
			nodoPagina->numeroMarco = NULO;
		}
	}
}

void flushTLBActivacion() {
	sem_post(&mutexFlushTLB);
}


int recibirNodoDeCPU(t_nodo_mem * nodo){
	unsigned char buffer[100];
	int nbytes;
	nbytes = recv(socketCpu , buffer , sizeof(buffer) , 0);
	desempaquetarNodoInstruccion(buffer,nodo);
	return nbytes;
}



void desempaquetarNodoInstruccion(unsigned char *buffer,t_nodo_mem * nodo){

	char instruccion[50];
	unpack(buffer,SECUENCIA_CPU_MEM,&nodo->pid,instruccion);

	pid = nodo->pid;
	strcpy(linea,instruccion);
	nodo->instruccion = instruccion;
}


int enviarMensajeDeNodoACPU(t_resp_swap_mem * nodo)
{
	int nbytes;
	unsigned char buffer[1024];
	empaquetarNodoRtaCPU(buffer,nodo);
	nbytes = send(socketCpu, buffer, sizeof(buffer) , 0);
	if (nbytes == 0) {
		printf("Memoria: Socket CPU desconectado.\n");
		//TODO CERRAR LA CPU
	} else if (nbytes < 0){
		printf("Memoria: Socket CPU envío de mensaje fallido.\n");
		perror("Error - Enviando mensaje");
	}
	return nbytes;
}

void empaquetarNodoRtaCPU(unsigned char *buffer,t_resp_swap_mem * nodo)
{
	unsigned int tamanioBuffer;
	tamanioBuffer = pack(buffer,SECUENCIA_NODO_RTA_SWAP_MEM,
			nodo->tipo,nodo->exito,nodo->pagina,nodo->contenido);
}


int recibirNodoDeRtaSwap(t_resp_swap_mem * nodo)
{
	unsigned char buffer[1024];
	int nbytes;
	if ((nbytes = recv(socketSwap , buffer , sizeof(buffer) , 0)) < 0){
		printf("Memoria: Error recibiendo mensaje de Swap");
	} else if (nbytes == 0) {
		printf("Memoria: Socket Swap desconectado");
		//TODO CERRAR LA MEM
	}
	desempaquetarNodoRtaSwap(buffer,nodo);
	return nbytes;
}



void desempaquetarNodoRtaSwap(unsigned char *buffer,t_resp_swap_mem * nodo)
{
	//t_pcb * pcb = malloc(sizeof(t_pcb));
	char contenido[50];
	unpack(buffer,SECUENCIA_NODO_RTA_SWAP_MEM,&nodo->tipo,&nodo->exito,&nodo->pagina,contenido);

	nodo->contenido = contenido;
}


int enviarMensajeDeNodoASWAP(t_nodo_mem_swap * nodo)
{
	int nbytes;
	unsigned char buffer[1024];
	empaquetarNodoMemSWAP(buffer,nodo);
	nbytes = send(socketSwap, buffer, sizeof(buffer) , 0);
	if (nbytes == 0) {
		printf("Memoria: Socket SWAP desconectado.\n");
		//TODO CERRAR LA CPU
	} else if (nbytes < 0){
		printf("Memoria: Socket SWAP envío de mensaje fallido.\n");
		perror("Error - Enviando mensaje");
	}
	return nbytes;
}

void empaquetarNodoMemSWAP(unsigned char *buffer,t_nodo_mem_swap * nodo)
{
	unsigned int tamanioBuffer;
	tamanioBuffer = pack(buffer,SECUENCIA_MEM_SWAP,
			nodo->tipo,nodo->pid,nodo->pagina,nodo->contenido);
}


void* monitorearSockets(){

	fd_set coleccionSockets;    // coleccion de sockets
	fd_set coleccionTemp;  // coleccionTemp de sockets temporal

    int nuevoCliente;        // socket cliente recibido
    int i;
    int colMax;
    int nbytes;
    socketSwap = socketCrearCliente(PUERTO_SWAP,IP_SWAP,"Memoria","Swap");
    socketServidor = socketCrearServidor(PUERTO_ESCUCHA,"Memoria");


    FD_ZERO(&coleccionSockets);    // Limpia los sets de colecciones
    FD_ZERO(&coleccionTemp);// Limpia los sets de colecciones
    // agrega el servidor a la colección
	FD_SET(socketServidor, &coleccionSockets);
	FD_SET(socketSwap, &coleccionSockets);
	if (socketSwap < socketServidor){
		colMax = socketServidor;
	} else {
		colMax = socketSwap;
	}

    // main loop
    for(;;) {
        coleccionTemp = coleccionSockets; // lo copio para no perderlo
        if (select(colMax+1, &coleccionTemp, NULL, NULL, NULL) == -1) {
            perror("Error - Función Select.");
        }

        // busca el socket que tiene dato por leer
        for(i = 0; i <= colMax; i++) {
            if (FD_ISSET(i, &coleccionTemp)) {
                if (i == socketServidor) {
                    // El servidor tiene que aceptar un nuevo cliente
                	//Aceptamos la conexion entrante, y creamos un nuevo socket mediante el cual nos podamos comunicar.
                    nuevoCliente = socketAceptarConexion(socketServidor,"Memoria","CPU");
                    if (nuevoCliente > 0) {
                        FD_SET(nuevoCliente, &coleccionSockets); // agrega el cliente al set de sockets

                        if (nuevoCliente > colMax) {    // actualiza el maximo
                            colMax = nuevoCliente;
                        }
                        int pid;
                        //la agrega a la lista de CPUs
                        agregarCPUALista(nuevoCliente);
                    }
                } else if (i == socketSwap) {
                    // Recibe una respuesta de Swap
                	sem_post(&mutexRespuestaSwap);
                	if (tamanioSwapMjeGlobal <= 0) {
						// connection closed
						close(i); // bye!
						FD_CLR(i, &coleccionSockets); // remove from master set
						puts("swap desconectado");
					}
                } else {
                	//Tengo un mensaje de algún cliente.
                	//Respuesta de alguna instrucción de CPU
                	recibirSolicitudDeCpu(i, &nbytes);
                	if (nbytes <= 0) {
						// connection closed
                		close(i); // bye!
						FD_CLR(i, &coleccionSockets); // remove from master set
						informarDesconexionCPU(i);
					}

                } // fin recepcion de mensajes de cliente
            } // fin isset
        } // fin recorrido del set
    } // fin del for(;;)
}

void* atenderSolicitudes()
{

	linea = malloc(strlen("")+1);
	nodoInstruccion = NULL;
	while (1){
		nodoInstruccion = list_get(listaSolicitudes,0);
		//Buscar una CPU disponible en la colección de CPUS donde agrega el monitor
		if (nodoInstruccion != NULL){
			interpretarLinea(nodoInstruccion->instruccion);
		}
		nodoInstruccion = NULL;

	}

}


void agregarCPUALista(int socketCpu) {
	t_cpu * nodoCpu = malloc(sizeof(t_cpu));
	nodoCpu->pid = listaDeCPUs->elements_count;
	nodoCpu->socket = socketCpu;
	printf("CPU: %d, socket: %d, agregada a la lista de CPUs.\n", nodoCpu->pid,nodoCpu->socket );
	list_add(listaDeCPUs, nodoCpu);
}

void informarDesconexionCPU(int socketCPU)
{
	bool buscarCPUporSocket(t_cpu * nodoCPU) {
		return (nodoCPU->socket == socketCPU);
	}
	t_cpu* nodoCPU=NULL;
	nodoCPU = list_find(listaDeCPUs,(void*)buscarCPUporSocket);
	list_remove_by_condition(listaDeCPUs,(void*)buscarCPUporSocket);
	printf("La CPU: %d ha sido desconectada.\n", nodoCPU->pid);
}

void recibirSolicitudDeCpu(int socket, int * nbytes){
	socketCpu = socket;
	t_nodo_mem * nodo = malloc(sizeof(t_nodo_mem));
	int err = recibirNodoDeCPU(nodo);
	*nbytes = err;//Dereferencia del puntero para cambmiarle el valor
	if (err > 0){
		//agrega a lista
		list_add(listaSolicitudes,nodo);
		nodoInstruccion = nodo;
	}
}
