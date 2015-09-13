#ifndef SOCKETCLIENTE_H_
#define SOCKETCLIENTE_H_

int socketCrearCliente(char * PUERTO, char * IP);
int socketEnviarMensaje(int serverSocket, char * mensaje);
void socketRecibirMensaje(int serverSocket, char * mensaje);
void socketCerrarSocket(int serverSocket);


#endif /* SOCKETCLIENTE_H_ */
