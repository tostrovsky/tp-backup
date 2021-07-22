#ifndef CONEXIONES_H_
#define CONEXIONES_H_

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/string.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include<commons/collections/queue.h>
#include<commons/config.h>
#include<string.h>
#include"token.h"
#include<pthread.h>
#include<semaphore.h>
#include"bitacoras.h"

//// CONFIG ////


typedef struct {
	char* punto_montaje;
	uint32_t tiempo_sincronizacion;
	t_queue* posiciones_sabotaje;
	char* ip_discordiador;
	char* puerto_discordiador;

} t_imongostore;

t_imongostore i_mongo_store;
void mapear_config_ims();
t_queue* cargar_posiciones(char*);
pthread_t hilo_server;
sem_t terminar_prog;

//// CONFIG ////

typedef enum{
	MENSAJE,
	PAQUETE
}op_code;

typedef struct{
	int size;
	void* stream;
} t_buffer;

typedef struct{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;


typedef enum {
	GENERAR_OXIGENO = 0,
	GENERAR_COMIDA = 1,
	GENERAR_BASURA = 2,
	CONSUMIR_OXIGENO = 3,
	CONSUMIR_COMIDA = 4,
	DESCARTAR_BASURA = 5,
	COMENZAR = 7,
	FINALIZAR = 8,
	INICIAR_TRIPULANTES = 9,
	MOVER = 10,
	ATENDER_SABOTAJE = 11,
	RESOLVER_SABOTAJE = 12,
	INICIAR_TRIPULANTE = 13,
	EXPULSAR_TRIPULANTE = 14,
	OBTENER_BITACORA = 15,
	COMENZAR_PROTOCOLO_FSCK = 16,
}t_mensaje;

typedef enum
{
	ARRIBA = 21,
	ABAJO = 22,
	DERECHA = 23,
	IZQUIERDA = 24,
}t_movimiento;

t_mensaje recurso_files;

t_log* logger;
t_config* config;
pthread_t thread;
pthread_t thread_servidor;

int indice_nuevo_tripulante;
int indice_nueva_patota;

char* ip_mi_ram_hq;
char* puerto_mi_ram_hq;
char* logger_mi_ram_hq;
int conexion_discordiador;

void apagar_semaforos();
void crear_servidor();
void avisar_conexion_a_puerto(t_log* logger, char* puerto);
int crear_conexion(char *ip, char* puerto);
void enviar_mensaje(char* mensaje, int socket_cliente);
void liberar_conexiones(int un_socket_cliente);
void terminar_programa(int una_conexion,t_log* logger, int hola);
int request(int*);
void iterator(char* value);
void* recibir_buffer(int*, int);
int iniciar_servidor(char*,char*);
int esperar_cliente(int);
t_list* recibir_paquete(int);
void recibir_mensaje(int);
int recibir_operacion(int);
void* serializar_paquete(t_paquete* paquete, int bytes);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void liberar_conexion(int socket_cliente);
void enviar_paquete_por_conexion(t_paquete* paquete, char* ip, char* puerto);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
t_paquete* crear_paquete(void);
void crear_buffer(t_paquete* paquete);
void realizar_tarea(t_list* lista);
int obtener_codigo_de_operacion(char*);
void iniciar_patota(t_list*);
void iniciar_un_tripulante(int id);
void mover_tripulante(t_list*);
void expulsar_tripulante(t_list*);
void eliminar_paquete(t_paquete* paquete);
void obtener_bitacora(t_list* lista);

pthread_t thread;
pthread_t thread_peticiones;

sem_t sem_peticiones;
pthread_mutex_t mutex_encolar_peticion;
pthread_mutex_t mutex_desencolar_peticion;
pthread_mutex_t mutex_comunicacion;
t_queue* cola_peticiones;

void encolar_peticion( t_list*, t_queue*);
t_list* desencolar_peticion(t_queue*);
void* instruccion(int);

#endif /* CONEXIONES_H_ */
