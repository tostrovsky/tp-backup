#include "iMongoStore.h"

void rutina(int n){
	char* posicion;
	switch(n){

	case SIGINT :
		sem_post(&terminar_prog);
		break;
	case SIGUSR1 :
		posicion = queue_pop(i_mongo_store.posiciones_sabotaje);
		t_paquete * paquete = crear_paquete();
		agregar_a_paquete(paquete, "ATENDER_SABOTAJE", strlen("ATENDER,SABOTAJE") + 1);
		agregar_a_paquete(paquete, posicion, strlen(posicion)+1);
		//pthread_mutex_lock(&mutex_comunicacion);
		enviar_paquete_por_conexion(paquete, i_mongo_store.ip_discordiador, i_mongo_store.puerto_discordiador);
		//pthread_mutex_unlock(&mutex_comunicacion);
		eliminar_paquete(paquete);

		free(posicion);
		//sleep(60);
		//sem_post(&terminar_prog);
		break;
	default:
	break;
	}
}


int main(void){
	signal(SIGINT, rutina); //TODO cada vez q se pone ctrl+c se le manda al discordiador un mensaje
	signal(SIGUSR1, rutina);
	sem_init(&terminar_prog,0,0);

	//signal(SIGUSR2, zapallo);

	metadata_o = malloc(sizeof(metadata_recurso));
	metadata_b = malloc(sizeof(metadata_recurso));
	metadata_c = malloc(sizeof(metadata_recurso));
	metadata_o->blocks= string_new();
	metadata_o->md5 = malloc(33);
	metadata_b->blocks= string_new();
	metadata_b->md5 = malloc(33);
	metadata_c->blocks= string_new();
	metadata_c->md5 = malloc(33);

	iniciar_config_ims();
	metadatas_bitacora = dictionary_create();

	dar_comienzo_a_la_inicializacion_de_semaforos_habidos();

	int blocks_fd;

	if(!existe_FS()){
		log_info(logger,"Generando archivos...");
		blocks_fd = crear_todos_archivos();
		log_info(logger,"Archivos creados exitosamente");
	}else {
		blocks_fd = crear_todos_archivos();
		recuperar_archivo_si_y_solo_si_hay_que_recuperarlos(GENERAR_OXIGENO);
		recuperar_archivo_si_y_solo_si_hay_que_recuperarlos(GENERAR_COMIDA);
		recuperar_archivo_si_y_solo_si_hay_que_recuperarlos(GENERAR_BASURA);
		log_info(logger,"File System recuperado exitosamente");
	}


	indice_nuevo_tripulante = 0;
	cola_peticiones = queue_create();

	pthread_create(&hilo_server, NULL,(void*)crear_servidor, NULL);
	sem_wait(&terminar_prog);
	pthread_cancel(thread);
	pthread_cancel(hilo_server);
	terminar_programa(conexion_discordiador,logger, blocks_fd);
}



void terminar_programa(int una_conexion, t_log* logger, int blocks_fd){
	log_warning(logger, "Terminando Programa...");
	//sem_destroy(&sem_peticiones);
	free(metadata_o->blocks);
	free(metadata_o->md5);
	free(metadata_o);
	free(metadata_c->blocks);
	free(metadata_c->md5);
	free(metadata_c);
	free(metadata_b->blocks);
	free(metadata_b->md5);
	free(metadata_b);
	free(superbloque.bitmap->bitarray);
	bitarray_destroy(superbloque.bitmap);
	close(blocks_fd);
	free(copia_mapeo);
	queue_destroy_and_destroy_elements(i_mongo_store.posiciones_sabotaje, free);


	//TODO: ARREGLAR ESTO
	dictionary_iterator(metadatas_bitacora, liberar_metadata_t);
	dictionary_destroy(metadatas_bitacora);

	liberar_conexiones(una_conexion);
	log_destroy(logger);
	apagar_semaforos();
	/*
	sem_destroy(&sem_sleep);
	sem_destroy(&sem_sync);
	sem_destroy(&sem_peticiones);*/
	//pthread_cancel((pthread_t)hilo_server);
}

void liberar_metadata_t(char* key , void* value){
	metadata_bitacora* metadata_t = value;
	free(metadata_t->blocks);
	free(metadata_t);
}


