#ifndef INTERPRETE_H_
#define INTERPRETE_H_

#include <commons/string.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/stat.h>
#include <commons/collections/dictionary.h>
#include <commons/config.h>
#include <unistd.h>
#include <commons/log.h>

int esElComando(char * linea, char * comando);
char* devolverParteUsable(char * linea, int desde);
int devolverParteUsableInt(char * linea, int desde);
int devolverIntInstruccion(char * linea, int desde);

#endif /* INTERPRETE_H_ */
