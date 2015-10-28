#include "memoria.h"

//para mensajes recibidos
char instruccion[30];
char respuesta[30];
int nbytes;

int main(int argc, char* argv[]) {

	listaTablasPaginas = list_create();
	listaMarco = list_create();
	listaTLB = list_create();
	const char *programa1[8];
	programa1[0] = "iniciar 9";
	programa1[1] = "escribir 8 nicolas";
	programa1[2] = "escribir 5 joaquin";
	programa1[3] = "leer 8";
	programa1[4] = "leer 5";
	programa1[6] = "finalizar";

	char cfgFin[] ="/src/config.cfg";

	char *dir = getcwd(NULL, 0);

	char *directorioActual = malloc(strlen(dir)+strlen(cfgFin)+1);

	strcat(directorioActual,dir);
	strcat(directorioActual,cfgFin);

    int resultado = 1;

	puts(directorioActual);

	archConfig = malloc(sizeof(t_config));
	archivoLog = log_create("memoria.log.log", "Memoria", false, 2);//Eclipse
	archConfig = config_create("/home/utnso/rigonijami/tp-2015-2c-riganijomi/memoria/src/config.cfg");
	resultado = levantarCfgInicial(archConfig);

	inicializarTLB();

	inicializarMarco();

	int i=0;
	for (i = 0; i < 8; i++) {
		t_nodo_mem * mem = malloc(sizeof(t_nodo_mem));
		strcpy(mem->instruccion, programa1[i]);
		mem->pid = 1234;
		interpretarLinea(mem);
	}

	/*
	inicializarMemoria(); //se inicializan los marcos
	inicializarTLB();

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
				if (interpretarLinea(nodoInstruccion->instruccion) < 0) {
					if (socketSwap > 0){
						if (socketEnviarMensaje(socketSwap, nodoInstruccion, sizeof(t_nodo_mem)) > 0) {
							//envio el nodoInstruccion.. recibo respuesta
							t_resp_swap_mem * nodoRespuesta = malloc(sizeof(t_resp_swap_mem));
							nbytes = socketRecibirMensaje(socketSwap, nodoRespuesta,sizeof(t_resp_swap_mem));
							//Interpreta la respuesta
							interpretarRespuestaSwap(nodoRespuesta);
							//La envia
							nbytes = socketEnviarMensaje(socketCpu, respuesta,sizeof(respuesta));
						} else {
							log_debug(archivoLog,"error envio mensaje swap: %d",socketSwap);
						}
						//sem_post(&sem_sockets);
					} else {
						log_debug(archivoLog,"seridor no encontrado swap: %d",socketSwap);
						perror("no hay swap disponible");
					}//If socketSwap
				}
				free(nodoInstruccion);
			} else {
				log_debug(archivoLog,"error recepcion mensaje cpu: %d",socketCpu);
				continuar = 0;
			}//Enviar mensaje CPU
		}

		free(directorioActual);
		return 1;
		**/
}

static t_marco * marco_create(int processID, char * valor, int numeroMarco,  int presencia, int numeroPagina, int bitModificacion) {
	t_marco *new = malloc(sizeof(t_marco));
    new->processID = processID;
    new->numeroMarco = numeroMarco;
    new->presencia = presencia;
    new->numeroPagina = numeroPagina;
    new->bitModificacion = bitModificacion;
    new->valor = malloc(sizeof(valor));
    strcpy(new->valor, valor);
    return new;
}

void inicializarTLB() {
	int i = 0;
	t_tlb * nodoTLB;
	for (i=0; i<ENTRADAS_TLB; i++) {
		nodoTLB = malloc(sizeof(t_tlb));
		list_add(listaTLB, nodoTLB);
		free(nodoTLB);
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
		list_add(listaMarco, marco_create(processID, valor, numeroMarco, presencia, numeroPagina, bitModificacion));
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

/*
void levantarCfgInicial() {
	//Levanta sus puertos cfg e ip para conectarse
	char directorioActual[1024];
	getcwd(directorioActual, sizeof(directorioActual));
	strcat(directorioActual, "/src/config.cfg");

	//puertoSwap = configObtenerPuertoSwap(directorioActual);
	//ipSwap = configObtenerIpSwap(directorioActual);
	//puertoEscucha = configObtenerPuertoEscucha(directorioActual);
	maxMarcosProceso = configObtenerMaxMarcosProceso(directorioActual);
	cantMarcos = configObtenerCantMarcos(directorioActual);
	tamanioMarco = configObtenerTamanioMarco(directorioActual);
	entradasTLB = configObtenerEntradasTLB(directorioActual);
	TLBHabilitada = configObtenerTLBHabilitada(directorioActual);
	//retardoMemoria = configObtenerRetardoMemoria(directorioActual);

}*/


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

	RETARDO_MEMORIA =config_get_long_value(archConfig,"RETARDO_MEMORIA");

	CANTIDAD_MARCOS = config_get_long_value(archConfig,"CANTIDAD_MARCOS");

	TAMANIO_MARCO = config_get_long_value(archConfig,"TAMANIO_MARCO");

	MAXIMO_MARCOS_POR_PROCESO = config_get_long_value(archConfig,"MAXIMO_MARCOS_POR_PROCESO");
	/*
	ENTRADAS_TLB = config_get_long_value(archConfig,"ENTRADAS_TLB");
	strcpy(TLB_HABILITADA,config_get_string_value(archConfig,"TLB_HABILITADA"));
	strcpy(ALGORITMO_REEMPLAZO,config_get_string_value(archConfig,"ALGORITMO_REEMPLAZO"));

*/
	if(RETARDO_MEMORIA == 0 || PUERTO_SWAP == 0 || IP_SWAP == NULL ){
		retorno = -1;
	}
	return retorno;
}

void configurarSockets(){
	//se conecta con el swap que tiene un servidor escuchando
	socketSwap = socketCrearCliente(PUERTO_SWAP,IP_SWAP,"Memoria","Swap");
	socketServidor = socketCrearServidor(PUERTO_ESCUCHA,"Memoria");
	if (socketServidor > 0){
		socketCpu = socketAceptarConexion(socketServidor,"Memoria","CPU");
	}
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

static t_tablaPaginasProceso * nodoTablaPaginaProceso_create(int numeroMarco, int numeroPagina) {
	t_tablaPaginasProceso * new = malloc(sizeof(t_tablaPaginasProceso));

	new->numeroMarco = numeroMarco;
	new->numeroPagina = numeroPagina;

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
		list_add(nodoTablaPaginas->listaPaginas, nodoTablaPaginaProceso_create(1000000, i));
		free(nodoPaginasProceso);
	}
	list_add(listaTablasPaginas, nodoTablaPaginas);
}

void assert_valorTLB(t_tlb * nodoTLB, int processID, int numeroPagina, int fueModificado, int marco) {
	processID = nodoTLB->processID;
	numeroPagina = nodoTLB->numeroPagina;
	fueModificado = nodoTLB->fueModificado;
	marco = nodoTLB->marco;
}

int buscarEnTLB(int processID) {
	int pid;
	int numeroPagina;
	int fueModificado;
	int marco;

	int devolverValorDeTLB(t_tlb * nodo) {
		return (nodo->processID == processID);
	}

	t_tlb * nodoTLB = malloc(sizeof(t_tlb));

	nodoTLB = NULL;

	nodoTLB = list_find(listaTLB, (void *) devolverValorDeTLB);

	if (nodoTLB != NULL) {
		return nodoTLB->marco;
	}
	return 0;
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

	if (nodoTablasPaginaProceso->numeroMarco < 1000000) {
		return (nodoTablasPaginaProceso);
	}

	return NULL;
}

static t_tablasPaginas * buscarTablaPaginas(int processID, int numeroPagina){
	t_list * listaPagina;
	int marco;

	int devolverTablaProceso(t_tablasPaginas * nodo){
		return (nodo->processID == processID);
	}

	t_tablasPaginas * nodoTablaDeProceso = NULL;
	nodoTablaDeProceso = list_find(listaTablasPaginas, (void *) devolverTablaProceso);
	//assert_valorTablaPag(nodoTablasPagina,&listaPagina);

	/*int devolverPaginasProceso(t_tablaPaginasProceso * nodo){
		return (nodo->numeroPagina == numeroPagina);
	}

	t_tablaPaginasProceso * nodoTablasPaginaProceso = NULL;
	nodoTablasPaginaProceso = list_find(nodoTablasPagina->listaPaginas, (void *) devolverPaginasProceso);*/

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

	int contarMarcosAsignados(t_tablasPaginas * nodo) {
		return (nodo->processID == processID);
	}
	int valor = list_count_satisfying(listaTablasPaginas, (void*) contarMarcosAsignados);
	return valor;
}

int obtenerUnMarcoLibre(t_list * lista) {

	int marcoLibre(t_marco * nodo) {
		return (nodo->presencia == 0);
	}
	t_marco * nodoMarco = malloc(sizeof(t_marco));
	nodoMarco = (list_find(lista, (void*) marcoLibre));

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

	resultadoBusqueda = buscarEnTLB(nodoInstruccion->pid);
	if (resultadoBusqueda == 0) {
		//numeroPagina = devolverParteUsableInt(nodoInstruccion->instruccion, 5);
		tablaDeProceso = buscarTablaPaginas(nodoInstruccion->pid, numeroPagina);
		nodoPagina = obtenerPagina(numeroPagina, tablaDeProceso);
		resultadoBusqueda = nodoPagina->numeroMarco;
		if (resultadoBusqueda < 0) {
			//TODO pedir a swap la pagina.
			/*if (send(socketSwap, ____, sizeof(t_nodo_mem)) > 0) { TODO envia el numero de pagina y el processID para que el swap lo traiga de vuelta.
				if (recv(socketSwap, valor, sizeof(____)) > 0) {
					puts(valor);
				}
			} */
			if (cantidadMarcosAsignados(nodoInstruccion->pid) < MAXIMO_MARCOS_POR_PROCESO) {
				indiceMarco = obtenerUnMarcoLibre(listaMarco);
				if (indiceMarco < 0) {
					//TODO finalizar proceso.
				} else {
					resultadoBusqueda = indiceMarco;
					//TODO Terminar de leer o escribir.
				}
			} else if (cantidadMarcosAsignados(nodoInstruccion->pid) == MAXIMO_MARCOS_POR_PROCESO) {
				//TODO ejecutar algoritmo de reemplazo FIFO.
				//TODO eliminar numero de marco en la pagina victima
				//enviarPaginaASwap(nodoInstruccion->pid, texto, pagina);
			}
			if (strlen(texto) > 0) {
				// comando escribir.
				escribirMarco(nodoInstruccion->pid, marco->numeroMarco, texto, 1, numeroPagina);
			}
			actualizarTablaPaginas(indiceMarco, nodoInstruccion->pid, numeroPagina);
		}
	}
	marco = list_find(listaMarco, (void*) devolverValor);
	return marco;
}

void actualizarTablaPaginas(int indiceMarco, int processID, int numeroPagina) {
	t_tablasPaginas * tablaDeProceso;
	t_tablaPaginasProceso * nodoPagina;

	tablaDeProceso = buscarTablaPaginas(processID, numeroPagina);

	nodoPagina = obtenerPagina(numeroPagina, tablaDeProceso);

	nodoPagina->numeroMarco = indiceMarco;
}

int interpretarLinea(t_nodo_mem * nodoInstruccion) {

	int resultadoBusqueda;
    char * valor;
    char * texto;
    int processID;
    t_marco * marco = malloc(sizeof(t_marco));
    if (esElComando(nodoInstruccion->instruccion, "iniciar")) {
    	int cantidadPaginasProceso;
    	cantidadPaginasProceso = devolverParteUsableInt(nodoInstruccion->instruccion, 8);
    	inicializarTablaDePaginas(cantidadPaginasProceso, nodoInstruccion->pid);
		strcpy(respuesta,"iniciar");
		return 1;
	} else if (esElComando(nodoInstruccion->instruccion, "leer")) {
		resultadoBusqueda = buscarEnTLB(nodoInstruccion->pid);
		marco = accederAPaginaCiclicamente(nodoInstruccion, devolverParteUsableInt(nodoInstruccion->instruccion, 5));
		puts(marco->valor);
		strcpy(respuesta,"AFX");
	} else if (esElComando(nodoInstruccion->instruccion, "escribir")) {
		int pagina;
		char * paginaChar;
		paginaChar = string_substring(nodoInstruccion->instruccion, 9, 1);
		pagina = atoi(paginaChar);
		if (pagina > 9) {
			texto = malloc(sizeof(devolverParteUsable(nodoInstruccion->instruccion, 11)) + 1);
			strcpy(texto, devolverParteUsable(nodoInstruccion->instruccion, 11));
			strcat(texto, "\0");
		}else{
			texto = malloc(sizeof(devolverParteUsable(nodoInstruccion->instruccion, 10)) + 1);
			strcpy(texto, devolverParteUsable(nodoInstruccion->instruccion, 10));
			strcat(texto, "\0");
		}
		marco = accederAPaginaCiclicamente(nodoInstruccion, pagina, texto);
		//resultadoBusqueda = buscarTablaPaginas(nodoInstruccion->pid, pagina, &listaTablaProceso);
		printf("%d",nodoInstruccion->pid);
		printf("%d",pagina);
		puts(texto);
		/*if (enviarASwap > 0) {

		}*/
	} else if (esElComando(nodoInstruccion->instruccion, "finalizar")) {
		int procesoActual(t_tablasPaginas * nodo) {
			return (nodo->processID == nodoInstruccion->pid);
		}
		list_remove_by_condition(listaTablasPaginas, (void *) procesoActual);
	} else {
		strcpy(respuesta,"error");
		perror("comando invalido");
	}

    return 1;
}

int enviarPaginaASwap(int processID, char * valor, int numeroPagina) {
	t_envioPaginaSwap * nodoSwap = malloc(sizeof(t_envioPaginaSwap));

	nodoSwap->numeroPagina = numeroPagina;
	nodoSwap->processID = processID;
	nodoSwap->valor = malloc(sizeof(valor));
	strcpy(nodoSwap->valor, valor);

	socketEnviarMensaje(socketSwap, nodoSwap, sizeof(t_envioPaginaSwap)); //TODO usar send y recv
}


int escribirMarco(int processID, int marco, char * texto, int presencia, int numeroPagina){
	if (!string_equals_ignore_case(texto, "NULL")) {
		list_replace(listaMarco, marco, marco_create(processID, texto, marco, presencia, numeroPagina, 0));
		return 0;
	} else {
		list_replace(listaMarco, marco, marco_create(processID, texto, marco, presencia, numeroPagina, 1));
		return 1;
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

