#include "memoria.h"

#include <commons/string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>

int main(int argc, char* argv[]) {
	sem_init(&mutexFlushMarcos, 0, 1);
	sem_init(&mutexFlushTLB, 0, 1);
	sem_init(&cantSolicitudes,0,0);
	listaTablasPaginas = list_create();
	listaMarco = list_create();
	listaTLB = list_create();
	listaDeCPUs = list_create();
	listaSolicitudes = list_create();

	levantarCfgInicial();
	configurarSockets();
	//ESTRUCTURAS
	inicializarTLB();
	inicializarMarco();

	//Tratamiento de la señan enviada por el SO

	signal(SIGINT, rutina);
	signal(SIGUSR1, rutina);
	signal(SIGUSR2, rutina);

	pthread_t tasaDeAciertos;
	char *m10 = "tasaDeAciertos";
	int r10;

	r10 = pthread_create( &tasaDeAciertos, NULL, calcularTasaAciertos, (void*) m10);

	pthread_t monitor;
	char *m3 = "monitor";
	int r3;

	r3 = pthread_create( &monitor, NULL, monitorearSockets, (void*) m3);

	pthread_t atenderSol;
	char *m2 = "atenderSol";
	int r1;

	r1 = pthread_create( &atenderSol, NULL, atenderSolicitudes, (void*) m2);
	pthread_join(atenderSol,NULL);
	return 1;
}

void * calcularTasaAciertos() {
	while(1) {
		sleep(60);
		if (comandosTotales > 0) {
			printf("Tasa de aciertos de la TLB historica: %g %\n", (double)((aciertosTLB / comandosTotales) * 100));
		} else {
			printf("Tasa de aciertos de la TLB historica: no se ha ejecutado ningun programa\n");
		}
	}
}

static t_marco * marco_create(int numeroMarco) {
	t_marco *new = malloc(sizeof(t_marco));
    new->processID = NULO;
    new->numeroMarco = numeroMarco;
    new->numeroPagina = 0;
    new->bitModificacion = 1;
    new->bitLeido = 0;
    new->valor = malloc(sizeof("")+1);
    strcpy(new->valor,"NULL");
    //Si esta en 1 arranca desde ahi
    new->punteroClock = 0;
    return new;
}

static t_tlb * tlb_create() {
	t_tlb * new = malloc(sizeof(t_tlb));
    new->processID = NULO;
    new->numeroPagina = NULO;
    new->marco = NULO;
    return new;
}

void inicializarTLB() {
	int i = 0;
	for (i=0; i<ENTRADAS_TLB; i++) {
		list_add(listaTLB, tlb_create());
	}
}


void inicializarMarco() {
	uint32_t i = 0;
	int numeroMarco;
	for(i=0; i<CANTIDAD_MARCOS; i++) {
		numeroMarco = i;
		list_add(listaMarco, marco_create(numeroMarco));
	}
}


void levantarCfgInicial()
{
	char cfgFin[] ="/memoria/src/config.cfg";//Consola
	//char cfgFin[] ="/src/config.cfg"; //eclipse

	char *dir = getcwd(NULL, 0);

	char *directorioActual = malloc(strlen(dir)+strlen(cfgFin)+1);

	strcpy(directorioActual,dir);
	strcat(directorioActual,cfgFin);

	puts(directorioActual);

	archConfig = malloc(sizeof(t_config));
	archivoLog = log_create("memoria.log", "Memoria", false, 2);
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

	RETARDO_MEMORIA =config_get_double_value(archConfig,"RETARDO_MEMORIA");

	CANTIDAD_MARCOS = config_get_long_value(archConfig,"CANTIDAD_MARCOS");

	TAMANIO_MARCO = config_get_long_value(archConfig,"TAMANIO_MARCO");

	MAXIMO_MARCOS_POR_PROCESO = config_get_long_value(archConfig,"MAXIMO_MARCOS_POR_PROCESO");

	ENTRADAS_TLB = config_get_long_value(archConfig,"ENTRADAS_TLB");
	strcpy(TLB_HABILITADA,config_get_string_value(archConfig,"TLB_HABILITADA"));
	strcpy(ALGORITMO_REEMPLAZO,config_get_string_value(archConfig,"ALGORITMO_REEMPLAZO"));

	free(directorioActual);
}

void configurarSockets()
{
	socketSwap = socketCrearCliente(PUERTO_SWAP,IP_SWAP,"Memoria","Swap");
}


void rutina (int n) {
	switch (n) {
		case SIGINT:
			printf("Flush de marcos.\n");
			pthread_t threadFlushMarcos;
			char *m2 = "threadFlushMarcos";
			int r7;

			r7 = pthread_create( &threadFlushMarcos, NULL, flushMarcos, (void*) m2);
			log_info(archivoLog,"Flush de Marcos.");
		break;
		case SIGUSR1:
			printf("Flush de TLB.\n");
			log_info(archivoLog,"Flush de TLB.");
			pthread_t threadFlushTLB;
			char *m1 = "threadFlushTLB";
			int r5;

			r5 = pthread_create( &threadFlushTLB, NULL, flushTLB, (void*) m1);
		break;
		case SIGUSR2:
			printf("Dump de memoria.\n");
			log_info(archivoLog,"Dump de memoria.");
			int pid = fork();

			if (pid == 0) {
				dumpMemoria();
			}
		break;
	}
}

static t_tablaPaginasProceso * nodoTablaPaginaProceso_create(int numeroMarco, int numeroPagina, int ingreso) {
	t_tablaPaginasProceso * new = malloc(sizeof(t_tablaPaginasProceso));

	new->numeroMarco = numeroMarco;
	new->numeroPagina = numeroPagina;
	new->ingreso = ingreso;

	return new;
}

void inicializarTablaDePaginas(int cantidadPaginas, int pid) {
	t_tablasPaginas * nodoTablaPaginas = malloc(sizeof(t_tablasPaginas));
	nodoTablaPaginas->processID = pid;
	nodoTablaPaginas->contadorPageFault = 0;
	nodoTablaPaginas->paginasAccedidas = 0;
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
		printf("encontrado en TLB marco %d\n",nodoTLB->marco);
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
		//printf("La pagina %d, con numero de marco %d \n", nodo->numeroPagina, nodo->numeroMarco);
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

	if (nodoMarco == NULL) {
		return NULO;
	}

	return (nodoMarco->numeroMarco);

}


//Se usar para LEER u ESCRIBIR, busca un marco en TLB, luego lo busca en TABLA DE PAGINAS.
//Si es necesario envia/pide a swap.
static t_marco * detectarPageFault(t_nodo_mem * nodoInst, int numeroPagina) {
	int resultadoBusqueda;
	t_marco * nodoMarco = NULL;
	bool devolverValor(t_marco * nodo) {
		return (nodo->numeroMarco == resultadoBusqueda);
	}
	bool marcosPorProceso(t_marco * nodo) {
		return (nodo->processID == nodoInst->pid);
	}
	void limpiarPunterosClock(t_marco * nodo){
		nodo->punteroClock = 0;
	}
	t_tablasPaginas * tablaDeProceso;
	t_tablaPaginasProceso * nodoPagina;
	resultadoBusqueda = buscarEnTLB(nodoInst->pid, numeroPagina);
	if (resultadoBusqueda == NULO) {
		tablaDeProceso = buscarTablaPaginas(nodoInst->pid);
		nodoPagina = obtenerPagina(numeroPagina, tablaDeProceso);
		resultadoBusqueda = nodoPagina->numeroMarco;
	}
	//Encontrado en TLB
	if (resultadoBusqueda != NULO){
		//No hubo PF
		aciertosTLB++;
		nodoMarco = list_find(listaMarco, (void*) devolverValor);
		tablaDeProceso = buscarTablaPaginas(nodoInst->pid);
		tablaDeProceso->paginasAccedidas++;
		if(string_equals_ignore_case(ALGORITMO_REEMPLAZO, "LRU")){
			tablaDeProceso = buscarTablaPaginas(nodoInst->pid);
			nodoPagina = obtenerPagina(numeroPagina, tablaDeProceso);
			nodoPagina->ingreso = LRU;
		}
		if(string_equals_ignore_case(ALGORITMO_REEMPLAZO, "CLOCK")){
			nodoMarco->bitLeido = 1;
			t_list * listaMarcoFiltrados = list_filter(listaMarco, (void*) marcosPorProceso);
			//list_iterate(listaMarcoFiltrados, limpiarPunterosClock);
			//nodoMarco->punteroClock = 1;
		}
	}
	return nodoMarco;
}

int algoritmoReemplazo(int processID){
	int marco;
	if (string_equals_ignore_case(ALGORITMO_REEMPLAZO, "FIFO")) {
		marco = algoritmoReemplazoFIFO(processID);
	}
	if (string_equals_ignore_case(ALGORITMO_REEMPLAZO, "LRU")) {
		marco = algoritmoReemplazoLRU(processID);
	}
	if (string_equals_ignore_case(ALGORITMO_REEMPLAZO, "CLOCK")) {
		marco = algoritmoReemplazoClock(processID);
	}
	return marco;
}

int algoritmoReemplazoClock(int processID) {
	int numeroMarco = NULO;
	int indiceMarco = -1;
	int contadorMarco = 1;
	bool marcosPorProceso(t_marco * nodo) {
		return (nodo->processID == processID);
	}
	bool devolverMarcoPuntero(t_marco * nodo){
		indiceMarco++;
		return (nodo->punteroClock == 1);
	}
	t_list * listaMarcoFiltrados = list_create();
	listaMarcoFiltrados = list_filter(listaMarco, (void*) marcosPorProceso);
	list_find(listaMarcoFiltrados, (void*) devolverMarcoPuntero);
	while (numeroMarco == NULO) {
		contadorMarco = 1;
		while (contadorMarco <= MAXIMO_MARCOS_POR_PROCESO && numeroMarco == NULO) {
			if ((indiceMarco + 1) >= MAXIMO_MARCOS_POR_PROCESO) {
				indiceMarco = 0;
			} else {
				indiceMarco++;
			}
			t_marco * marco = list_get(listaMarcoFiltrados, indiceMarco);
			if (marco->bitLeido == 0 && marco->bitModificacion == 0) {
				numeroMarco = marco->numeroMarco;
			}
			contadorMarco++;
		}
		contadorMarco = 1;
		while (contadorMarco <= MAXIMO_MARCOS_POR_PROCESO && numeroMarco == NULO) {
			if ((indiceMarco + 1) >= MAXIMO_MARCOS_POR_PROCESO) {
				indiceMarco = 0;
			} else {
				indiceMarco++;
			}
			t_marco * marco = list_get(listaMarcoFiltrados, indiceMarco);
			if (marco->bitLeido == 0 && marco->bitModificacion == 1) {
				numeroMarco = marco->numeroMarco;
			} else {
				marco->bitLeido = 0;
			}
			contadorMarco++;
		}
	}

	return numeroMarco;
}

int algoritmoReemplazoLRU(int processID) {
	bool ordenarParaLRU(t_tablaPaginasProceso * nodo1, t_tablaPaginasProceso * nodo2) {
		return (nodo1->ingreso < nodo2->ingreso);
	}
	t_tablasPaginas * tablaDeProceso = NULL;

	tablaDeProceso = buscarTablaPaginas(processID);
	list_sort(tablaDeProceso->listaPaginas, (void*) ordenarParaLRU);

	t_tablaPaginasProceso * nodoPagina = list_get(tablaDeProceso->listaPaginas, 0);

	//es el que va a ser reemplazado
	int numMarcoPa = nodoPagina->numeroMarco;

	return numMarcoPa;
}

static t_tablaPaginasProceso * buscarElMasAntiguo(t_list * listaDePaginas) {
	int i;
	t_tablaPaginasProceso * nodoPagina = malloc(sizeof(t_tablaPaginasProceso));
	t_tablaPaginasProceso * nodoPagina2 = malloc(sizeof(t_tablaPaginasProceso));
	for (i=1; i < (list_size(listaDePaginas)-1); i++) {
		nodoPagina = list_get(listaDePaginas, 0);
		nodoPagina2 = list_get(listaDePaginas, i);
		if (nodoPagina->ingreso < nodoPagina2->ingreso) {
			//no hace nada
		} else {
			nodoPagina = nodoPagina2;
		}
	}

	return nodoPagina;
}

int algoritmoReemplazoFIFO(int processID) {
	bool ordenarParaFIFO(t_tablaPaginasProceso * nodo1, t_tablaPaginasProceso * nodo2) {
		return (nodo1->ingreso < nodo2->ingreso);
	}
	t_tablasPaginas * tablaDeProceso = NULL;

	tablaDeProceso = buscarTablaPaginas(processID);
	list_sort(tablaDeProceso->listaPaginas, (void*) ordenarParaFIFO);

	t_tablaPaginasProceso * nodoPagina = buscarElMasAntiguo(tablaDeProceso->listaPaginas);

	//t_tablaPaginasProceso * nodoPagina = list_get(tablaDeProceso->listaPaginas, 0);

	//es el que va a ser reemplazado
	int numMarcoPa = nodoPagina->numeroMarco;
	/*
	int conMarcosAsignados(t_tablaPaginasProceso * nodo) {
		return (nodo->ingreso != NULO);
	}
	t_list * listaPaginasAlgoritmo = list_filter(tablaDeProceso->listaPaginas, (void*) conMarcosAsignados);
	int i;
	for (i=1; i<listaPaginasAlgoritmo->elements_count; i++) {
		t_tablaPaginasProceso * nodoPagina2 = list_get(listaPaginasAlgoritmo, i);
		nodoPagina2->ingreso--;
	}*/

	return numMarcoPa;
}

void actualizarMarco(char * texto,int pid, int numeroPagina, int indiceMarco, int tipo)
{
		escribirMarco(pid, indiceMarco, texto, numeroPagina,tipo);
		actualizarNodoPaginas(indiceMarco, pid, numeroPagina);
		/*
		if (tipo == ESCRIBIR) {
			///////TESTING
			t_tablasPaginas * tablaDeProceso = malloc(sizeof(t_tablasPaginas));

			tablaDeProceso = buscarTablaPaginas(pid);

			int contarMarcosAsignados(t_tablaPaginasProceso * nodo) {
				printf(" ESCRITURA La pagina %d, con numero de marco %d, ingreso %d \n", nodo->numeroPagina, nodo->numeroMarco, nodo->ingreso);
				return (nodo->numeroMarco != NULO);
			}
			int valor = list_count_satisfying(tablaDeProceso->listaPaginas, (void*) contarMarcosAsignados);
			//////FIN TESTING
		} else {
			///////TESTING
			t_tablasPaginas * tablaDeProceso = malloc(sizeof(t_tablasPaginas));

			tablaDeProceso = buscarTablaPaginas(pid);

			int contarMarcosAsignados(t_tablaPaginasProceso * nodo) {
				printf(" LECTURA La pagina %d, con numero de marco %d, ingreso %d \n", nodo->numeroPagina, nodo->numeroMarco, nodo->ingreso);
				return (nodo->numeroMarco != NULO);
			}
			int valor = list_count_satisfying(tablaDeProceso->listaPaginas, (void*) contarMarcosAsignados);
			//////FIN TESTING
		}
		*/
}

void desasignarMarco(int processID, int marco) {
	t_tablasPaginas * tablaDeProceso;
	t_tablaPaginasProceso * nodoPagina = malloc(sizeof(t_tablaPaginasProceso));

	tablaDeProceso = buscarTablaPaginas(processID);

	nodoPagina = obtenerPaginaPorNumMarco(marco, tablaDeProceso);

	t_marco * nodoMarco = list_get(listaMarco, marco);
	if (nodoMarco->bitModificacion == 1){
		//TODO escribir en SWAP
		printf("PageFault (REEMPLAZO): Guardando contenido modificado a Swap: Proceso %d - Pagina %d - Contenido %s.\n",processID,nodoMarco->numeroPagina,nodoMarco->valor);
		t_nodo_mem_swap * nodito = malloc(sizeof(t_nodo_mem_swap));
		nodito->tipo = ESCRIBIR;
		nodito->pagina = nodoMarco->numeroPagina;
		nodito->pid = processID;
		nodito->contenido = nodoMarco->valor;
		enviarMensajeDeNodoASWAP(nodito);
		free(nodito);
		//recibirNodoDeRtaSwap(nodoRtaSwap);
	}
	nodoPagina->ingreso = NULO;
	nodoPagina->numeroMarco = NULO;
}

void actualizarNodoPaginas(int indiceMarco, int processID, int numeroPagina) {
	t_tablasPaginas * tablaDeProceso;
	t_tablaPaginasProceso * nodoPagina;

	tablaDeProceso = buscarTablaPaginas(processID);

	nodoPagina = obtenerPagina(numeroPagina, tablaDeProceso);

	/*
	int contarMarcosAsignados(t_tablaPaginasProceso * nodo) {
		return (nodo->numeroMarco != NULO);
	}
	*/
	/*
	if (string_equals_ignore_case(ALGORITMO_REEMPLAZO, "FIFO") ) {
		//int valor = list_count_satisfying(tablaDeProceso->listaPaginas, (void*) contarMarcosAsignados);
		nodoPagina->ingreso = bitIngresoFIFO;
		bitIngresoFIFO++;
	}
	*/

	if (string_equals_ignore_case(ALGORITMO_REEMPLAZO, "LRU")) {

		modificarBitIngresoLRU(processID);

		nodoPagina->ingreso = LRU;
	}
	/*
	if (string_equals_ignore_case(ALGORITMO_REEMPLAZO, "CLOCK")) {
		modificarBitsClock(processID,listaMarco,indiceMarco);
	}*/
	nodoPagina->numeroMarco = indiceMarco;
}

int interpretarLinea(t_nodo_mem * nodoInst)
{
	int pid = nodoInst->pid;

    //Envio a SWAP
    nodoASwap = malloc(sizeof(t_nodo_mem_swap));
	nodoASwap->pid = pid;
	nodoASwap->contenido = malloc(strlen("")+1);
	strcpy(nodoASwap->contenido,"\0");

	//Respuesta a CPU
	nodoRtaSwap = malloc(sizeof(t_resp_swap_mem));
	nodoRtaSwap->contenido = malloc(strlen("")+1);
	strcpy(nodoRtaSwap->contenido,"\0");
	int pagina;
	pagina = nodoInst->pagina;
	int finalizaPorError = 0;
	//para la Lectura/escritura
	t_marco * marco = NULL;
	switch(nodoInst->instruccion){
		case INICIAR:
			nodoASwap->tipo = INICIAR;
			nodoRtaSwap->tipo = INICIAR;
			nodoASwap->pagina = pagina;
			enviarMensajeDeNodoASWAP(nodoASwap);
			recibirNodoDeRtaSwap(nodoRtaSwap);
			if (nodoRtaSwap->exito){
				inicializarTablaDePaginas(pagina, pid);
			}
			enviarMensajeDeNodoACPU(nodoRtaSwap);
			log_info(archivoLog, "Proceso ID %d, cantidad de paginas %d. Inicio: %s", pid, pagina,traducirExitoStatus(nodoRtaSwap->exito));
			printf("Proceso ID %d, cantidad de paginas %d. Inicio: %s\n", pid, pagina,traducirExitoStatus(nodoRtaSwap->exito));
			break;
		case FINALIZAR:
			nodoASwap->tipo = FINALIZAR;
			nodoRtaSwap->tipo = FINALIZAR;
			t_tablasPaginas * tablaDeProceso;
			tablaDeProceso = buscarTablaPaginas(pid);
			log_info(archivoLog, "Cantidad de paginas accedidas: %d del proceso %d.", tablaDeProceso->paginasAccedidas, pid);
			log_info(archivoLog, "Cantidad de page faults: %d, del proceso %d", tablaDeProceso->contadorPageFault, pid);
			finalizarProceso(pid);//Finaliza el proceso
			nodoASwap->pagina = 0;
			enviarMensajeDeNodoASWAP(nodoASwap);
			log_info(archivoLog, "Se ha finalizado el proceso con ID: %d.", pid);
			printf("Se ha finalizado el proceso con ID: %d.\n", pid);
			break;
		default:
			sem_wait(&mutexFlushMarcos);
			//Lectura o Escritura
			comandosTotales++; //para calcuar la tasa de aciertos de la TLB
			marco = detectarPageFault(nodoInst,pagina);
			if (marco == NULL)
			{//Hay page fault
				pageFaultPa++;
				printf("pg n: %d",pageFaultPa);
				pageFaultLectura = 1;
				t_tablasPaginas * tablaDeProceso;
				tablaDeProceso = buscarTablaPaginas(nodoInst->pid);
				tablaDeProceso->contadorPageFault++;
				tablaDeProceso->paginasAccedidas++;
				printf("PageFault: Solicitando contenido a Swap: Proceso %d - Pagina %d\n",pid,pagina);
				nodoASwap->tipo = LEER;
				nodoASwap->pagina = pagina;
				enviarMensajeDeNodoASWAP(nodoASwap);
				recibirNodoDeRtaSwap(nodoRtaSwap);
				marco = seleccionarMarcoVictima(pid);
				if (marco == NULL){
					//No se pudo asignar un marco
					finalizarProceso(pid);
					//Se finaliza el proceso
					log_info(archivoLog, "Finalizado por asignacion de marcos: process ID %d", pid);
					printf("Finalizado por asignacion de marcos: process ID %d\n", pid);
					finalizaPorError = 1;
				} else {
					strcpy(marco->valor, nodoRtaSwap->contenido);
				}
			}
			if (nodoInst->instruccion == LEER)
			{
				nodoRtaSwap->tipo = LEER;
				if (!finalizaPorError){
					nodoRtaSwap->exito = 1;
					strcpy(nodoRtaSwap->contenido, marco->valor);
					log_info(archivoLog, "Se ha leido una pagina: process ID %d, pagina leida %d con el valor '%s'", pid, pagina, marco->valor);
					printf("Se ha leido una pagina: process ID %d, pagina leida %d con el valor '%s'\n", pid, pagina, marco->valor);

				} else {
					//Si falla la CPU debe finalizarlo
					nodoRtaSwap->exito = 0;
				}
				enviarMensajeDeNodoACPU(nodoRtaSwap);
				usleep(RETARDO_MEMORIA);
			} else {
				//ESCRIBIR
				nodoRtaSwap->tipo = ESCRIBIR;
				if (!finalizaPorError){
					nodoRtaSwap->exito = 1;
					char * tamTexto = malloc(sizeof("")+1);
					strncpy(tamTexto,nodoInst->texto,TAMANIO_MARCO);
					strcat(tamTexto,"\0");
					strcpy(marco->valor,tamTexto);
					strcpy(nodoRtaSwap->contenido,tamTexto);
					log_info(archivoLog, "Se ha escrito una pagina: process ID %d, pagina escrita %d con el valor '%s'", pid, pagina, marco->valor);
					printf("Se ha escrito una pagina: process ID %d, pagina escrita %d con el valor '%s'\n", pid, pagina, marco->valor);

				} else {
					//Si falla la CPU debe finalizarlo
					nodoRtaSwap->exito = 0;
				}
				enviarMensajeDeNodoACPU(nodoRtaSwap);
				usleep(RETARDO_MEMORIA);
			}
			//printf("El retardo fue de %g", RETARDO_MEMORIA);
			if (!finalizaPorError){
				//Actualizar el bit de uso y/o Modificacion
				actualizarMarco(marco->valor,pid,pagina,marco->numeroMarco,nodoRtaSwap->tipo);
				//actualiza TLB con el uso reciente.
				cargarTlb(nodoInstruccion, marco, pid);
			}
			sem_post(&mutexFlushMarcos);

			//imprimirTLB();

			break;
	}

    free(nodoRtaSwap);
    free(nodoASwap);

    return 1;
}

void imprimirTLB() {
	void imprimirla(t_tlb * nodo) {
		printf("El marco es: %d, con el numero de pagina %d.\n", nodo->marco, nodo->numeroPagina);
	}

	list_iterate(listaTLB, imprimirla);
}

void finalizarProceso(int pid){
	//Se borra los marcos de ese proceso. INICIO
	int i=0;
	t_marco * marco = NULL;
	bool buscarMarcoPorProceso(t_marco * nodo) {
		return (nodo->processID == pid);
	}

	for (i = 0; i <MAXIMO_MARCOS_POR_PROCESO ; i++) {
		marco = list_find(listaMarco,(void *) buscarMarcoPorProceso);
		if (marco != NULL) {
			if(string_equals_ignore_case(ALGORITMO_REEMPLAZO, "CLOCK")){
				marco->punteroClock = 0;
			}
			marco->processID = NULO;
		}
	}
	//FIN Joaquin

	bool procesoActual(t_tablasPaginas * nodo) {
		return (nodo->processID == pid);
	}
	list_remove_by_condition(listaTablasPaginas, (void *) procesoActual);
}

static t_marco * seleccionarMarcoVictima(int pid)
{
	int indiceMarco = NULO;
	t_marco * marco = NULL;
	int cantMarcosAsignados = cantidadMarcosAsignados(pid);
	if ( cantMarcosAsignados < MAXIMO_MARCOS_POR_PROCESO)
	{
		indiceMarco = obtenerUnMarcoLibre(listaMarco);
		if (string_equals_ignore_case(ALGORITMO_REEMPLAZO, "LRU")) {
			modificarBitIngresoLRU(pid);
		}
		/*
		if(string_equals_ignore_case(ALGORITMO_REEMPLAZO, "CLOCK")){
			if (indiceMarco != NULO) {
				modificarBitsClock(pid, listaMarco, indiceMarco);
			}
		}*/
	} else if (cantMarcosAsignados == MAXIMO_MARCOS_POR_PROCESO) {
		//ejecuta algoritmo de reemplazo solicitado
		//elimina numero de marco en la pagina victima
		indiceMarco = algoritmoReemplazo(pid);
		//Debe escribirlo en swap en caso de haber sido escrita
		printf("La victima es: %d\n", indiceMarco);
		desasignarMarco(pid,indiceMarco);
	} else {
		puts("Error de cantidad de marcos");
	}
	if (indiceMarco != NULO) {
		marco = list_get(listaMarco,indiceMarco);
	}
	return marco;
 }

void cargarTlb(t_nodo_mem * nodoInstruccion, t_marco * marco, int pid){
	sem_wait(&mutexFlushTLB);

	t_tlb * nodoTLB = malloc(sizeof(t_tlb));

	int devolverNodoTLBLibre(t_tlb * nodo) {
		return (nodo->processID == NULO);
	}
	//buscar si ya existe ese marco asignado a ese proceso
	int encontrarMarcoRepetido(t_tlb * nodo) {
		return (nodo->marco == marco->numeroMarco);
	}

	nodoTLB = list_find(listaTLB, (void *) encontrarMarcoRepetido);

	if (nodoTLB == NULL) {
		nodoTLB = list_find(listaTLB, (void *) devolverNodoTLBLibre);
		nodoTLB->marco = marco->numeroMarco;
		nodoTLB->processID = nodoInstruccion->pid;
		nodoTLB->numeroPagina = marco->numeroPagina;
		printf("Marco: %d, processID: %d, Numero Pagina: %d\n", nodoTLB->marco, nodoTLB->processID, nodoTLB->numeroPagina);
		if (nodoTLB == NULL){
			nodoTLB = list_remove(listaTLB,0);
			list_add(listaTLB,nodoTLB);
		}
	} else {
		nodoTLB->numeroPagina = marco->numeroPagina;
		nodoTLB->processID = nodoInstruccion->pid;
		printf("processID: %d, Numero Pagina: %d\n", nodoTLB->processID, nodoTLB->numeroPagina);
	}


	sem_post(&mutexFlushTLB);
}

int valorPagina(char * instruccion){
	int character = 0;
	while (!string_equals_ignore_case(string_substring(instruccion, character, 1),"\"")){
		character++;
	}
	int pagina = atoi(string_substring(instruccion, 9, character));
	return pagina;

}


void escribirMarco(int processID, int marco, char * texto, int numeroPagina,int tipo){
	t_marco * ptrMarco = NULL;
	char * tamTexto = malloc(sizeof("")+1);
	strncpy(tamTexto,texto,TAMANIO_MARCO);
	strcat(tamTexto,"\0");
	ptrMarco = list_get(listaMarco, marco);
	ptrMarco->processID = processID;
	ptrMarco->numeroPagina = numeroPagina;
	if (ptrMarco != NULL){
		if (LEER == tipo) {
			//es lectura
			ptrMarco->bitLeido = 1;
			//Si hubo PF guardo el valor
			strcpy(ptrMarco->valor,tamTexto);

			if (pageFaultLectura == 1 && string_equals_ignore_case(ALGORITMO_REEMPLAZO, "FIFO") ) {
				//int valor = list_count_satisfying(tablaDeProceso->listaPaginas, (void*) contarMarcosAsignados);
				t_tablasPaginas * tablaDeProceso = NULL;

				tablaDeProceso = buscarTablaPaginas(processID);
				t_tablaPaginasProceso * nodoPagina = NULL;

				nodoPagina = obtenerPagina(numeroPagina, tablaDeProceso);
				nodoPagina->ingreso = bitIngresoFIFO;
				bitIngresoFIFO++;

				ptrMarco->bitModificacion = 0;

			}

			if (pageFaultLectura == 1 && string_equals_ignore_case(ALGORITMO_REEMPLAZO, "CLOCK")) {
				bool marcosPorProceso(t_marco * nodo) {
					return (nodo->processID == processID);
				}
				void limpiarPunterosClock(t_marco * nodo){
					nodo->punteroClock = 0;
				}
				t_list * listaMarcoFiltrados = list_create();
				listaMarcoFiltrados = list_filter(listaMarco, (void*) marcosPorProceso);
				list_iterate(listaMarcoFiltrados, limpiarPunterosClock);
				ptrMarco->punteroClock = 1;

				ptrMarco->bitModificacion = 0;
			}

		} else {
			//es escritura

			if (string_equals_ignore_case(ALGORITMO_REEMPLAZO, "FIFO")) {
				if (ptrMarco->bitModificacion == 1) {
					//int valor = list_count_satisfying(tablaDeProceso->listaPaginas, (void*) contarMarcosAsignados);
					t_tablasPaginas * tablaDeProceso = NULL;

					tablaDeProceso = buscarTablaPaginas(processID);
					t_tablaPaginasProceso * nodoPagina = NULL;

					nodoPagina = obtenerPagina(numeroPagina, tablaDeProceso);
					nodoPagina->ingreso = bitIngresoFIFO;
					bitIngresoFIFO++;
				} else {
					//no hace nada
				}
			}

			if (string_equals_ignore_case(ALGORITMO_REEMPLAZO, "CLOCK")) {
				bool marcosPorProceso(t_marco * nodo) {
					return (nodo->processID == processID);
				}
				void limpiarPunterosClock(t_marco * nodo){
					nodo->punteroClock = 0;
				}
				t_list * listaMarcoFiltrados = list_create();
				listaMarcoFiltrados = list_filter(listaMarco, (void*) marcosPorProceso);
				list_iterate(listaMarcoFiltrados, limpiarPunterosClock);
				ptrMarco->punteroClock = 1;
			}

			ptrMarco->bitLeido = 1;
			ptrMarco->bitModificacion = 1;

			strcpy(ptrMarco->valor,tamTexto);
		}
		pageFaultLectura = 0;
		//printf("Bit del FIFO: %d \n", bitIngresoFIFO);
	}
}

void dumpMemoria(){
	int pid = fork();
	if (pid == 0) {
		int i;
		for (i = 0; i < listaMarco->elements_count; i++) {
			t_marco * nodoMarco = list_get(listaMarco, i);
			log_info(archivoLog,"Volcado de memoria del marco: %d, pagina: %d, valor de %s",nodoMarco->numeroMarco,nodoMarco->numeroPagina, nodoMarco->valor);
		}
	}
}

void* flushTLB() {
	sem_wait(&mutexFlushTLB);
	listaTLB = list_create();
	inicializarTLB();
	sem_post(&mutexFlushTLB);
}

void* flushMarcos() {
	sem_wait(&mutexFlushMarcos);
	listaMarco = list_create();
	inicializarMarco();
	desasignarTodosLosProcesos();
	sem_post(&mutexFlushMarcos);
}

void desasignarTodosLosProcesos() {
	void limpiarTodo(t_tablasPaginas * nodo) {
		void limpiarPaginas(t_tablaPaginasProceso * nodoPagina) {
			nodoPagina->ingreso = NULO;
			nodoPagina->numeroMarco = NULO;
		}
		list_iterate(nodo->listaPaginas, limpiarPaginas);
	}
	list_iterate(listaTablasPaginas, limpiarTodo);

}


int recibirNodoDeCPU(t_nodo_mem * nodo){
	unsigned char buffer[1024];
	int nbytes;
	nbytes = recv(socketCpu , buffer , sizeof(buffer) , 0);
	desempaquetarNodoInstruccion(buffer,nodo);
	return nbytes;
}



void desempaquetarNodoInstruccion(unsigned char *buffer,t_nodo_mem * nodo){

	char texto[100];
	unpack(buffer,SECUENCIA_CPU_MEM,&nodo->pid,&nodo->pagina,&nodo->instruccion,texto);
	strcpy(nodo->texto,texto);
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
	nodo->contenido = malloc(sizeof(contenido));
	strcpy(nodo->contenido, contenido);
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

    socketServidor = socketCrearServidor(PUERTO_ESCUCHA,"Memoria");


    FD_ZERO(&coleccionSockets);    // Limpia los sets de colecciones
    FD_ZERO(&coleccionTemp);// Limpia los sets de colecciones
    // agrega el servidor a la colección
	FD_SET(socketServidor, &coleccionSockets);
	colMax = socketServidor;

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
                } else {
                	//Tengo un mensaje de algún cliente.
                	//Respuesta de alguna instrucción de CPU
                	socketCpu = i;
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
	nodoInstruccion = NULL;
	while (1){
		sem_wait(&cantSolicitudes);
		nodoInstruccion = list_get(listaSolicitudes,0);
		if (nodoInstruccion != NULL){
			interpretarLinea(nodoInstruccion);
			list_remove(listaSolicitudes,0);
		}
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
	t_nodo_mem * nodo = malloc(sizeof(t_nodo_mem));
	nodo->texto = malloc(sizeof("")+1);
	int err = recibirNodoDeCPU(nodo);
	*nbytes = err;//Dereferencia del puntero para cambmiarle el valor
	if (err > 0){
		//agrega a lista
		list_add(listaSolicitudes,nodo);
		sem_post(&cantSolicitudes);
		nodoInstruccion = nodo;
	}
}

void modificarBitIngresoLRU (int pid){

	t_tablasPaginas * tablaPagProceso = buscarTablaPaginas(pid);

	int conMarcosAsignados(t_tablaPaginasProceso * nodo) {
		return (nodo->ingreso != NULO);
	}
	t_list * listaPaginasAlgoritmo = list_filter(tablaPagProceso->listaPaginas, (void*) conMarcosAsignados);
	int i;
	for (i=0; i<listaPaginasAlgoritmo->elements_count; i++) {
		t_tablaPaginasProceso * nodoPagina2 = list_get(listaPaginasAlgoritmo, i);
		nodoPagina2->ingreso--;
	}

}

void modificarBitsClock (int pid, t_list * listaMarcos,int indiceMarco){
	/*bool marcosPorProceso(t_marco * nodo) {
		return (nodo->processID == pid);
	}
	void limpiarPunterosClock(t_marco * nodo){
		nodo->punteroClock = 0;
	}
	bool buscarIndiceMarco(t_marco * nodo){
		return (nodo->numeroMarco == indiceMarco);
	}
	t_list * listaMarcoFiltrados = list_create();
	listaMarcoFiltrados = list_filter(listaMarco, (void*) marcosPorProceso);
	list_iterate(listaMarcoFiltrados, limpiarPunterosClock);
	t_marco * marco = list_find(listaMarco, (void*) buscarIndiceMarco);
	marco->bitLeido = 1;
	marco->punteroClock = 1;*/
}
