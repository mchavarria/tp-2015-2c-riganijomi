#include "memoria.h"

//para mensajes recibidos
char instruccion[20];
char respuesta[30];
int nbytes;


int main(int argc, char* argv[]) {
	RETARDO_MEMORIA=0;
	PUERTO_ESCUCHA=0;
	PUERTO_SWAP=0;

	char cfgFin[] ="/src/config.cfg";

	char *dir = getcwd(NULL, 0);

	char *directorioActual = malloc(strlen(dir)+strlen(cfgFin)+1);

	strcat(directorioActual,dir);
	strcat(directorioActual,cfgFin);

    int resultado = 1;

	puts(directorioActual);

	archConfig = malloc(sizeof(t_config));
	archivoLog = log_create("mem.log", "MEM", false, 2);
	archConfig = config_create(directorioActual);
	resultado = levantarCfgInicial(archConfig);

	if (resultado == -1 ){
		log_error(archivoLog,"MEM: Error leyendo del archivo de configuracion");
		return -1;
	}else{
		puts(IP_SWAP);
		log_info(archivoLog,"MEM: Archivo de configuracion levantado correctamente");
		configurarSockets();
		//sem_init(&sem_mem, 0, 0);
		//sem_init(&sem_sockets, 0, 1);
		//Inicia los parametros

		/*pthread_t hiloMonitorSockets;
		char *arg1 = "memoria";
		int r1;*/

		//Tratamiento de la señan enviada por el SO
		signal(SIGINT, rutina);
		signal(SIGUSR1, rutina);

		//r1 = pthread_create(&hiloMonitorSockets,NULL,monitorPrepararServidor(&sem_mem,&sem_sockets), (void *) arg1);
		for(;(socketCpu > 0);){
			t_nodo_mem * nodoInstruccion = malloc(sizeof(t_nodo_mem));
			//sem_wait(&sem_mem);
			nbytes = socketRecibirMensaje(socketCpu, nodoInstruccion,sizeof(t_nodo_mem));
			// tengo un mensaje de algun cliente
			if (nbytes <= 0) {
				// Error o conexion cerrada por el cliente
				if (nbytes == 0) {
					printf("servidor MEM: socket %d desconectado\n", socketCpu);
				} else {
					perror("recepcion error");
				//monitorEliminarSocket(socketCpu);
				}
			} else {
				// Hay dato para leer. Enviarlo a alguien
				if (socketSwap > 0){
					nbytes = socketEnviarMensaje(socketSwap, nodoInstruccion, sizeof(t_nodo_mem));
					if (nbytes <= 0) {
						if (nbytes == 0) {
							printf("servidor MEM: socket %d desconectado\n", socketSwap);
						} else {
							perror("envio error");
						}
					} else {
						//envio el mensaje.. recibo respuesta
						nbytes = socketRecibirMensaje(socketSwap, respuesta,sizeof(respuesta));
						//TODO falta hacer que interprete la respuesta.
						nbytes = socketEnviarMensaje(socketCpu, respuesta,sizeof(respuesta));
					}
					//sem_post(&sem_sockets);
				} else {
					//interpretarLinea(nodoInstruccion);
					//nbytes = socketEnviarMensaje(socketCpu, respuesta,sizeof(respuesta));
					perror("no hay swap");
				}
				//free(respuesta);
				free(nodoInstruccion);
			}
		}
	}

	free(directorioActual);
	return 1;
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


	if(RETARDO_MEMORIA == 0 || PUERTO_SWAP == 0 || PUERTO_ESCUCHA==0 || IP_SWAP == NULL ){
		retorno = -1;
	}
	return retorno;
}

void configurarSockets(){
	//se conecta con el swap que tiene un servidor escuchando
	socketSwap = socketCrearCliente(PUERTO_SWAP,IP_SWAP);
	socketServidor = socketCrearServidor(PUERTO_ESCUCHA);
	if (socketServidor > 0){
		socketCpu = socketAceptarConexion(socketServidor);
	}
}

void rutina (int n) {
	switch (n) {
		case SIGINT:
			printf("En tu cara, no salgo nada…\n");
		break;
		case SIGUSR1:
			printf("LLEGO SIGUSR1\n");
		break;
		case SIGUSR2:
			printf("LLEGO SIGUSR2\n");
		break;
	}
}


void interpretarLinea(t_nodo_mem * nodoInstruccion) {

    char * valor;
    //respuesta = malloc(sizeof(char[30]));
    if (esElComando(nodoInstruccion->instruccion, "iniciar")) {
		valor = devolverParteUsable(nodoInstruccion->instruccion, 8);
		strcpy(respuesta,"iniciar");
	} else if (esElComando(nodoInstruccion->instruccion, "leer")) {
		valor = devolverParteUsable(nodoInstruccion->instruccion, 5);
		strcpy(respuesta,"AFX");
	} else if (esElComando(nodoInstruccion->instruccion, "escribir")) {
		char * rta;
		rta = string_substring(nodoInstruccion->instruccion, 9, 1);
		valor = devolverParteUsable(nodoInstruccion->instruccion, 11);
		strcpy(respuesta,"escribir");
	} else if (esElComando(nodoInstruccion->instruccion, "finalizar")) {
		strcpy(respuesta,"finalizar");
	} else {
		strcpy(respuesta,"error");
		perror("comando invalido");
	}
}


