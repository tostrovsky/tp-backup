/*
 * estructuras.h
 *
 *  Created on: 22 jun. 2021
 *      Author: utnso
 */

#ifndef INCLUDES_ESTRUCTURAS_H_
#define INCLUDES_ESTRUCTURAS_H_

#include<math.h>
#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<commons/log.h>
#include<commons/config.h>
#include<readline/readline.h>
#include<commons/collections/list.h>
#include<commons/collections/queue.h>
#include<pthread.h>
#include<semaphore.h>


typedef struct
{
	uint32_t x;
	uint32_t y;
}t_posicion;

typedef struct
{
	uint32_t id;
	char est;
	t_posicion posicion;
	uint32_t expulsado;
	uint32_t patota_id;
	pthread_t hilo;
	int quantum_disponible;
	int rafaga_faltante;
	int esta_ejecutando;
	int atiende_sabotaje;
	char estado_pre_sabotaje;
}t_tripulante;

typedef struct
{
	char* nombre;
	int cantidad;
	int pos_x;
	int pos_y;
	int tiempo;
}t_tarea;

typedef struct
{
	t_tripulante* tripulante;
	t_tarea* tarea;
}t_planificando;


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
	LISTAR_TRIPULANTES = 11,
	EXPULSAR_TRIPULANTE = 14,
	OBTENER_BITACORA = 15,
	COMENZAR_PROTOCOLO_FSCK = 16,
	ATENDER_SABOTAJE = 12,
}t_mensaje;

typedef enum
{
	ARRIBA = 21,
	ABAJO = 22,
	DERECHA = 23,
	IZQUIERDA = 24,
}t_movimiento;

int id_ultimo_tripulante;
int id_ultima_patota;
t_queue* cola_ready;
t_queue* cola_block;
t_queue* cola_emergency;
t_queue* cola_peticiones;
t_list* lista_tripulantes;
int id_primer_tripulante_de_ultima_patota;
int se_esta_planificando;
int planificacion_esta_pausada;
int cantidad_tripulantes_en_f;
int cantidad_tripulantes_en_z;
int hay_tripulantes_new;
int corre_sabotaje;
t_posicion posicion_sabotaje;

char* valor;
t_log* logger;
t_config* config;
char* logger_discordiador;
int   conexion_i_mongo_store;
int   conexion_mi_ram_hq;
char* ip_i_mongo_store;
char* ip_mi_ram_hq;
char* ip_discordiador;
char* puerto_discordiador;
char* puerto_i_mongo_store;
char* puerto_mi_ram_hq;
int   grado_mutitarea;
char* algoritmo;
int   quantum;
int   duracion_sabotaje;
double   retardo_ciclo_cpu;

t_tripulante crear_tripulante(uint32_t, char, t_posicion, uint32_t, uint32_t);
t_tarea* crear_tarea(char*, uint32_t, uint32_t, uint32_t, uint32_t);
int iniciar_servidor(char* ip, char* puerto);
int esperar_cliente(int socket_servidor);
void mapear_valores_del_config(t_config* config);

#endif /* INCLUDES_ESTRUCTURAS_H_ */
