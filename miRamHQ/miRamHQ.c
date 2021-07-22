#include "miRamHQ.h"

int main(void) {
	logger_pruebas = log_create("PRUEBAS.txt", "Pruebas", 0, LOG_LEVEL_DEBUG); // TODO borrar despues
	sem_init(&terminar_prog,0,0);
	signal(SIGUSR1, realizar_dump_memoria);
	signal(SIGUSR2, signal_compactar_memoria);
	signal(SIGINT, enviar_senial);


	config = config_create("Mi-RAM-HQ.config");
	memoria_principal = crear_memoria(config);
	tablas_segmentos = list_create();
	lista_de_tcb_reconstruidos = list_create();
	cola_peticiones = queue_create();

	indice_nuevo_tripulante = 1;
	indice_nueva_patota = 1;

	//pthread_create(&thread_mapa, NULL, (void*)crear_mapa, NULL);
	//pthread_create(&thread_servidor, NULL, (void*)crear_servidor, NULL);
	//pthread_join(thread_mapa,NULL);
	//pthread_join(thread_servidor,NULL);
	crear_mapa();
	pthread_create(&hilo_server,NULL,(void*) crear_servidor,NULL);
	sem_wait(&terminar_prog);
	pthread_cancel(thread1);
	pthread_cancel(thread2);
	pthread_cancel(thread3);
	pthread_cancel(thread4);
	pthread_cancel(hilo_server);
	liberar_memoria();
}

void liberar_memoria(){
	log_info(logger,"Terminando Programa ...");
	free(memoria_principal->algoritmo_reemplazo);
	free(memoria_principal->bloque_memoria_principal);
	free(memoria_principal->criterio_fit);
	free(memoria_principal->path_swap);
	sem_destroy(&sem_peticiones);
	pthread_mutex_destroy(&mutex_comunicacion);
	//TODO: HABRIA Q HACER ESTO DE ABAJO, PERO TIRA DOUBLE FREE.
	//list_destroy_and_destroy_elements(lista_de_tcb_reconstruidos,free);
	log_info(logger,"ANTES DE LIBERAR TABLAS EXTRANIAS");
	liberar_tablas_extranias();
	log_info(logger,"DSPS DE LIBERAR TABLAS EXTRANIAS");
	free(memoria_principal->tipo_memoria);
	free(memoria_principal);
	log_info(logger,"antes de romper el logger");
	log_destroy(logger);
	log_destroy(logger_pruebas);
	config_destroy(config);
	nivel_gui_terminar();
	//destruir_colas_y_listas();
}

void enviar_senial(int n){
	sem_post(&terminar_prog);
}


void liberar_tablas_extranias(){
	log_info(logger,"libero tablas extranias %d", list_size(tablas_segmentos));
	//list_iterate(tablas_segmentos,liberar_tablas_patotas);
	//list_destroy_and_destroy_elements(tablas_segmentos,free);
	list_destroy(tablas_segmentos);

}

/*
void liberar_tablas_patotas(void* value){
	log_info(logger,"libero tablas patotas");
	t_tabla_patota* tabla_patotas = value;
	list_destroy(tabla_patotas->filas);
	if (strcmp(memoria_principal->tipo_memoria, "PAGINACION") == 0){
		log_info(logger,"entre al if");
		list_iterate(tabla_patotas->datos_tripulantes,liberar_listas_en_datos_tripulantes);
		//list_destroy_and_destroy_elements(tabla_patotas->datos_tripulantes);
		list_destroy(tabla_patotas->datos_pcb->paginas_que_ocupa);
		list_destroy_and_destroy_elements(tabla_patotas->datos_tareas->paginas_que_ocupa,free);
		log_info(logger,"destrui elements");
		free(tabla_patotas->datos_pcb);
		free(tabla_patotas->datos_tareas);
	}
}


void liberar_listas_en_datos_tripulantes(void* value){
	t_list* paginas_que_ocupa = value;
	list_destroy(paginas_que_ocupa);
}

*/


