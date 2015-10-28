#include "configuracion.h"

/*
 * Esta linea es la que necesita cada proceso antes de enviar la ruta
 * char directorioActual[1024];
 * getcwd(directorioActual, sizeof(directorioActual));
 * strcat(directorioActual, "/src/config.cfg")
 * luego invoca configObetner.....(ruta);
*/
char * configObtenerIpPlanificador(char * ruta) {
	t_config* config;
	config = config_create(ruta);
	char * IP;
	IP = config_get_string_value(config, "IP_PLANIFICADOR");
	return IP;
}

char * configObtenerPuertoEscucha(char * ruta) {
	t_config* config;
	config = config_create(ruta);
	char * PUERTO;
	PUERTO = config_get_string_value(config, "PUERTO_ESCUCHA");
	return PUERTO;
}
char * configObtenerPuertoSwap(char * ruta) {
	t_config* config;
	config = config_create(ruta);
	char * PUERTO_SWAP;
	PUERTO_SWAP = config_get_string_value(config, "PUERTO_SWAP");
	return PUERTO_SWAP;
}
char * configObtenerPuertoPlanificador(char * ruta) {
	t_config* config;
	config = config_create(ruta);
	char * PUERTO_PLANIFICADOR;
	PUERTO_PLANIFICADOR = config_get_string_value(config, "PUERTO_PLANIFICADOR");
	return PUERTO_PLANIFICADOR;
}
char * configObtenerPuertoMemoria(char * ruta) {
	t_config* config;
	config = config_create(ruta);
	char * PUERTO_MEMORIA;
	PUERTO_MEMORIA = config_get_string_value(config, "PUERTO_MEMORIA");
	return PUERTO_MEMORIA;
}
char * configObtenerIpSwap(char * ruta) {
	t_config* config;
	config = config_create(ruta);
	char * IP_SWAP;
	IP_SWAP = config_get_string_value(config, "IP_SWAP");
	return IP_SWAP;
}
char * configObtenerNombreArchivoSwap(char * ruta) {
	t_config* config;
	config = config_create(ruta);
	char * NOMBRE_ARCHIVO;
	NOMBRE_ARCHIVO = config_get_string_value(config, "NOMBRE_SWAP");
	return NOMBRE_ARCHIVO;
}
char * configObtenerCantPaginasSwap(char * ruta) {
	t_config* config;
	config = config_create(ruta);
	char * CANTIDAD_PAGINAS;
	CANTIDAD_PAGINAS = config_get_string_value(config, "CANTIDAD_PAGINAS");
	return CANTIDAD_PAGINAS;
}
char * configObtenerTamPaginasSwap(char * ruta) {
	t_config* config;
	config = config_create(ruta);
	char * TAMANIO_PAGINA;
	TAMANIO_PAGINA = config_get_string_value(config, "TAMANIO_PAGINA");
	return TAMANIO_PAGINA;
}
char * configObtenerRetardoSwap(char * ruta) {
	t_config* config;
	config = config_create(ruta);
	char * RETARDO_SWAP;
	RETARDO_SWAP = config_get_string_value(config, "RETARDO_SWAP");
	return RETARDO_SWAP;
}
char * configObtenerRetardoCompactacionSwap(char * ruta) {
	t_config* config;
	config = config_create(ruta);
	char * RETARDO_COMPACTACION;
	RETARDO_COMPACTACION = config_get_string_value(config, "RETARDO_COMPACTACION");
	return RETARDO_COMPACTACION;
}
char * configObtenerMaxMarcosProceso(char * ruta) {
	t_config* config;
	config = config_create(ruta);
	char * MAXIMO_MARCOS_POR_PROCESO;
	MAXIMO_MARCOS_POR_PROCESO = config_get_string_value(config, "MAXIMO_MARCOS_POR_PROCESO");
	return MAXIMO_MARCOS_POR_PROCESO;
}
char * configObtenerCantMarcos(char * ruta) {
	t_config* config;
	config = config_create(ruta);
	char * CANTIDAD_MARCOS;
	CANTIDAD_MARCOS = config_get_string_value(config, "CANTIDAD_MARCOS");
	return CANTIDAD_MARCOS;
}
char * configObtenerTamanioMarco(char * ruta) {
	t_config* config;
	config = config_create(ruta);
	char * TAMANIO_MARCO;
	TAMANIO_MARCO = config_get_string_value(config, "TAMANIO_MARCO");
	return TAMANIO_MARCO;
}
char * configObtenerEntradasTLB(char * ruta) {
	t_config* config;
	config = config_create(ruta);
	char * ENTRADAS_TLB;
	ENTRADAS_TLB = config_get_string_value(config, "ENTRADAS_TLB");
	return ENTRADAS_TLB;
}
char * configObtenerTLBHabilitada(char * ruta) {
	t_config* config;
	config = config_create(ruta);
	char * TLB_HABILITADA;
	TLB_HABILITADA = config_get_string_value(config, "TLB_HABILITADA");
	return TLB_HABILITADA;
}
char * configObtenerRetardoMemoria(char * ruta) {
	t_config* config;
	config = config_create(ruta);
	char * RET;
	RET = config_get_string_value(config, "RETARDO_MEMORIA");
	return RET;
}

int configObtenerCantidadHilos(char * ruta) {
	t_config* config;
	config = config_create(ruta);
	int CANTIDAD_HILOS;
	CANTIDAD_HILOS = config_get_int_value(config, "CANTIDAD_HILOS");
	return CANTIDAD_HILOS;
}
char * configObtenerIpADM(char * ruta) {
	t_config* config;
	config = config_create(ruta);
	char * NOMBRE_ARCHIVO;
	NOMBRE_ARCHIVO = config_get_string_value(config, "IP_ADM");
	return NOMBRE_ARCHIVO;
}
char * configObtenerPuertoADM(char * ruta) {
	t_config* config;
	config = config_create(ruta);
	char * PUERTO_ADM;
	PUERTO_ADM = config_get_string_value(config, "PUERTO_ADM");
	return PUERTO_ADM;
}
int configObtenerRetardoCPU(char * ruta) {
	t_config* config;
	config = config_create(ruta);
	int RETARDO;
	RETARDO = config_get_int_value(config, "RETARDO");
	return RETARDO;
}

int configObtenerQuantum(char * ruta) {
	t_config* config;
	config = config_create(ruta);
	char * QUANTUM;
	QUANTUM = config_get_int_value(config, "QUANTUM");
	return QUANTUM;
}
