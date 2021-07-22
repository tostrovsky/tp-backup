/*----Funciones de Monitores
 **----------------------------
 */
#include "semaforos.h"

uint32_t buscar_indice_por_id(t_list* lista, uint32_t id) {
	uint32_t size = list_size(lista);
	int index = -1;
	t_tripulante* tripulante;
	for (int i = 0; i < size && index == -1; i++) {
		tripulante = get_tripulante(i);
		if (tripulante->id == id) {
			index = i;
		}
	}
	return index;
}

void inicializar_monitores() {
	sem_init(&sem_puede_realizar_operacion,0,0);
	sem_init(&sem_grado_multiprocesamiento, 0, grado_mutitarea);
	sem_init(&sem_planificacion_pausada, 0, 0);
	sem_init(&sem_dispositivo_i_o, 0, 1); //un dispositivo E/S
	sem_init(&sem_sabotaje_resuelto, 0, 0);
	sem_init(&sem_peticiones, 0, 0);
	pthread_mutex_init(&mutex_id_tripulante, NULL);
	pthread_mutex_init(&mutex_id_patota, NULL);
	pthread_mutex_init(&mutex_get_tripulante, NULL);
	pthread_mutex_init(&mutex_add_tripulante, NULL);
	pthread_mutex_init(&mutex_remove_tripulante, NULL);
	pthread_mutex_init(&mutex_encolar_tripulante, NULL);
	pthread_mutex_init(&mutex_desencolar_tripulante, NULL);
	pthread_mutex_init(&mutex_gtgi, NULL);
	pthread_mutex_init(&mutex_triuplantes_finished, NULL);
	pthread_mutex_init(&se_puede_expulsar, NULL);
	pthread_mutex_init(&mutex_comunicacion, NULL);
	pthread_mutex_init(&mutex_get_semaforo, NULL);
	pthread_mutex_init(&mutex_desencolar_peticion, NULL);
	pthread_mutex_init(&mutex_encolar_peticion,NULL);
}
void destruir_monitores() {
	pthread_mutex_destroy(&mutex_id_tripulante);
	pthread_mutex_destroy(&mutex_id_patota);
	pthread_mutex_destroy(&mutex_get_tripulante);
	pthread_mutex_destroy(&mutex_add_tripulante);
	pthread_mutex_destroy(&mutex_remove_tripulante);
	pthread_mutex_destroy(&mutex_gtgi);
	pthread_mutex_destroy(&mutex_desencolar_tripulante);
	pthread_mutex_destroy(&mutex_encolar_tripulante);
	pthread_mutex_destroy(&mutex_triuplantes_finished);
	pthread_mutex_destroy(&se_puede_expulsar); //debiera ser un sem binario iniciado en 0
	pthread_mutex_destroy(&mutex_comunicacion);
	pthread_mutex_destroy(&mutex_get_semaforo);
	pthread_mutex_destroy(&mutex_encolar_peticion);
	pthread_mutex_destroy(&mutex_desencolar_peticion);
	sem_destroy(&sem_grado_multiprocesamiento);
	sem_destroy(&sem_hay_tripulantes_en_ready);
	sem_destroy(&sem_puede_realizar_operacion);
	sem_destroy(&sem_planificacion_pausada);
	sem_destroy(&sem_dispositivo_i_o);
	sem_destroy(&sem_sabotaje_resuelto);
	sem_destroy(&sem_peticiones);
	for(int i = 0; i < 20; i++){
		sem_destroy(&sem_tripulantes[i]);
	}
	for(int i = 0; i < 20; i++){
		sem_destroy(&sem_bloqueados[i]);
	}
	for(int i = 0; i < 20; i++){
		sem_destroy(&sem_sabotaje[i]);
	}
}

void destruir_listas_y_colas(){
	list_destroy_and_destroy_elements(lista_tripulantes, free);
	queue_destroy_and_destroy_elements(cola_ready, free);
	queue_destroy_and_destroy_elements(cola_block, free);
	queue_destroy_and_destroy_elements(cola_emergency, free);
	queue_destroy_and_destroy_elements(cola_peticiones, free);
}

void incrementar_id_ultimo_tripulante() {
	pthread_mutex_lock(&mutex_id_tripulante);
	id_ultimo_tripulante = id_ultimo_tripulante + 1;
	pthread_mutex_unlock(&mutex_id_tripulante);
}
void incrementar_id_ultima_patota() {
	pthread_mutex_lock(&mutex_id_patota);
	id_ultima_patota = id_ultima_patota + 1;
	pthread_mutex_unlock(&mutex_id_patota);
}
t_tripulante get_t_tripulante(int i) {
	t_tripulante trip;
	pthread_mutex_lock(&mutex_get_t_tripulante);
	trip = *((t_tripulante*) list_get(lista_tripulantes, i));
	pthread_mutex_unlock(&mutex_get_t_tripulante);
	return trip;
}
t_tripulante* get_tripulante(int i) {
	t_tripulante* trip;
	pthread_mutex_lock(&mutex_get_tripulante);
	trip = (t_tripulante*) list_get(lista_tripulantes, i);
	pthread_mutex_unlock(&mutex_get_tripulante);
	return trip;
}
t_tripulante* get_tripulante_given_id(int i) {
	t_tripulante* trip;
	pthread_mutex_lock(&mutex_gtgi);
	trip = (t_tripulante*) list_get(lista_tripulantes,
	buscar_indice_por_id(lista_tripulantes, i));
	pthread_mutex_unlock(&mutex_gtgi);
	return trip;
}

void add_tripulante(t_tripulante *tripulante) {
	pthread_mutex_lock(&mutex_add_tripulante);
	list_add(lista_tripulantes, tripulante);
	pthread_mutex_unlock(&mutex_add_tripulante);
}

void remove_tripulante(int indice) {
	pthread_mutex_lock(&mutex_remove_tripulante);
	list_remove(lista_tripulantes, indice);
	pthread_mutex_unlock(&mutex_remove_tripulante);
}

void encolar_tripulante(t_planificando* planificando, t_queue* cola) {
	pthread_mutex_lock(&mutex_encolar_tripulante);
	queue_push(cola, planificando);
	pthread_mutex_unlock(&mutex_encolar_tripulante);
}
t_planificando* desencolar_tripulante(t_queue* cola) {
	t_planificando* planificando;
	pthread_mutex_lock(&mutex_desencolar_tripulante);
	planificando = (t_planificando*) queue_pop(cola);
	pthread_mutex_unlock(&mutex_desencolar_tripulante);
	return planificando;
}

void incrementar_tripulantes_finished(){
	pthread_mutex_lock(&mutex_triuplantes_finished);
	cantidad_tripulantes_en_f ++;
	pthread_mutex_unlock(&mutex_triuplantes_finished);
}
