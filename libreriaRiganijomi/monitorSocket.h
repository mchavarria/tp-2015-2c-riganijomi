#ifndef MONITORSOCKET_H_
#define MONITORSOCKET_H_

void *monitorPrepararServidor(char * PUERTO,sem_t * hilo1Mutex, sem_t * hilo2Mutex, int * socket);
void monitorEliminarSocket(int serverSocket);

#endif /* MONITORSOCKET_H_ */


