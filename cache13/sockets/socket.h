#ifndef SOCKET_H_
#define SOCKET_H_

int crear_socket_servidor(char * puerto);
int crearSocketCliente(char * puerto, char * ip, int serverSocket);
int crearSocketClienteSinReferencia(char * puerto, char * ip);


#endif /* SOCKET_H_ */
