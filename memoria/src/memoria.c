#include "memoria.h"

//para mensajes recibidos
char instruccion[30];
char respuesta[30];
int nbytes;

void flushTLB() {
	inicializarTLB();
}

void flushMarcos() {
	inicializarMarco();
	desasignarTodosLosProcesos();
}

void dumpMemoria(){
	int pid = fork();
	if (pid == 0) {
		int i;
		for (i = 0; i < listaMarco->elements_count; i++) {
			t_marco * nodoMarco = list_get(listaMarco, i);
			log_info(archivoLog,"Volcado de memoria del marco: %d con un valor de %s",nodoMarco->numeroMarco, nodoMarco->valor);
		}
	}
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


int main(int argc, char* argv[]) {

	signal(SIGINT, rutina);
	signal(SIGUSR1, rutina);
	signal(SIGUSR2, rutina);

	listaTablasPaginas = list_create();
	listaMarco = list_create();
	listaTLB = list_create();

	const char *programa1[9];
	programa1[0] = "iniciar 9";
	programa1[1] = "escribir 7 'nicolas'";
	programa1[2] = "escribir 1 'juan'";
	programa1[3] = "escribir 2 'florencia'";
	programa1[4] = "escribir 3 'pepe'";
	programa1[5] = "escribir 4 'joaquin'";
	programa1[6] = "leer 7";
	programa1[7] = "leer 4";
	programa1[8] = "finalizar";

	//Consola
	char cfgFin[] ="/memoria/src/config.cfg";
	//Debug
	//char cfgFin[] ="/src/config.cfg";

	char *dir = getcwd(NULL, 0);

	char *directorioActual = malloc(strlen(dir)+strlen(cfgFin)+1);

	strcat(directorioActual,dir);
	strcat(directorioActual,cfgFin);

    int resultado = 1;

	puts(directorioActual);

	archConfig = malloc(sizeof(t_config));
	archivoLog = log_create("memoria.log", "Memoria", false, 2);//Eclipse
	//archConfig = config_create("/home/utnso/rigonijami/tp-2015-2c-riganijomi/memoria/src/config.cfg");
	archConfig = config_create(directorioActual);
	resultado = levantarCfgInicial(archConfig);
	configurarSockets();
	//configurarSockets();
	inicializarTLB();

	inicializarMarco();

	int i=0;
	for (i = 0; i < 10; i++) {
		t_nodo_mem * mem = malloc(sizeof(t_nodo_mem));
		strcpy(mem->instruccion, programa1[i]);
		if (i == 6) {
			int a = 2;
		}
		mem->pid = 1234;
		interpretarLinea(mem);
	}

/*
	int continuar = 1;
	if (resultado == -1 ){
		log_error(archivoLog,"MEM: Error leyendo del archivo de configuracion");
		return -1;
	}else{
		log_info(archivoLog,"MEM: Archivo de configuracion levantado correctamente");
		configurarSockets();
		//sem_init(&sem_mem, 0, 0);
		//sem_init(&sem_sockets, 0, 1);
		//Inicia los parametros

		//Tratamiento de la señan enviada por el SO
	}	signal(SIGINT, rutina);
		signal(SIGUSR1, rutina);
		signal(SIGUSR2, rutina);

		//r1 = pthread_create(&hiloMonitorSockets,NULL,monitorPrepararServidor(&sem_mem,&sem_sockets), (void *) arg1);
		for(;(socketCpu > 0) && continuar;){
			nodoInstruccion = malloc(sizeof(t_nodo_mem));
			//sem_wait(&sem_mem);

			if (socketRecibirMensaje(socketCpu, nodoInstruccion,sizeof(t_nodo_mem)) > 0) {
			// tengo un mensaje de algun cliente
				interpretarLinea(nodoInstruccion);

					/*
							n

				}
				//free(nodoInstruccion);
			} else {
				log_debug(archivoLog,"error recepcion mensaje cpu: %d",socketCpu);
				continuar = 0;
			}//Enviar mensaje CPU
			}
		}

		free(directorioActual);*/
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


int levantarCfgInicial(t_config* archConfig){
	int retorno = 1;
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


	if(RETARDO_MEMORIA == 0 || PUERTO_SWAP == 0 || IP_SWAP == NULL ){
		retorno = -1;
	}
	return retorno;
}

void configurarSockets(){
	//se conecta con el swap que tiene un servidor escuchando
	socketSwap = socketCrearCliente(PUERTO_SWAP,IP_SWAP,"Memoria","Swap");
	//socketServidor = socketCrearServidor(PUERTO_ESCUCHA,"Memoria");
	if (socketServidor > 0){
		socketCpu = socketAceptarConexion(socketServidor,"Memoria","CPU");
	}
}

void rutina (int n) {
	switch (n) {
		case SIGINT:
			log_info(archivoLog,"ingreso el mensaje rutina SIGINT: %d",SIGINT);
			puts("Llego SIGINT");
			flushTLB();
		break;
		case SIGUSR1:
			log_info(archivoLog,"ingreso el mensaje rutina SIGUSR1: %d",SIGUSR1);
			puts("Llego SIGUSR1");
			flushMarcos();
		break;
		case SIGUSR2:
			log_info(archivoLog,"ingreso el mensaje rutina SIGUSR2: %d",SIGUSR2);
			puts("Llego SIGUSR2");
			dumpMemoria();
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
static t_marco * accederAPaginaCiclicamente(t_nodo_mem * nodoInstruccion, int numeroPagina, char * texto) {
	int resultadoBusqueda;
	char * devolverValor(t_marco * nodo) {
		return (nodo->numeroMarco == resultadoBusqueda);
	}
	t_list * listaTablaProceso;
	t_tablasPaginas * tablaDeProceso;
	t_tablaPaginasProceso * nodoPagina;
	listaTablaProceso = list_create();
	int indiceMarco;
	t_marco * marco = malloc(sizeof(t_marco));
	resultadoBusqueda = buscarEnTLB(nodoInstruccion->pid, numeroPagina);
	if (resultadoBusqueda == NULO) {
		//numeroPagina = devolverParteUsableInt(nodoInstruccion->instruccion, 5);
		tablaDeProceso = buscarTablaPaginas(nodoInstruccion->pid);
		nodoPagina = obtenerPagina(numeroPagina, tablaDeProceso);
		resultadoBusqueda = nodoPagina->numeroMarco;
		if (resultadoBusqueda == NULO) {
			//Pide a swap la pagina. Envia el numero de pagina y el processID para que el swap lo traiga de vuelta.
			t_decidirEstructuraSwap * nodoDecidir = malloc(sizeof(t_decidirEstructuraSwap));
			nodoDecidir->mensaje = 1;
			send(socketSwap, nodoDecidir, sizeof(t_decidirEstructuraSwap), 0);
			t_pedirPaginaSwap * nodoPedirPaginaSwap = malloc(sizeof(t_pedirPaginaSwap));
			nodoPedirPaginaSwap->numeroPagina = numeroPagina;
			nodoPedirPaginaSwap->processID = nodoInstruccion->pid;
			if (send(socketSwap, nodoPedirPaginaSwap, sizeof(t_pedirPaginaSwap), 0) > 0) {
				char paginaSwap [1024];
				if (recv(socketSwap, paginaSwap, 1024, 0) > 0) {
					if (cantidadMarcosAsignados(nodoInstruccion->pid) < MAXIMO_MARCOS_POR_PROCESO) {
						indiceMarco = obtenerUnMarcoLibre(listaMarco);
						if (indiceMarco < 0) {
							//finaliza proceso.
							char finalizarProceso [] = "finalizar";
							send(socketCpu, finalizarProceso, sizeof(finalizarProceso), 0);
							t_decidirEstructuraSwap * nodoDecidir = malloc(sizeof(t_decidirEstructuraSwap));
							nodoDecidir->mensaje = 3;
							send(socketSwap, nodoDecidir, sizeof(t_decidirEstructuraSwap), 0);
							t_eliminarPaginaSwap * nodoEliminar = malloc(sizeof(t_eliminarPaginaSwap));
							nodoEliminar->processID = nodoInstruccion->pid;
							nodoEliminar->valor = malloc(sizeof("error") + 1);
							strcpy(nodoEliminar->valor, "error");
							strcat(nodoEliminar->valor, "\0");
							send(socketSwap, nodoEliminar, sizeof(t_eliminarPaginaSwap), 0);
						} else {
							resultadoBusqueda = indiceMarco;
							actualizarMarco(texto,nodoInstruccion->pid, numeroPagina,paginaSwap,indiceMarco);
						}
					}
					else if (cantidadMarcosAsignados(nodoInstruccion->pid) == MAXIMO_MARCOS_POR_PROCESO) {
						//ejecuta algoritmo de reemplazo FIFO.
						//elimina numero de marco en la pagina victima
						int marco = algoritmoReemplazo(nodoInstruccion->pid);
						actualizarMarco(texto,nodoInstruccion->pid, numeroPagina,paginaSwap,marco);
						desasignarMarco(nodoInstruccion->pid,marco);
						resultadoBusqueda = marco;
					}
					puts(paginaSwap);
				}
			}
		}
	}
	marco = list_find(listaMarco, (void*) devolverValor);
	return marco;
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

void actualizarMarco(char * texto,int pid, int numeroPagina, char * paginaSwap, int indiceMarco){
	if (strlen(texto) == 0) {
		escribirMarco(pid, indiceMarco, paginaSwap, numeroPagina);
		actualizarNodoPaginas(indiceMarco, pid, numeroPagina);
	}else{
		escribirMarco(pid, indiceMarco, texto, numeroPagina);
		actualizarNodoPaginas(indiceMarco, pid, numeroPagina);
		t_decidirEstructuraSwap * nodoDecidir = malloc(sizeof(t_decidirEstructuraSwap));
		nodoDecidir->mensaje = 2;
		send(socketSwap, nodoDecidir, sizeof(t_decidirEstructuraSwap), 0);
		t_envioPaginaSwap * envioSwap = malloc(sizeof(t_envioPaginaSwap));
		envioSwap->processID = pid;
		envioSwap->numeroPagina = numeroPagina;
		envioSwap->valor = malloc(strlen(texto) + 1);
		strcpy(envioSwap->valor, texto);
		strcat(envioSwap->valor, "\0");
		envioSwap->tamanioTexto = strlen((const char *)envioSwap->valor);
		send(socketSwap, envioSwap, sizeof(t_envioPaginaSwap), 0);
		send(socketSwap, (const char *)envioSwap->valor, envioSwap->tamanioTexto, 0);
	}
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

int interpretarLinea(t_nodo_mem * nodoInstruccion) {

	int resultadoBusqueda;
    char * valor;
    char * texto;
    char logs[1024];
    int processID;
    t_marco * marco = malloc(sizeof(t_marco));
    t_respuestaCPU * nodoRespuestaCPU = malloc(sizeof(t_respuestaCPU));
    if (esElComando(nodoInstruccion->instruccion, "iniciar")) {
    	int cantidadPaginasProceso;
    	char mensajeDelSwap[1024];
    	cantidadPaginasProceso = devolverParteUsableInt(nodoInstruccion->instruccion, 8);
    	inicializarTablaDePaginas(cantidadPaginasProceso, nodoInstruccion->pid);
    	t_decidirEstructuraSwap * nodoDecidir = malloc(sizeof(t_decidirEstructuraSwap));
    	nodoDecidir->mensaje = 0;
    	send(socketSwap, nodoDecidir, sizeof(t_decidirEstructuraSwap), 0);
    	t_iniciarSwap * nodoIniciarSwap = malloc(sizeof(t_iniciarSwap));
    	nodoIniciarSwap->processID = nodoInstruccion->pid;
    	nodoIniciarSwap->cantidadPaginas = cantidadPaginasProceso;
    	send(socketSwap, nodoIniciarSwap, sizeof(t_iniciarSwap), 0);
    	recv(socketSwap, mensajeDelSwap, 1024, 0);
    	puts(mensajeDelSwap);
		//strcpy(respuesta,"iniciar");
    	nodoRespuestaCPU->mensaje = 1;
		send(socketCpu, nodoRespuestaCPU, sizeof(t_respuestaCPU), 0);
		log_info(archivoLog, "Proceso iniciado: process ID %d, cantidad de paginas %d.", nodoInstruccion->pid, cantidadPaginasProceso);
		return 1;
	} else if (esElComando(nodoInstruccion->instruccion, "leer")) {
		marco = accederAPaginaCiclicamente(nodoInstruccion, devolverParteUsableInt(nodoInstruccion->instruccion, 5), "");
		cargarTlb(nodoInstruccion, marco);
		puts(marco->valor);
		strcpy(respuesta,"AFX");
    	nodoRespuestaCPU->mensaje = 1;
		send(socketCpu, nodoRespuestaCPU, sizeof(t_respuestaCPU), 0);
		log_info(archivoLog, "Se ha leido una pagina: process ID %d, pagina leida %d con el valor '%s'", nodoInstruccion->pid, devolverParteUsableInt(nodoInstruccion->instruccion, 5), marco->valor);
		sleep(RETARDO_MEMORIA);
	} else if (esElComando(nodoInstruccion->instruccion, "escribir")) {
		int pagina = valorPagina(nodoInstruccion->instruccion);
		texto = malloc(sizeof(devolverParteUsable(nodoInstruccion->instruccion, posicionComilla(nodoInstruccion->instruccion))));
		strcpy(texto, devolverParteUsable(nodoInstruccion->instruccion, posicionComilla(nodoInstruccion->instruccion)));
		strtok(texto, "\'");
		char ** textoSinComillas = string_split(texto, "\'");
		strcat(texto, "\0");
		marco = accederAPaginaCiclicamente(nodoInstruccion, pagina, *textoSinComillas);
		cargarTlb(nodoInstruccion, marco);
		printf("%d",nodoInstruccion->pid);
		printf("%d",pagina);
		puts(texto);
    	nodoRespuestaCPU->mensaje = 1;
		send(socketCpu, nodoRespuestaCPU, sizeof(t_respuestaCPU), 0);
		log_info(archivoLog, "Se ha escrito una pagina: process ID %d, pagina escrita %d con el valor '%s'", nodoInstruccion->pid, devolverParteUsableInt(nodoInstruccion->instruccion, 9), marco->valor);
		sleep(RETARDO_MEMORIA);
	} else if (esElComando(nodoInstruccion->instruccion, "finalizar")) {
		int procesoActual(t_tablasPaginas * nodo) {
			return (nodo->processID == nodoInstruccion->pid);
		}
		list_remove_by_condition(listaTablasPaginas, (void *) procesoActual);
		t_decidirEstructuraSwap * nodoDecidir = malloc(sizeof(t_decidirEstructuraSwap));
		nodoDecidir->mensaje = 3;
		send(socketSwap, nodoDecidir, sizeof(t_decidirEstructuraSwap), 0);
		t_eliminarPaginaSwap * nodoEliminar = malloc(sizeof(t_eliminarPaginaSwap));
		nodoEliminar->processID = nodoInstruccion->pid;
		nodoEliminar->valor = malloc(sizeof("finalizar") + 1);
		strcpy(nodoEliminar->valor, "finalizar");
		strcat(nodoEliminar->valor, "\0");
		send(socketSwap, nodoEliminar, sizeof(t_eliminarPaginaSwap), 0);
		log_info(archivoLog, "Se ha finalizado el proceso con ID: %d.", nodoInstruccion->pid);
		puts("finalizado");
	} else {
		strcpy(respuesta,"error");
		perror("comando invalido");
	}

    return 1;
}

void cargarTlb(t_nodo_mem * nodoInstruccion, t_marco * marco){
	int var = 0;

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
	while (!string_equals_ignore_case(string_substring(instruccion, character, 1),"\'")){
		character++;
	}
	return character;
}

void escribirMarco(int processID, int marco, char * texto, int numeroPagina){
	t_marco * ptrMarco = NULL;
	ptrMarco = list_get(listaMarco, marco);
	ptrMarco->processID = processID;
	if (ptrMarco != NULL){
		strcpy(ptrMarco->valor,texto);
		if (!string_equals_ignore_case(texto, "NULL")) {
			//es lectura
			ptrMarco->bitLeido = 1;
			//list_replace(listaMarco, marco, marco_create(processID, texto, marco, numeroPagina, 0));
		} else {
			ptrMarco->bitLeido = 1;
			ptrMarco->bitModificacion = 1;
			//es escritura
			//list_replace(listaMarco, marco, marco_create(processID, texto, marco, numeroPagina, 1));
		}
	}
}



void interpretarRespuestaSwap(t_resp_swap_mem * nodoRespuesta) {

    int tipoResp = nodoRespuesta->tipo;
    int exito = nodoRespuesta->exito;
    switch (tipoResp) {
    		case INICIAR:
				if (exito){
					strcpy(respuesta,"exito-inicio");
					log_info(archivoLog,"Proceso mProc %d creado, cantidad de paginas: %s",nodoInstruccion->pid,devolverParteUsable(nodoInstruccion->instruccion, 8));
				} else {
					strcpy(respuesta,"fallo-inicio");
					log_debug(archivoLog,"Proceso mProc %d NO creado, cantidad de paginas: %s",nodoInstruccion->pid,devolverParteUsable(nodoInstruccion->instruccion, 8));
				}
			break;
    		case LEER:
    			if (exito){
    				log_info(archivoLog,"Proceso mProc %d leido, paginas: %s",nodoInstruccion->pid,devolverParteUsable(nodoInstruccion->instruccion, 5));
    				socketRecibirMensaje(socketSwap, respuesta,nodoRespuesta->largo);
				} else {
					log_debug(archivoLog,"Proceso mProc %d NO leido, paginas: %s",nodoInstruccion->pid,devolverParteUsable(nodoInstruccion->instruccion, 5));
					strcpy(respuesta,"fallo-lectura");
				}
    		break;
    		case ESCRIBIR:
    		//	t_tlb * nodoTLB = malloc(sizeof(t_tlb));
    		//	nodoTLB->datos = malloc(TAMANIO_MARCO);
				//TODO Solo loguear y actualizar las estructuras necesarias
    			if (exito){
					strcpy(respuesta,"exito");
				} else {
					strcpy(respuesta,"fallo");
				}
    		break;
    		case FINALIZAR:
    			//No necesita avisarle al cpu
    			//TODO Solo loguear y limpiar las estructuras necesarias
    			if (exito){
					strcpy(respuesta,"exito");
				} else {
					strcpy(respuesta,"fallo");
				}
			break;
    		default:
    		    perror("mensaje erroneo.");
    	}
}

