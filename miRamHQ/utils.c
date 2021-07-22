#include "utils.h"


void crear_mapa(void) {

	int columnas, filas;
	nivel_gui_inicializar();
	nivel_gui_get_area_nivel(&columnas, &filas);
	mapa = nivel_crear("Mapa (Toca ctrl+C para finalizar)");
	nivel_gui_dibujar(mapa);
}
/*
void crear_servidor() {
	pthread_mutex_init(&mutex_comunicacion, NULL);
	logger_mi_ram_hq = config_get_string_value(config, "LOG_FILE");
	logger = log_create(logger_mi_ram_hq, "MiRamHQ", 0, LOG_LEVEL_DEBUG);
	ip_mi_ram_hq = config_get_string_value(config, "IP_MI_RAM_HQ");
	puerto_mi_ram_hq = config_get_string_value(config, "PUERTO_MI_RAM_HQ");


	sem_init(&sem_peticiones, 0, 0);

	int server_fd = iniciar_servidor(ip_mi_ram_hq, puerto_mi_ram_hq);
	while (1) {
		log_info(logger, "Servidor listo para recibir al cliente");
		int cliente_fd = esperar_cliente(server_fd);
		if (cliente_fd != -1) {
			while(1){
				log_info(logger, "voy a crear un hilo");
				pthread_create(&thread1, NULL, (void*) request, &cliente_fd);
				sem_wait(&sem_peticiones);
				cliente_fd = esperar_cliente(server_fd);
//				pthread_create(&thread2, NULL, (void*) request, &cliente_fd);
//				sem_wait(&sem_peticiones);
//				cliente_fd = esperar_cliente(server_fd);
//				pthread_create(&thread3, NULL, (void*) request, &cliente_fd);
//				sem_wait(&sem_peticiones);
//				cliente_fd = esperar_cliente(server_fd);
//				pthread_create(&thread4, NULL, (void*) request, &cliente_fd);
//				sem_wait(&sem_peticiones);
//				cliente_fd = esperar_cliente(server_fd);
			}
		}
	}
}*/

//void crear_servidor() {
//	pthread_mutex_init(&mutex_comunicacion, NULL);
//	logger_mi_ram_hq = config_get_string_value(config, "LOG_FILE");
//	logger = log_create(logger_mi_ram_hq, "MiRamHQ", 0, LOG_LEVEL_DEBUG);
//	ip_mi_ram_hq = config_get_string_value(config, "IP_MI_RAM_HQ");
//	puerto_mi_ram_hq = config_get_string_value(config, "PUERTO_MI_RAM_HQ");
//
//
//	sem_init(&sem_peticiones, 0, 0);
//
//	int server_fd = iniciar_servidor(ip_mi_ram_hq, puerto_mi_ram_hq);
//
//	log_info(logger, "Servidor listo para recibir al cliente");
//
//	while(1){
//		pthread_t thread1;
//		int cliente_fd = esperar_cliente(server_fd);
//		pthread_create(&thread1, NULL, (void*) request, &cliente_fd);
//		sem_wait(&sem_peticiones);
//	}
//}

void crear_servidor() {
	pthread_mutex_init(&mutex_comunicacion, NULL);
	logger_mi_ram_hq = config_get_string_value(config, "LOG_FILE");
	logger = log_create(logger_mi_ram_hq, "MiRamHQ", 0, LOG_LEVEL_DEBUG);
	ip_mi_ram_hq = config_get_string_value(config, "IP_MI_RAM_HQ");
	puerto_mi_ram_hq = config_get_string_value(config, "PUERTO_MI_RAM_HQ");

	sem_init(&sem_peticiones, 0, 0);

	int server_fd = iniciar_servidor(ip_mi_ram_hq, puerto_mi_ram_hq);
	log_info(logger, "Servidor listo para recibir al cliente");

	while(1){
		pthread_t thread1;
		pthread_create(&thread1, NULL, (void*) request, &server_fd);
		//sem_wait(&sem_peticiones);
	}
}

int request(int* server_fd) {

	int cliente_fd = esperar_cliente(*server_fd);
	log_info(logger,"ARGENTINA soquet %d", cliente_fd);
	t_list* lista = recibir_paquete(cliente_fd);
	//sem_post(&sem_peticiones);
	instruccion(lista, cliente_fd);

	close(cliente_fd);
	log_info(logger, "soy un hilo y mori\n");
	return EXIT_SUCCESS;
}

int iniciar_servidor(char* ip, char* puerto) {
	int socket_servidor;

	struct addrinfo hints, *servinfo, *p;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &servinfo);

	for (p = servinfo; p != NULL; p = p->ai_next) {
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

int esperar_cliente(int socket_servidor) {
	struct sockaddr_in dir_cliente;
	uint32_t tam_direccion = sizeof(struct sockaddr_in);

	int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);

	log_info(logger, "Se conecto un cliente! soquet %d", socket_cliente );
	return socket_cliente;
}

void iterator(char* value) {
	printf("%s\n", value);
}

//int request(int* cliente_fd) {
//	//pthread_mutex_lock(&mutex_comunicacion);
//	log_info(logger,"ARGENTINA soquet %d\n", *cliente_fd);
//	//pthread_t hilo;
//	t_list* lista;
//	//while(1){
//		int cod_op = recibir_operacion(*cliente_fd);
//		switch (cod_op) {
//		/*case MENSAJE: // @suppress("Symbol is not resolved")
//			recibir_mensaje(*cliente_fd);
//			break;*/
//		case PAQUETE:
//			lista = recibir_paquete(*cliente_fd);
//			//encolar_peticion(lista, cola_peticiones);
//			sem_post(&sem_peticiones);
//			instruccion(lista, *cliente_fd);
//			break;
//		case -1:
//			log_error(logger, "el cliente se desconecto. Terminando servidor");
//			log_info(logger, "Servidor listo para recibir cliente");
//			break;
//		/*default:pthread_mutex_lock(&mutex_comunicacion);
//			log_warning(logger,
//					"Operacion desconocida. No quieras meter la pata");
//			break;*/
//		}
//	//
//		//pthread_mutex_unlock(&mutex_comunicacion);
//	return EXIT_SUCCESS;
//}

int recibir_operacion(int socket_cliente) {
	int cod_op;
	if (recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) != 0)
		return cod_op;
	else {
		log_info(logger, "hola pipi");
		close(socket_cliente);
		return -1;
	}
}

void* recibir_buffer(int* size, int socket_cliente) {
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void recibir_mensaje(int socket_cliente) {
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	log_info(logger, "Me llego el mensaje %s", buffer);
	free(buffer);
}

t_list* recibir_paquete(int socket_cliente) {

	recibir_operacion(socket_cliente);
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* valores = list_create();
	int tamanio;

	buffer = recibir_buffer(&size, socket_cliente);
	while (desplazamiento < size) {
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento += sizeof(int);
		char* valor = malloc(tamanio);
		memcpy(valor, buffer + desplazamiento, tamanio);
		desplazamiento += tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
	return NULL;
}

void* instruccion(t_list* lista, int socket_cliente) {

	//t_list* lista = desencolar_peticion(cola_peticiones);
	uint32_t codigo_operacion = obtener_codigo_de_operacion(
			(char*) list_get(lista, 0));

	//pthread_mutex_lock(&mutex_comunicacion);
	switch (codigo_operacion) {
	case INICIAR_PATOTA:
		//pthread_mutex_lock(&mutex_comunicacion);
		verificar_memoria_disponible(lista, socket_cliente);
		//pthread_mutex_unlock(&mutex_comunicacion);
		break;

	case INICIAR_TRIPULANTES:
		iniciar_patota(lista);
		break;

	case EXPULSAR_TRIPULANTE:
		log_info(logger, "Me pidieron expulsar tripulante");
		expulsar_tripulante(lista);
		break;

	case MOVER:
		log_info(logger, "Me pidieron mover tripulante");
		//pthread_mutex_lock(&mutex_comunicacion);
		mover_tripulante(lista);
		//pthread_mutex_unlock(&mutex_comunicacion);
		break;

	case ACTUALIZAR_ESTADO:
		log_info(logger, "Me pidieron actualizar estado");
		//pthread_mutex_lock(&mutex_comunicacion);
		actualizar_estado_tcb(lista);
		//pthread_mutex_unlock(&mutex_comunicacion);
		break;

	case ENVIAR_PROXIMA_TAREA:
		log_info(logger, "Me pidieron una tarea");
		//pthread_mutex_lock(&mutex_comunicacion);
		enviar_proxima_tarea(lista, socket_cliente);
		//pthread_mutex_unlock(&mutex_comunicacion);
		break;
	}
	//pthread_mutex_unlock(&mutex_comunicacion);
	list_destroy_and_destroy_elements(lista, free);
	return EXIT_SUCCESS;
}

void signal_compactar_memoria(int sig) {
	if (strcmp(memoria_principal->tipo_memoria, "SEGMENTACION") == 0) {
		compactar_memoria();
		realizar_dump_memoria(sig);
	}
}

//ACTUALIZAR_ESTADO | TID | ESTADO
void actualizar_estado_tcb(t_list* lista) {
	uint32_t tid = atoi((char*)list_get(lista, 1));
	char nuevo_estado = ((char*)list_get(lista, 2))[0];

	log_warning(logger, "act est Para el tripulante %d su nuevo estado es %c", tid, nuevo_estado);

	tcb_t* tcb = get_tripulante_por_id(tid);
	char* la_vieja_confiable = string_tcb(tcb);
	log_info(logger,la_vieja_confiable);
	tcb->estado = nuevo_estado;
	log_info(logger,la_vieja_confiable);
	free(la_vieja_confiable);
	if (strcmp(memoria_principal->tipo_memoria, "PAGINACION") == 0) {
		actualizar_tripulante(tcb);
	}

	if (nuevo_estado == 'F') {
		expulsar_tripulante(lista);
	}
}


void verificar_memoria_disponible(t_list *lista, int cliente) {
	uint32_t cantidad_tripulantes;

	char* path = string_new();
	string_append(&path, "Tareas/");
	cantidad_tripulantes = atoi((char*) list_get(lista, 1));
	string_append(&path,(char*) list_get(lista, 2));

	char* tareas = conseguir_tareas_del_path(path);

	t_paquete* paquete = crear_paquete();

	if (alcanza_memoria(sizeof(pcb_t) + strlen(tareas)+1 + sizeof(tcb_t)*cantidad_tripulantes)) {

		agregar_a_paquete(paquete, "MEMORIA DISPONIBLE", strlen("MEMORIA DISPONIBLE") + 1);

		enviar_paquete(paquete, cliente);
		log_info(logger, "Hubo memoria disponible para crear las estructuras!");
		eliminar_paquete(paquete);
	}else{

		agregar_a_paquete(paquete, "SIN MEMORIA DISPONIBLE", strlen("SIN MEMORIA DISPONIBLE") + 1);

		enviar_paquete(paquete, cliente);
		log_error(logger, "No hubo memoria suficiente para responder el pedido.");
		eliminar_paquete(paquete);
	}
	//close(cliente);
	free(path);
	free(tareas);
}


void iniciar_patota(t_list *lista) {
	uint32_t cantidad_tripulantes;

	char* path = string_new();
	string_append(&path, "Tareas/");
	cantidad_tripulantes = atoi((char*) list_get(lista, 1));
	string_append(&path,(char*) list_get(lista, 2));

	char* tareas = conseguir_tareas_del_path(path);

	pcb_t* pcb_nuevo = crear_patota(indice_nueva_patota);
	indice_nueva_patota++;

	cargar_a_memoria(pcb_nuevo, tareas, lista);
	free(pcb_nuevo);

	log_info(logger, "Inicie una patota con %i tripulantes.", cantidad_tripulantes);
	char* memhexstiring = mem_hexstring(memoria_principal->bloque_memoria_principal, memoria_principal->tamanio_memoria);
	log_info(logger,memhexstiring);
	free(memhexstiring);
	free(path);
	free(tareas);
}

//MOVER tid DIRECCION
void mover_tripulante(t_list *lista) {

	char* direccion = (char*) list_get(lista, 2);

	uint32_t direccion_cod = obtener_codigo_de_operacion(direccion);
	uint32_t tid = atoi((char*) list_get(lista, 1));
	tcb_t* tripulante;

	tripulante = get_tripulante_por_id(tid);

	char* imprimible = string_new();
	string_append(&imprimible, "El tripulante ");
	string_append_with_format(&imprimible,"%d", tid);
	string_append(&imprimible, " se desplazo hacia ");
	string_append(&imprimible, direccion);

	switch (direccion_cod) {
	case ARRIBA:
		tripulante->y_pos--;
		break;
	case ABAJO:
		tripulante->y_pos++;
		break;
	case DERECHA:
		tripulante->x_pos++;
		break;
	case IZQUIERDA:
		tripulante->x_pos--;
		break;
	}

	if(strcmp("PAGINACION", memoria_principal->tipo_memoria) == 0) {
		actualizar_tripulante(tripulante);
	}

	int nuevo_x = tripulante->x_pos;
	int nuevo_y = tripulante->y_pos;
	item_mover(mapa, indice_a_simbolo(tid), nuevo_x, nuevo_y);
	nivel_gui_dibujar(mapa);
	log_info(logger, imprimible);
	free(imprimible);
}

void expulsar_tripulante(t_list* lista) {
	log_info(logger, memoria_principal->tipo_memoria);
	//TODO: ACA ROMPE
	uint32_t id = atoi((char*) list_get(lista, 1));


	t_tabla_patota* tabla = get_tabla_por_tid(id);

	if(strcmp("SEGMENTACION", memoria_principal->tipo_memoria) == 0){
		tcb_t* direccion_tcb = get_tripulante_por_id(id);
		borrar_tcb_de_memoria(direccion_tcb);
		memoria_principal->memoria_disponible += sizeof(tcb_t);
	}
	else if (strcmp("PAGINACION", memoria_principal->tipo_memoria) == 0){
		borrar_tcb_de_memoria_paginada(id);
	}

	if (tabla->cantidad_de_tripulantes == 0) {
		eliminar_tabla(tabla);
	}

	log_info(logger, "Simbolo: %c", indice_a_simbolo(id));
	item_borrar(mapa, indice_a_simbolo(id));
	log_info(logger, "Lo borre perfectamente");
	nivel_gui_dibujar(mapa);
	char* imprimible = string_new();
	string_append(&imprimible, "El tripulante ");
	string_append(&imprimible, list_get(lista, 1));
	string_append(&imprimible, " fue expulsado... 1 imposter remains.");
	log_info(logger, imprimible);
	free(imprimible);
}


//Tener en cuenta: Recibe solo el TID en la lista.
void enviar_proxima_tarea(t_list* lista, int socket) {
	uint32_t tid = atoi(list_get(lista, 1));
	tcb_t* tcb = get_tripulante_por_id(tid);

	char* tareas = tareas_del_pcb_por_tid(tid);

//	uint32_t cantidad_tareas = token_count(tareas, '@');
	char* proxima_tarea;

//	if (tcb->proxima_instruccion == cantidad_tareas) {
//		proxima_tarea = string_new();
//		string_append(&proxima_tarea,"FIN_DE_TAREAS 0;0;0;0");
//		log_info(logger, "El tripulante pide fin de tareas");
//	}
//	else {
		char* la_vieja_confiable = string_tcb(tcb);
		log_info(logger,la_vieja_confiable);
		free(la_vieja_confiable);
		proxima_tarea = get_token_at(tareas, '@', tcb->proxima_instruccion);
		(tcb->proxima_instruccion)++;

		if (strcmp(memoria_principal->tipo_memoria, "PAGINACION") == 0) {
			t_tabla_patota* tabla = get_tabla_por_tid(tid);
			actualizar_paginas(tabla->datos_tareas->paginas_que_ocupa);

			actualizar_tripulante(tcb);
		}
//	}
	log_info(logger, "tarea es %s", proxima_tarea);
	t_paquete* paquete;
	paquete = crear_paquete();
	agregar_a_paquete(paquete, proxima_tarea, strlen(proxima_tarea) + 1);
	//pthread_mutex_lock(&mutex_comunicacion);
	enviar_paquete(paquete, socket);
	//close(socket);
	//pthread_mutex_unlock(&mutex_comunicacion);
	log_info(logger, "Envie proxima tarea : %s", proxima_tarea);
	eliminar_paquete(paquete);

	free(proxima_tarea);

	log_info(logger, "Termino de enviar tarea");
	if(strcmp(memoria_principal->tipo_memoria, "PAGINACION") == 0){
		free(tareas);
	}
}


//Funciona!!!!! :D
char* conseguir_tareas_del_path(char* path) {

	char* string_tareas = string_new();

	FILE* archivo;
	archivo = fopen(path, "r+b");

	uint32_t se_puede = 1;

	int i=0;
	while (se_puede) {
		char* tarea = string_new();
		se_puede = leer_linea(&tarea, &archivo);
		string_append(&string_tareas, tarea);
		free(tarea);
		if (se_puede) {
			string_append(&string_tareas, "@"); //Caracter separador de tareas
		}

		i++;
	}
	return string_tareas;
}

void realizar_dump_memoria(int senial) {
	char* dump;
	if (strcmp(memoria_principal->tipo_memoria, "SEGMENTACION") == 0) {
		dump = string_tabla_segmentos();
	}
	else if (strcmp(memoria_principal->tipo_memoria, "PAGINACION") == 0) {
		dump = string_tabla_paginas();
	}
	else {
		dump = "Error: El programa no soporta el esquema de memoria especificado.";
	}

	char* ruta_archivo = string_new();
	string_append(&ruta_archivo, "Dump_<");
	string_append(&ruta_archivo, temporal_get_string_time("%d-%m-%y|%H:%M:%S"));
	string_append(&ruta_archivo, ">.dmp");

	char* info_dump = string_new();
	string_append(&info_dump, "Se ha realizado un dump de la memoria en el siguiente archivo: ");
	string_append(&info_dump, ruta_archivo);
	string_append(&info_dump, ".");
	log_info(logger, info_dump);

	FILE* archivo_dump;
	archivo_dump = fopen(ruta_archivo, "w");
	fputs(dump, archivo_dump);
	fclose(archivo_dump);
	free(dump);
	free(info_dump);
	free(ruta_archivo);
}

int obtener_codigo_de_operacion(char* leido) {
	if (strcmp(leido, "INICIAR_PATOTA") == 0) {
		return INICIAR_PATOTA;
	} else if (strcmp(leido, "INICIAR_TRIPULANTES") == 0) {
		return INICIAR_TRIPULANTES;
	} else if (strcmp(leido, "EXPULSAR_TRIPULANTE") == 0) {
		return EXPULSAR_TRIPULANTE;
	} else if (strcmp(leido, "INICIAR_PLANIFICACION") == 0) {
		return INICIAR_PLANIFICACION;
	} else if (strcmp(leido, "PAUSAR_PLANIFICACION") == 0) {
		return PAUSAR_PLANIFICACION;
	} else if (strcmp(leido, "OBTENER_BITACORA") == 0) {
		return OBTENER_BITACORA;
	} else if (strcmp(leido, "GENERAR_OXIGENO") == 0) {
		return GENERAR_OXIGENO;
	} else if (strcmp(leido, "CONSUMIR_OXIGENO") == 0) {
		return CONSUMIR_OXIGENO;
	} else if (strcmp(leido, "GENERAR_COMIDA") == 0) {
		return GENERAR_COMIDA;
	} else if (strcmp(leido, "CONSUMIR_COMIDA") == 0) {
		return CONSUMIR_COMIDA;
	} else if (strcmp(leido, "GENERAR_BASURA") == 0) {
		return GENERAR_BASURA;
	} else if (strcmp(leido, "DESCARTAR_BASURA") == 0) {
		return DESCARTAR_BASURA;
	} else if (strcmp(leido, "MOVER") == 0) {
		return MOVER;
	} else if (strcmp(leido, "ENVIAR_PROXIMA_TAREA") == 0) {
		return ENVIAR_PROXIMA_TAREA;
	} else if (strcmp(leido, "RECIBIR_UBICACION_TRIUPLANTE") == 0) {
		return RECIBIR_UBICACION_TRIPULANTE;
	} else if (strcmp(leido, "COMENZAR_PROTOCOLO_FSCK") == 0) {
		return COMENZAR_PROTOCOLO_FSCK;
	} else if (strcmp(leido, "ACTUALIZAR_ESTADO") == 0) {
		return ACTUALIZAR_ESTADO;
	} else if (strcmp(leido, "ARRIBA") == 0) {
		return ARRIBA;
	} else if (strcmp(leido, "ABAJO") == 0) {
		return ABAJO;
	} else if (strcmp(leido, "DERECHA") == 0) {
		return DERECHA;
	} else if (strcmp(leido, "IZQUIERDA") == 0) {
		return IZQUIERDA;

	} else
		return -1;
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

void crear_buffer(t_paquete* paquete) {
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

t_paquete* crear_paquete(void) {
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = PAQUETE; // @suppress("Symbol is not resolved")
	crear_buffer(paquete);
	return paquete;
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

void eliminar_paquete(t_paquete* paquete) {
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}
