#include "iMongoStore.h"
#include "utils.h"

t_paquete* crear_paquete(void) {
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = PAQUETE; // @suppress("Symbol is not resolved")
	crear_buffer(paquete);
	return paquete;
}


void enviar_paquete_por_conexion(t_paquete* paquete, char* ip, char* puerto){
	int un_socket = crear_conexion(ip, puerto);
	enviar_paquete(paquete, un_socket);
	liberar_conexion(un_socket);
}

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio) {
	paquete->buffer->stream = realloc(paquete->buffer->stream,
			paquete->buffer->size + tamanio + sizeof(int));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio,
			sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor,
			tamanio);

	paquete->buffer->size += tamanio + sizeof(int);
}


void enviar_paquete(t_paquete* paquete, int socket_cliente) {
	int bytes = paquete->buffer->size + 2 * sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
}

void crear_buffer(t_paquete* paquete) {
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

void eliminar_paquete(t_paquete* paquete) {
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

void* serializar_paquete(t_paquete* paquete, int bytes) {
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream,
			paquete->buffer->size);
	desplazamiento += paquete->buffer->size;

	return magic;
}


void liberar_conexion(int socket_cliente) {
	close(socket_cliente);
}


void iniciar_config_ims(){
	config = config_create("i-Mongo-Store.config");
	if (config == NULL){
		printf("No fue posible generar la configuración.\n");
		exit(2);
	}
	mapear_config_ims();//En la estructura
}


void mapear_config_ims(){
	logger_i_mongo_store = config_get_string_value(config, "LOG_FILE");
	logger = log_create(logger_i_mongo_store, "iMongoStore", 1, LOG_LEVEL_DEBUG);
	ip_i_mongo_store = config_get_string_value(config, "IP_I_MONGO_STORE");
	puerto_i_mongo_store = config_get_string_value(config, "PUERTO_I_MONGO_STORE");
	i_mongo_store.punto_montaje = config_get_string_value(config, "PUNTO_MONTAJE");
	i_mongo_store.tiempo_sincronizacion = config_get_int_value(config, "TIEMPO_SINCRONIZACION");
	i_mongo_store.posiciones_sabotaje=cargar_posiciones(config_get_string_value(config,"POSICIONES_SABOTAJE"));
	i_mongo_store.ip_discordiador=config_get_string_value(config,"IP_DISCORDIADOR");
	i_mongo_store.puerto_discordiador=config_get_string_value(config,"PUERTO_DISCORDIADOR");
}

/**CONEXIONES A SERVER**/

int crear_conexion(char *ip, char* puerto) {
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	int socket_cliente = socket(server_info->ai_family,
			server_info->ai_socktype, server_info->ai_protocol);

	if (connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen)
			== -1)
		printf("error");

	freeaddrinfo(server_info);

	return socket_cliente;
}

void enviar_mensaje(char* mensaje, int socket_cliente) {
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = MENSAJE; // @suppress("Symbol is not resolved")
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2 * sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}

void liberar_conexiones(int un_socket_cliente) {
	close(un_socket_cliente);
}


t_queue* cargar_posiciones(char* lista_posiciones){

	t_queue* posiciones_sab = queue_create();

	for(int i = 0; i < token_count(lista_posiciones,','); i++){
		char* pos = get_token_at(lista_posiciones,',',i);
		queue_push(posiciones_sab, pos);
	}

	return posiciones_sab;
}

void crear_servidor(){

	sem_init(&sem_peticiones, 0, 0);

	log_info(logger,puerto_i_mongo_store);
	int server_fd = iniciar_servidor(ip_i_mongo_store, puerto_i_mongo_store);
	//while (1) {
		log_info(logger, "Servidor listo para recibir al cliente");
		int cliente_fd = esperar_cliente(server_fd);
		if (cliente_fd != -1) {
			while(1){
				pthread_create(&thread, NULL, (void*) request, &cliente_fd);
				sem_wait(&sem_peticiones);
				cliente_fd = esperar_cliente(server_fd);
			}
		}
}

int iniciar_servidor(char* ip, char* puerto)
{
	int socket_servidor;

    struct addrinfo hints, *servinfo, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(ip, puerto, &hints, &servinfo);

    for (p=servinfo; p != NULL; p = p->ai_next){
        if ((socket_servidor = socket(p->ai_family, p->ai_socktype,
        		p->ai_protocol)) == -1)
            continue;

        if (bind(socket_servidor, p->ai_addr, p->ai_addrlen) == -1) {
            close(socket_servidor);
            continue;
        }
        break;
    }

	listen(socket_servidor, SOMAXCONN);

    freeaddrinfo(servinfo);

    log_trace(logger, "Listo para escuchar a mi cliente");

    return socket_servidor;
}

int esperar_cliente(int socket_servidor)
{
	struct sockaddr_in dir_cliente;
	int tam_direccion = sizeof(struct sockaddr_in);

	int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);

	return socket_cliente;
}

void iterator(char* value){
		printf("%s\n", value);
}

int request(int* cliente_fd) {

	t_list* lista;
	//while(1){
		int cod_op = recibir_operacion(*cliente_fd);
		//

		switch (cod_op) {
		/*case MENSAJE: // @suppress("Symbol is not resolved")
			recibir_mensaje(*cliente_fd);
			break;*/
		case PAQUETE:
			lista = recibir_paquete(*cliente_fd);
			encolar_peticion(lista, cola_peticiones);
			sem_post(&sem_peticiones);
			instruccion(*cliente_fd);
			break;
		case -1:
			log_error(logger, "el cliente se desconecto. Terminando servidor");
			log_info(logger, "Servidor listo para recibir cliente");
			break;
		/*default:
			log_warning(logger,
					"Operacion desconocida. No quieras meter la pata");
			break;*/
		}
	//}
	return EXIT_SUCCESS;
}

int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) != 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}

void* recibir_buffer(int* size, int socket_cliente)
{
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void recibir_mensaje(int socket_cliente)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	log_info(logger, "Me llego el mensaje %s", buffer);
	free(buffer);
}

t_list* recibir_paquete(int socket_cliente)
{
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* valores = list_create();
	int tamanio;

	buffer = recibir_buffer(&size, socket_cliente);
	while(desplazamiento < size)
	{
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		char* valor = malloc(tamanio);
		memcpy(valor, buffer+desplazamiento, tamanio);
		desplazamiento+=tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
	return NULL;
}

void* instruccion(int socket_cliente) {

	t_list* lista = desencolar_peticion(cola_peticiones);
	uint32_t codigo_operacion = obtener_codigo_de_operacion( (char*) list_get(lista, 0));
	switch (codigo_operacion) {

	case INICIAR_TRIPULANTES:
		iniciar_patota(lista);
		break;

	case OBTENER_BITACORA :
		obtener_bitacora(lista);
		break;

	case EXPULSAR_TRIPULANTE:
	case ATENDER_SABOTAJE:
	case RESOLVER_SABOTAJE:
	case FINALIZAR :
	case MOVER :
		cargar_en_bitacora(lista);
		break;

	case COMENZAR :
		realizar_tarea(lista);
		cargar_en_bitacora(lista);
		break;

	case COMENZAR_PROTOCOLO_FSCK :
		list_destroy_and_destroy_elements(lista, free);
		protocolo_fsck();
		break;
	}
	return EXIT_SUCCESS;
}


void iniciar_patota(t_list *lista){
	int cant_tripulantes = atoi(list_get(lista, 1));

	for(int i = 0; i<cant_tripulantes ;i++){
		indice_nuevo_tripulante++;
		iniciar_un_tripulante(indice_nuevo_tripulante);
	}
	log_info(logger, "Inicie una patota con %d tripulantes.",cant_tripulantes );
	list_destroy_and_destroy_elements(lista,free);
}

void iniciar_un_tripulante(int id){
	char* tid = string_itoa(id);

	t_list* lista = list_create();
	list_add(lista, "INICIAR_TRIPULANTE");
	list_add(lista, tid);

	crear_archivo_bitacora(id);
	cargar_en_bitacora(lista);
}

void realizar_tarea(t_list* lista){
	char* operacion = (char*)list_get(lista,1);
	int mensaje = obtener_codigo_de_operacion(operacion);
	char* path = generar_path_metadata(mensaje);

	switch(mensaje){
		case GENERAR_OXIGENO:
		case GENERAR_COMIDA:
		case GENERAR_BASURA:
			cargar_recurso(mensaje, atoi(list_get(lista, 3)));
			log_info(logger,"Tripulante %s comenzo a %s",list_get(lista, 2), list_get(lista, 1));
		break;
		case CONSUMIR_OXIGENO:
		case CONSUMIR_COMIDA:
			descargar_recurso(mensaje, atoi(list_get(lista, 3)));
			log_info(logger,"Tripulante %s comenzo a %s",list_get(lista, 2), list_get(lista, 1));
		break;
		case DESCARTAR_BASURA:
			eliminar_archivo(path);
			log_info(logger,"Tripulante %s descarto basura",list_get(lista, 2));
		break;
		default:
		break;
	}
	free(path);
}

void obtener_bitacora(t_list* lista){
	uint32_t id = atoi((char*)list_get(lista, 1));
	log_info(logger, "Se quiere obtener la bitácora del tripulante %d", id);
	loggear_bitacora_del_tripulante(id);
	list_destroy_and_destroy_elements(lista, free);
}

int obtener_codigo_de_operacion(char* leido){
	if (strcmp(leido, "INICIAR_TRIPULANTES") == 0 ){
		return INICIAR_TRIPULANTES;
	} else if (strcmp(leido, "COMENZAR") == 0){
		return COMENZAR;
	} else if (strcmp(leido, "ATENDER_SABOTAJE") == 0){
		return ATENDER_SABOTAJE;
	} else if (strcmp(leido, "RESOLVER_SABOTAJE") == 0){
		return RESOLVER_SABOTAJE;
	} else if (strcmp(leido, "FINALIZAR") == 0){
		return FINALIZAR;
	} else if (strcmp(leido, "INICIAR_TRIPULANTE") == 0){
		return INICIAR_TRIPULANTE;
	} else if (strcmp(leido, "EXPULSAR_TRIPULANTE") == 0){
		return EXPULSAR_TRIPULANTE;
	} else if (strcmp(leido,"OBTENER_BITACORA") == 0){
		return OBTENER_BITACORA;
	} else if (strcmp(leido,"GENERAR_OXIGENO") == 0){
		return GENERAR_OXIGENO;
	} else if (strcmp(leido,"CONSUMIR_OXIGENO") == 0){
		return CONSUMIR_OXIGENO;
	} else if (strcmp(leido,"GENERAR_COMIDA") == 0){
		return GENERAR_COMIDA;
	} else if (strcmp(leido,"CONSUMIR_COMIDA") == 0){
		return CONSUMIR_COMIDA;
	} else if (strcmp(leido,"GENERAR_BASURA") == 0){
		return GENERAR_BASURA;
	} else if (strcmp(leido,"DESCARTAR_BASURA") == 0){
		return DESCARTAR_BASURA;
	} else if (strcmp(leido,"MOVER") == 0){
		return MOVER;
	} else if (strcmp(leido,"COMENZAR_PROTOCOLO_FSCK") == 0){
		return COMENZAR_PROTOCOLO_FSCK;
	} else if (strcmp(leido,"ARRIBA") == 0){
		return ARRIBA;
	} else if (strcmp(leido,"ABAJO") == 0){
		return ABAJO;
	} else if (strcmp(leido,"DERECHA") == 0){
		return DERECHA;
	} else if (strcmp(leido,"IZQUIERDA") == 0){
		return IZQUIERDA;

	} else return -1;
}

//----semaforos----------------------------

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

void dar_comienzo_a_la_inicializacion_de_semaforos_habidos(){
	for(int i = 0; i < 20; i++){
		pthread_mutex_init(&mutex_bitacora[i], NULL);
	}
	for(int i = 0; i < 3; i++){
		pthread_mutex_init(&mutex_recurso[i], NULL);
	}
}

void apagar_semaforos(){
	for(int i = 0; i < 20; i++){
		pthread_mutex_destroy(&mutex_bitacora[i]);
	}
	for(int i = 0; i < 3; i++){
		pthread_mutex_destroy(&mutex_recurso[i]);
	}
	sem_destroy(&sem_sleep);
	sem_destroy(&sem_sync);
	sem_destroy(&sem_peticiones);
}


