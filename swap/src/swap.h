#ifndef SWAP_H_
#define SWAP_H_


typedef struct nodoLibre {
	int indice;
	int tamanio;
} t_nodoLibre;

typedef struct nodoProceso {
	int idProc;
	int indice;
	int tamanio;
} t_nodoProceso;

typedef struct nodoEspera {
	int idProc;
	int tamanio;
} t_nodoEspera;


void levantarCfgInicial();
void crearParticion();
void configurarSocketServer();
static t_nodoLibre *crearNodoLibre(int indice, int tamanio);
static t_nodoEspera *crearNodoEspera(int idProc, int cantPagProceso);
static t_nodoProceso *crearNodoProceso(int idProc, int indice, int cantPagProceso);

//Recibe un proceso, en caso de aceptarlo crea un nodoProceso y lo agrega a la lista
//En caso de rechazarlo nada jajaja
//En caso de estar fragmentando (SEMAFOOOROOO) lo pone en nodoEspera
//En todos los casos genera un MENSAJE de aviso.
void recibirProceso();

//cuando finaliza el proceso, lo elimina.
//En la lista de libres agrega el indice y el tamaño del nodo eliminado
void eliminarProceso(int idProc);


void leerPaginaProceso(int idProc, int pagina);
//Condicion para el list_find en nodos libres

t_list * listaLibres;
t_list * listaProcesos;
t_list * listaEspera;

#endif /* SWAP_H_ */




