#ifndef CONEXIONES_H_
#define CONEXIONES_H_

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include<commons/collections/queue.h>
#include<commons/config.h>
#include<commons/memory.h>
#include<commons/temporal.h>
#include<string.h>
#include<pthread.h>
#include<semaphore.h>
#include"Includes/token.h"
#include"Includes/memoria.h"
#include"Includes/archivos.h"

typedef enum
{
	MENSAJE,
	PAQUETE
}op_code;

typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

typedef enum {
	GENERAR_OXIGENO = 1,
	CONSUMIR_OXIGENO = 2,
	GENERAR_COMIDA = 3,
	CONSUMIR_COMIDA = 4,
	GENERAR_BASURA = 5,
	DESCARTAR_BASURA = 6,
	INICIAR_PLANIFICACION = 7,
	PAUSAR_PLANIFICACION = 8,
	INICIAR_PATOTA = 9,
	MOVER = 10,
	INICIAR_TRIPULANTES = 11,
	ENVIAR_PROXIMA_TAREA = 12,
	RECIBIR_UBICACION_TRIPULANTE = 13,
	EXPULSAR_TRIPULANTE = 14,
	OBTENER_BITACORA = 15,
	COMENZAR_PROTOCOLO_FSCK = 16,
	ACTUALIZAR_ESTADO = 17
}t_mensaje;

typedef enum
{
	ARRIBA = 21,
	ABAJO = 22,
	DERECHA = 23,
	IZQUIERDA = 24,
}t_movimiento;

t_log* logger;
t_config* config;
pthread_t thread_servidor;
pthread_t thread_mapa;


char* ip_mi_ram_hq;
char* puerto_mi_ram_hq;
char* logger_mi_ram_hq;
pthread_mutex_t mutex_comunicacion;

void crear_servidor();
int request(int*);
void iterator(char*);
void* recibir_buffer(int*, int);
int iniciar_servidor(char*,char*);
int esperar_cliente(int);
t_list* recibir_paquete(int);
void recibir_mensaje(int);
int recibir_operacion(int);
void* instruccion(t_list*, int);
void crear_mapa(void);

int obtener_codigo_de_operacion(char*);
void verificar_memoria_disponible(t_list*, int);
void iniciar_patota(t_list*);

void mover_tripulante(t_list*);
void actualizar_estado_tcb(t_list*);
void expulsar_tripulante(t_list*);
void enviar_proxima_tarea(t_list* , int);

void signal_compactar_memoria(int);
void realizar_dump_memoria(int);
char* conseguir_tareas_del_path(char*);

t_paquete* crear_paquete(void);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void eliminar_paquete(t_paquete* paquete);

pthread_t thread4;
pthread_t thread3;
pthread_t thread2;
pthread_t thread1;
pthread_t thread_peticiones;

sem_t sem_peticiones;
pthread_mutex_t mutex_encolar_peticion;
pthread_mutex_t mutex_desencolar_peticion;
t_queue* cola_peticiones;

void encolar_peticion( t_list*, t_queue*);
t_list* desencolar_peticion(t_queue*);

#endif /* CONEXIONES_H_ */
