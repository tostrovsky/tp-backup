#ifndef UTILS_H_
#define UTILS_H_

#include "Includes/token.h"
//#include "Includes/estructuras.h"
#include "Includes/semaforos.h"
#include "Includes/movimientos.h"

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




pthread_t thread_servidor;
pthread_t thread;
int crear_conexion(char* ip, char* puerto);
void enviar_mensaje(char* mensaje, int socket_cliente);
void enviar_paquete_por_conexion(t_paquete* paquete, char* ip, char* puerto);
int enviar_paquete_por_conexion_especial(t_paquete* paquete, char* ip, char* puerto);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void liberar_conexion(int socket_cliente);
void eliminar_paquete(t_paquete* paquete);
void avisar_conexion_a_puerto(t_log* logger, char* puerto);
void liberar_conexiones(int un_socket, int otro_socket);
void* instruccion(int socket_cliente);
int request(int* cliente_fd);
void realizar_operacion();
void preparar_y_enviar_paquete(char* ip, char* puerto, int cod_operacion);
int obtener_codigo_de_operacion(char* leido);
int obtener_codigo(char* leido);
char* obtener_tipo_mensaje_string(int codigo);
void preparar_y_enviar_paquete_para_ambas_conexiones(int cod_operacion);
t_list* desencolar_peticion(t_queue* cola);

void iniciar_patota();
pthread_t hilo_iniciar_patota;
void expulsar_tripulante();
pthread_t hilo_expulsar_tripulante;

void listar_tripulantes();
t_paquete* crear_paquete(void);
t_paquete* crear_super_paquete(void);
t_paquete* armar_paquete_segun_codigo(t_paquete* paquete, int codigo);
t_paquete* armar_paquete_iniciar_patota(t_paquete* paquete);
t_paquete* armar_paquete_expulsar_tripulante(t_paquete* paquete);
t_paquete* armar_paquete_obtener_bitacora(t_paquete* paquete);
t_paquete* armar_paquete_enviar_proxima_tarea(t_paquete* paquete);
t_paquete* armar_paquete_comenzar_protocolo_fcsk(t_paquete* paquete);

void avisar_servidores_movimiento_del_tripulante(uint32_t, uint32_t);
void avisar_mi_ram_estado(uint32_t id, char estado) ;
void avisar_imongo_bitacora(char* operacion, uint32_t id, char* posicion);
void avisar_imongo_bitacora_tarea(char* operacion, uint32_t id, char* tarea, uint32_t cantidad);


//planificacion

pthread_t hilo_planificacion;
pthread_t hilo_sabotaje;
t_tarea* pedir_proxima_tarea(t_tripulante*);

void reanudar_planificacion();
void pausar_planificacion();
void wait_signal_pausar();
void wait_signal_pausar_planificando(t_planificando* planificando);
void wait_signal_pausar_durante_tarea(t_planificando* planificando);
void* iniciar_planificacion();
void* ejecutar_tripulante_fifo(void* planificando_void);
void* ejecutar_tripulante_rr(void* planificando_void);
void echar(t_planificando* planificando);
void* iniciar_planificacion_tareas_bloqueantes();
void poner_ultima_patota_en_ready();
void ejecutar_tarea(t_planificando* planificando);
void ejecutar_tarea_bloqueante(t_planificando* planificando);
void ejecutar_tarea_round_robin(t_planificando* planificando);
void replanifica(t_planificando* planificando);
void replanifica_por_bloqueo(t_planificando* planificando);
int todos_en_f();
void set_hilo(t_tripulante* tripulante, pthread_t hilo);
void poner_todos_los_tripulantes_en_ready();
int cola_de_ready_tiene_tripulantes();
int es_tarea_bloqueante(t_tarea* tarea);
int sigue_en_execute(t_tripulante* tripulante);
int fue_expulsado(t_tripulante* tripulante);
int tarea_not_null(t_tarea* tarea);
void setear_rafaga_faltante(t_planificando* planificando, t_posicion destino);


//sabotajes

double diagonal_hasta(t_tripulante* tripulante, t_posicion destino);
double distancia_hasta(t_tripulante* tripulante, t_posicion destino);
char* posicion_to_string(t_posicion posicion);
void* protocolo_anti_sabotaje();


//recibir paquetes
int recibir_operacion(int socket_cliente);
void recibir_mensaje(int socket_cliente);
t_list* recibir_paquete(int);

#endif /* UTILS_H_ */
