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
	programa1[5] = "finalizar";

	char cfgFin[] ="/src/config.cfg";

	char *dir = getcwd(NULL, 0);

	char *directorioActual = malloc(strlen(dir)+strlen(cfgFin)+1);

	strcat(directorioActual,dir);
	strcat(directorioActual,cfgFin);

    int resultado = 1;

	puts(directorioActual);

	archConfig = malloc(sizeof(t_config));
	archivoLog = log_create("memoria.log.log", "Memoria", false, 2);//Eclipse
	archConfig = config_create("/home/utnso/ws/tp-2015-2c-riganijomi/memoria/src/config.cfg");
	resultado = levantarCfgInicial(archConfig);

	inicializarTLB();

	inicializarMarco();

	int i=0;
	for (i = 0; i < 5; i++) {
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
	int i = 0;
	t_marco * nodoMarco;
	//listaMarco = malloc(sizeof(t_marco) * CANTIDAD_MARCOS);
	for(i=0; i<CANTIDAD_MARCOS; i++) {
		nodoMarco = malloc(sizeof(t_marco));
		nodoMarco->numeroMarco = i;
		nodoMarco->processID = 0;
		//nodoMarco->valor = NULL;
		nodoMarco->presencia = 0;
		list_add(listaMarco, nodoMarco);
		free(nodoMarco);
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

int inicializarTablaDePaginas(int cantidadPaginas, int pid) {
	int j = 0;
	t_marco * nodoMarco = malloc(sizeof(t_marco));
	int cantidadMarcosDisponibles = 0;
	for (j = 0; j < CANTIDAD_MARCOS; j++) {
		nodoMarco = list_get(listaMarco, j);
		if (nodoMarco->presencia == 0) {
			cantidadMarcosDisponibles ++;
		}

	}
	if (cantidadMarcosDisponibles >= cantidadPaginas) {
		t_tablasPaginas * nodoTablaPaginas = malloc(sizeof(t_tablasPaginas));
		nodoTablaPaginas->processID = pid;
		nodoTablaPaginas->listaPaginas = malloc(sizeof(t_tablaPaginasProceso));
		int i = 0;
		t_tablaPaginasProceso * nodoPaginasProceso;
		for (i=0; i<cantidadPaginas; i++) {
			nodoPaginasProceso = malloc(sizeof(t_tablaPaginasProceso));
			nodoPaginasProceso->marco = 0;
			nodoPaginasProceso->numeroPagina = 0;
			list_add(nodoTablaPaginas->listaPaginas, nodoPaginasProceso);
			free(nodoPaginasProceso);
		}
		listaTablasPaginas = malloc(sizeof(nodoTablaPaginas));
		list_add(listaTablasPaginas, nodoTablaPaginas);
		return 0;
	}else{
		return 1;
	}
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

	assert_valorTLB(list_find(listaTLB, (void *) devolverValorDeTLB), &pid, &numeroPagina, &fueModificado, &marco);

	return marco;
}

void assert_valor(t_marco * nodoMarco, int processID, char * valor) {
	processID = nodoMarco->processID;
	valor = nodoMarco->valor;
}

void assert_valorTablaPag(t_tablasPaginas * nodoTablasPagina, t_list * listaPaginas) {
	listaPaginas = nodoTablasPagina->listaPaginas;
}

void assert_valorPagina(t_tablaPaginasProceso * nodoTablasPaginaProceso, int marco) {
	marco = nodoTablasPaginaProceso->marco;
}

int buscarTablaPaginas(int processID, int numeroPagina){
	t_list * listaPagina;
	int marco;

	int devolverTablaProceso(t_tablasPaginas * nodo){
		int valor = nodo->processID == processID;
		return (nodo->processID == processID);
	}

	t_tablasPaginas * nodoTablasPagina = NULL;
	nodoTablasPagina = list_find(listaTablasPaginas, (void *) devolverTablaProceso);
	//assert_valorTablaPag(nodoTablasPagina,&listaPagina);

	int devolverPaginasProceso(t_tablaPaginasProceso * nodo){
		int  valor = nodo->numeroPagina == numeroPagina;
		return (nodo->numeroPagina == numeroPagina);
	}

	t_tablaPaginasProceso * nodoTablasPaginaProceso = NULL;
	nodoTablasPaginaProceso = list_find(nodoTablasPagina->listaPaginas, (void *) devolverPaginasProceso);
	//assert_valorPagina(nodoTablasPaginaProceso, &marco);

	return marco;
}

/*
int buscarEnMarcos(int processID, int numeroPagina){

}
*/

int interpretarLinea(t_nodo_mem * nodoInstruccion) {

	int resultadoBusqueda;
	char * devolverValor(t_marco * nodo) {
		return (nodo->numeroMarco == resultadoBusqueda);
	}

    char * valor;
    char * texto;
    int processID;
    //respuesta = malloc(sizeof(char[30]));
    if (esElComando(nodoInstruccion->instruccion, "iniciar")) {
    	int cantidadPaginasProceso;
    	cantidadPaginasProceso = devolverParteUsableInt(nodoInstruccion->instruccion, 8);
    	inicializarTablaDePaginas(cantidadPaginasProceso, nodoInstruccion->pid);
		strcpy(respuesta,"iniciar");
		return 1;
	} else if (esElComando(nodoInstruccion->instruccion, "leer")) {
		resultadoBusqueda = buscarEnTLB(nodoInstruccion->pid);
		if (resultadoBusqueda < 0) {
			int numeroPagina;
			numeroPagina = devolverParteUsableInt(nodoInstruccion->instruccion, 5);
			resultadoBusqueda = buscarTablaPaginas(nodoInstruccion->pid, numeroPagina);
			if (resultadoBusqueda < 0) {
				//resultadoBusqueda = buscarEnMarcos(nodoInstruccion->pid);
			} else {
				assert_valor(list_find(listaMarco, (void*) devolverValor), &processID, &valor);
			}
		} else {
			assert_valor(list_find(listaMarco, (void*) devolverValor), &processID, &valor);
		}
		puts(valor);
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
		resultadoBusqueda = buscarTablaPaginas(nodoInstruccion->pid, pagina);
		printf("%d",nodoInstruccion->pid);
		printf("%d",pagina);
		puts(texto);
		escribirMarco(nodoInstruccion->pid, resultadoBusqueda, texto, 1);
	} else if (esElComando(nodoInstruccion->instruccion, "finalizar")) {
		strcpy(respuesta,"finalizar");
	} else {
		strcpy(respuesta,"error");
		perror("comando invalido");
	}

    return 0;
}

static t_marco * marco_create(int processID, char * valor, int numeroMarco,  int presencia) {
	t_marco *new = malloc(sizeof(t_marco));
    new->processID = processID;
    new->numeroMarco = numeroMarco;
    new->valor = malloc(sizeof(valor) + 1);
    new->presencia = presencia;
    strcpy(new->valor, valor);
    strcat(new->valor, "\0");
    return new;
}

void escribirMarco(int processID, int marco, char * texto, int presencia){
	list_replace(listaMarco, marco, marco_create(processID, texto, marco, presencia));
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

