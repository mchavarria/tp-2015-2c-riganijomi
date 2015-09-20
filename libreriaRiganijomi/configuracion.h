#ifndef CONFIGURACION_H_
#define CONFIGURACION_H_

char * configObtenerIpPlanificador(char * ruta);
char * configObtenerPuertoEscucha(char * ruta);
char * configObtenerPuertoMemoria(char * ruta);
char * configObtenerPuertoSwap(char * ruta);
char * configObtenerPuertoPlanificador(char * ruta);
char * configObtenerIpSwap(char * ruta);
char * configObtenerNombreArchivoSwap(char * ruta);
char * configObtenerMaxMarcosProceso(char * ruta);
char * configObtenerCantMarcos(char * ruta);
char * configObtenerTamanioMarco(char * ruta);
char * configObtenerEntradasTLB(char * ruta);
char * configObtenerTLBHabilitada(char * ruta);
char * configObtenerRetardoMemoria(char * ruta);
char * configObtenerPuertoADM(char * ruta);
char * configObtenerIpADM(char * ruta);
char * configObtenerCantidadHilos(char * ruta);

#endif /* CONFIGURACION_H_ */
