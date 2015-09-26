#include <commons/config.h>

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
	puts("despues de IP");
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
	char * PUERTO;

	PUERTO = config_get_string_value(config, "PUERTO_SWAP");
	return PUERTO;
}
char * configObtenerPuertoPlanificador(char * ruta) {
	t_config* config;
	config = config_create(ruta);
	char * PUERTO;

	PUERTO = config_get_string_value(config, "PUERTO_PLANIFICADOR");
	return PUERTO;
}
char * configObtenerPuertoMemoria(char * ruta) {
	t_config* config;
	config = config_create(ruta);
	char * PUERTO;

	PUERTO = config_get_string_value(config, "PUERTO_MEMORIA");
	return PUERTO;
}
char * configObtenerIpSwap(char * ruta) {
	t_config* config;
	config = config_create(ruta);
	char * IP;
	IP = config_get_string_value(config, "IP_SWAP");
	return IP;
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
	char * CANT;
	CANT = config_get_string_value(config, "MAXIMO_MARCOS_POR_PROCESO");
	return CANT;
}
char * configObtenerCantMarcos(char * ruta) {
	t_config* config;
	config = config_create(ruta);
	char * CANT;
	CANT = config_get_string_value(config, "CANTIDAD_MARCOS");
	return CANT;
}
char * configObtenerTamanioMarco(char * ruta) {
	t_config* config;
	config = config_create(ruta);
	char * TAM;
	TAM = config_get_string_value(config, "TAMANIO_MARCO");
	return TAM;
}
char * configObtenerEntradasTLB(char * ruta) {
	t_config* config;
	config = config_create(ruta);
	char * CANT;
	CANT = config_get_string_value(config, "ENTRADAS_TLB");
	return CANT;
}
char * configObtenerTLBHabilitada(char * ruta) {
	t_config* config;
	config = config_create(ruta);
	char * HAB;
	HAB = config_get_string_value(config, "TLB_HABILITADA");
	return HAB;
}
char * configObtenerRetardoMemoria(char * ruta) {
	t_config* config;
	config = config_create(ruta);
	char * RET;
	RET = config_get_string_value(config, "RETARDO_MEMORIA");
	return RET;
}

char * configObtenerCantidadHilos(char * ruta) {
	t_config* config;
	config = config_create(ruta);
	char * NOMBRE_ARCHIVO;
	NOMBRE_ARCHIVO = config_get_string_value(config, "CANTIDAD_HILOS");
	return NOMBRE_ARCHIVO;
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
	char * NOMBRE_ARCHIVO;
	NOMBRE_ARCHIVO = config_get_string_value(config, "PUERTO_ADM");
	return NOMBRE_ARCHIVO;
}
