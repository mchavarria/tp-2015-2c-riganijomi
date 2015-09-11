#include <stdio.h>
#include "sockets/socket.h"
#include <unistd.h>
#include <stdlib.h>

int main () {
	int pid = fork();
	if (pid != 0){
		char mensaje[1024];
		crearSocketServidor("6691");
		printf("Llego la mierda\n");
		sleep(10);
	} else {
		//hijo
		sleep(10);
		printf("Va a crearse un socket cliente.\n");
		crearSocketCliente("6990","192.168.1.128");
	}
}
