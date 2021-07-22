#ifndef INCLUDES_SEMAFOROS_H_
#define INCLUDES_SEMAFOROS_H_

#include "estructuras.h"

sem_t sem_tripulantes[20];
sem_t sem_bloqueados[20];
sem_t sem_sabotaje[20];
sem_t sem_puede_realizar_operacion;
sem_t sem_grado_multiprocesamiento;
sem_t sem_hay_tripulantes_en_ready;
sem_t sem_hay_tripulantes_en_block;
sem_t sem_dispositivo_i_o;
sem_t sem_planificacion_pausada;
sem_t sem_sabotaje_resuelto;
sem_t sem_peticiones;

pthread_mutex_t mutex_id_tripulante;
pthread_mutex_t mutex_id_patota;
pthread_mutex_t mutex_get_t_tripulante;
pthread_mutex_t mutex_get_tripulante;
pthread_mutex_t mutex_add_tripulante;
pthread_mutex_t mutex_remove_tripulante;
pthread_mutex_t mutex_gtgi;
pthread_mutex_t mutex_encolar_tripulante;
pthread_mutex_t mutex_desencolar_tripulante;
pthread_mutex_t mutex_triuplantes_finished;
pthread_mutex_t se_puede_expulsar;
pthread_mutex_t mutex_comunicacion;
pthread_mutex_t mutex_get_semaforo;
pthread_mutex_t mutex_desencolar_peticion;
pthread_mutex_t mutex_encolar_peticion;

uint32_t buscar_indice_por_id(t_list*, uint32_t);

void inicializar_monitores();
void destruir_monitores();
void incrementar_id_ultimo_tripulante();
void destruir_listas_y_colas();
void incrementar_id_ultima_patota();
t_tripulante get_t_tripulante(int); // devuelve tripulante dado indice de lista
t_tripulante* get_tripulante(int); //devuelve puntero a tripulante dado indice de lista
void add_tripulante(t_tripulante*);
void remove_tripulante(int);
t_tripulante* get_tripulante_given_id(int);//devuelve el puntero a tripulante dado id
void incrementar_tripulantes_finished();
void encolar_tripulante( t_planificando*, t_queue* cola);
void encolar_peticion(t_list* peticion, t_queue* cola);
t_planificando* desencolar_tripulante(t_queue* cola);

#endif /* INCLUDES_SEMAFOROS_H_ */
