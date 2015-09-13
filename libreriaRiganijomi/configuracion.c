
#include <commons/collections/dictionary.h>
#include <commons/config.h>
#include <commons/string.h>

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
	IP = config_get_string_value(config, "IP");
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

	PUERTO = config_get_string_value(config, "PORT");
	return PUERTO;
}
char * configObtenerPuertoMemoria(char * ruta) {
	t_config* config;
	config = config_create(ruta);
	char * PUERTO;

	PUERTO = config_get_string_value(config, "PORT");
	return PUERTO;
}
char * configObtenerIpSwap(char * ruta) {
	t_config* config;
	config = config_create(ruta);
	char * IP;
	IP = config_get_string_value(config, "IP_SWAP");
	return IP;
}
