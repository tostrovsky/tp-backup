#include "Discordiador.h"

int main(void)
{
	lista_tripulantes = list_create();
	cola_ready = queue_create();
	cola_block = queue_create();
	cola_emergency = queue_create();
	cola_peticiones = queue_create();
	id_ultimo_tripulante = 0;
	id_ultima_patota = 0;
	cantidad_tripulantes_en_f = 0;
	se_esta_planificando = 0;
	planificacion_esta_pausada = 0;
	cantidad_tripulantes_en_z = 0;
	corre_sabotaje = 0;


	config = leer_config();

	mapear_valores_del_config(config);

	inicializar_monitores();

	logger = iniciar_logger(logger_discordiador);

	pthread_create(&thread_servidor, NULL, (void*) crear_servidor, NULL);

	/*** funciones viejasy
	avisar_conexion_a_puerto(logger, puerto_i_mongo_store);
	avisar_conexion_a_puerto(logger, puerto_mi_ram_hq);
	conexion_i_mongo_store = crear_conexion(ip_i_mongo_store, puerto_i_mongo_store);
	conexion_mi_ram_hq = crear_conexion(ip_mi_ram_hq,puerto_mi_ram_hq);
	enviar_mensaje("Hola I-Mongo-Store, soy el Discordiador", conexion_i_mongo_store);
	enviar_mensaje("Hola Mi-Ram-HQ, soy el Discordiador", conexion_mi_ram_hq);
	//leer_consola(logger);
	****/
	realizar_operacion();

	pthread_cancel(thread_servidor);
	terminar_programa(logger, config);
}


t_log* iniciar_logger(char* logger_discordiador)
{
	t_log* logger = log_create(logger_discordiador,"Discordiador", true, LOG_LEVEL_INFO);
	log_info(logger, "Log De Discordiador Creado");
	return logger;
}

t_config* leer_config(void)
{
	t_config* config = config_create("./Discordiador.config");
	if (config == NULL){
		exit (1);
	}
	return config;
}

void terminar_programa(t_log* logger, t_config* config)
{
	log_info(logger,"Terminando Programa...");
	destruir_monitores();
	destruir_listas_y_colas();
	log_destroy(logger);
	config_destroy(config);
}


void avisar_conexion_a_puerto(t_log* logger, char* puerto) {
	char* mensaje = malloc(sizeof(char)*33);
	strcpy(mensaje, "Voy a conectarme al puerto ");
	strcat(mensaje, puerto);
	log_info(logger, mensaje);
	free(mensaje);
}

void encolar_peticion(t_list* peticion, t_queue* cola) {
	pthread_mutex_lock(&mutex_encolar_peticion);
	queue_push(cola, peticion);
	pthread_mutex_unlock(&mutex_encolar_peticion);
}

t_list* desencolar_peticion(t_queue* cola) {
	t_list* peticion;
	pthread_mutex_lock(&mutex_desencolar_peticion);
	peticion = (t_list*) queue_pop(cola);
	pthread_mutex_unlock(&mutex_desencolar_peticion);
	return peticion;
}



