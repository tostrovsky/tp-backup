#include "utils.h"
//ASD
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

void enviar_paquete_por_conexion(t_paquete* paquete, char* ip, char* puerto){
	int un_socket = crear_conexion(ip, puerto);
	enviar_paquete(paquete, un_socket);
	liberar_conexion(un_socket);
}

int enviar_paquete_por_conexion_especial(t_paquete* paquete, char* ip, char* puerto){
	int un_socket = crear_conexion(ip, puerto);
	enviar_paquete(paquete, un_socket);
	return un_socket;
}

void* instruccion(int socket_cliente) {

	t_list* lista = desencolar_peticion(cola_peticiones);
	uint32_t codigo_operacion = obtener_codigo_de_operacion(list_get(lista, 0));

	if(codigo_operacion == ATENDER_SABOTAJE){
		char* posx = get_token_at(list_get(lista, 1), '|', 0);
		char* posy = get_token_at(list_get(lista, 1), '|', 1);
		posicion_sabotaje.x = atoi(posx);
		posicion_sabotaje.y = atoi(posy);
		free(posx);
		free(posy);
		sem_post(&sem_sabotaje_resuelto);
	}
	list_destroy_and_destroy_elements(lista, free);

	return EXIT_SUCCESS;
}

int request(int* cliente_fd) {

	t_list* lista;
	//while(1){
		int cod_op = recibir_operacion(*cliente_fd);

		switch (cod_op) {
		case MENSAJE: // @suppress("Symbol is not resolved")
			recibir_mensaje(*cliente_fd);
			break;
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
			log_warning(logger, "Operacion desconocida. No quieras meter la pata");
			break;*/
		//}
	}
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
	int tam_direccion = sizeof(struct sockaddr_in);

	int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);

	log_info(logger, "Se conecto un cliente!");

	return socket_cliente;
}

void crear_servidor(){

	int server_fd = iniciar_servidor(ip_discordiador, puerto_discordiador);
	while (1) {
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
}


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
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

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

void liberar_conexion(int socket_cliente) {
	close(socket_cliente);
}

void liberar_conexiones(int un_socket_cliente, int otro_socket_cliente) {
	liberar_conexion(un_socket_cliente);
	liberar_conexion(otro_socket_cliente);
}

void realizar_operacion() {
	char* leido;

	printf("Queres realizar una operacion? y/n \n");
	pthread_mutex_lock(&se_puede_expulsar);

	leido = readline(">");

	while (strcmp(leido, "n") != 0 && strcmp(leido, "N") != 0) {
		if (strcmp(leido, "y") == 0 || strcmp(leido, "Y") == 0) {
			free(leido);
			printf("Que operacion queres realizar? \n");
			leido = readline(">");
			int cod_operacion = obtener_codigo(leido);
			switch (cod_operacion) {
			case INICIAR_PATOTA:
				iniciar_patota();
				sem_post(&sem_puede_realizar_operacion);

				break;
			case LISTAR_TRIPULANTES:
				listar_tripulantes();
				sem_post(&sem_puede_realizar_operacion);
				break;
			case EXPULSAR_TRIPULANTE:
				if(!se_esta_planificando)pthread_mutex_unlock(&se_puede_expulsar);
				preparar_y_enviar_paquete_para_ambas_conexiones(cod_operacion);
				sem_post(&sem_puede_realizar_operacion);

				break;
			case INICIAR_PLANIFICACION:
				if(!se_esta_planificando){
					reanudar_planificacion();
					sem_post(&sem_puede_realizar_operacion);
					pthread_create(&hilo_planificacion, NULL, iniciar_planificacion, NULL);
				}else{
					reanudar_planificacion();
					log_info(logger,"Se ha reanudado la planificacion!");
					sem_post(&sem_puede_realizar_operacion);
				}
				break;
			case PAUSAR_PLANIFICACION:
				pausar_planificacion();
				sem_post(&sem_puede_realizar_operacion);
				break;
			case OBTENER_BITACORA:
				preparar_y_enviar_paquete(ip_i_mongo_store, puerto_i_mongo_store, cod_operacion);
				sem_post(&sem_puede_realizar_operacion);
				break;
			case -1:
				log_error(logger, "No entendi el mensaje, intentalo de nuevo\n");
				sem_post(&sem_puede_realizar_operacion);
			}
			sem_wait(&sem_puede_realizar_operacion);
			printf("Queres realizar otra operacion? y/n \n");
			leido = readline(">");
		} else {
			free(leido);
			log_error(logger, "No entendi el mensaje, intentalo de nuevo");
			leido = readline(">");
			sem_post(&sem_puede_realizar_operacion);
		}
	}
	free(leido);
}

void preparar_y_enviar_paquete_para_ambas_conexiones(int cod_operacion) {
	t_paquete* paquete;
	paquete = crear_paquete();
	paquete = armar_paquete_segun_codigo(paquete, cod_operacion);

	//pthread_mutex_lock(&mutex_comunicacion);
	enviar_paquete_por_conexion(paquete,ip_mi_ram_hq, puerto_mi_ram_hq);
	enviar_paquete_por_conexion(paquete,ip_i_mongo_store, puerto_i_mongo_store);
	//pthread_mutex_unlock(&mutex_comunicacion);
	eliminar_paquete(paquete);
}

void preparar_y_enviar_paquete(char* ip, char* puerto , int cod_operacion) {
	t_paquete* paquete;
	paquete = crear_paquete();
	paquete = armar_paquete_segun_codigo(paquete, cod_operacion);
	//pthread_mutex_lock(&mutex_comunicacion);
	enviar_paquete_por_conexion(paquete, ip, puerto);
	//pthread_mutex_unlock(&mutex_comunicacion);
	eliminar_paquete(paquete);
}

int obtener_codigo_de_operacion(char* leido) {
	if (strcmp(leido, "INICIAR_PATOTA") == 0) {
		return INICIAR_PATOTA;
	} else if (strcmp(leido, "LISTAR_TRIPULANTES") == 0) {
		return LISTAR_TRIPULANTES;
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
	} else if (strcmp(leido, "ATENDER_SABOTAJE") == 0) {
		return ATENDER_SABOTAJE;
	} else if (strcmp(leido, "COMENZAR_PROTOCOLO_FSCK") == 0) {
		return COMENZAR_PROTOCOLO_FSCK;
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

char* codigo_to_string(int codigo) {
	if (codigo == GENERAR_OXIGENO) {
		return "GENERAR_OXIGENO";
	} else if (codigo == CONSUMIR_OXIGENO) {
		return "CONSUMIR_OXIGENO";
	} else if (codigo == GENERAR_COMIDA) {
		return "GENERAR_COMIDA";
	} else if (codigo == CONSUMIR_COMIDA) {
		return "CONSUMIR_COMIDA";
	} else if (codigo == GENERAR_BASURA) {
		return "GENERAR_BASURA";
	} else if (codigo == DESCARTAR_BASURA) {
		return "DESCARTAR_BASURA";
	} else if (codigo == INICIAR_PLANIFICACION) {
		return "INICIAR_PLANIFICACION";
	} else if (codigo == PAUSAR_PLANIFICACION) {
		return "PAUSAR_PLANIFICACION";
	} else if (codigo == INICIAR_PATOTA) {
		return "INICIAR_PATOTA";
	} else if (codigo == MOVER) {
		return "MOVER";
	} else if (codigo == LISTAR_TRIPULANTES) {
		return "LISTAR_TRIPULANTES";
	} else if (codigo == ATENDER_SABOTAJE) {
		return "ATENDER_SABOTAJE";
	} else if (codigo == EXPULSAR_TRIPULANTE) {
		return "EXPULSAR_TRIPULANTE";
	} else if (codigo == OBTENER_BITACORA) {
		return "OBTENER_BITACORA";
	} else if (codigo == COMENZAR_PROTOCOLO_FSCK) {
		return "COMENZAR_PROTOCOLO_FSCK";
	} else if (codigo == ARRIBA) {
		return "ARRIBA";
	} else if (codigo == ABAJO) {
		return "ABAJO";
	} else if (codigo == DERECHA) {
		return "DERECHA";
	} else if (codigo == IZQUIERDA) {
		return "IZQUIERDA";
	} else
		return "No existe esa operacion";
}

int obtener_codigo(char* leido) {
	int codigo = obtener_codigo_de_operacion(leido);
	free(leido);
	switch (codigo) {
	case GENERAR_OXIGENO:
		log_info(logger, "Generando Oxigeno...");
		break;
	case CONSUMIR_OXIGENO:
		log_info(logger, "Consumiendo Oxigeno...");
		break;
	case GENERAR_COMIDA:
		log_info(logger, "Generando Comida...");
		break;
	case CONSUMIR_COMIDA:
		log_info(logger, "Consumiendo Comida...");
		break;
	case GENERAR_BASURA:
		log_info(logger, "Generando Basura...");
		break;
	case DESCARTAR_BASURA:
		log_info(logger, "Descartando Basura...");
		break;
	case COMENZAR_PROTOCOLO_FSCK:
		log_info(logger, "Comenzando Protocolo FSCK...");
		break;
	case MOVER:
		log_info(logger, "Moviendo tripulante...");
		break;
	case ATENDER_SABOTAJE:
		log_info(logger, "Hubo un sabotaje. Atendiendo sabotaje...");
		break;
	case INICIAR_PATOTA:
		log_info(logger, "Iniciando Patota...");
		break;
	case LISTAR_TRIPULANTES:
		log_info(logger, "Listando Tripulantes...");
		break;
	case EXPULSAR_TRIPULANTE:
		log_info(logger, "Expulsando Tripulante...");
		break;
	case INICIAR_PLANIFICACION:
		log_info(logger, "Iniciando Planificacion...");
		break;
	case PAUSAR_PLANIFICACION:
		log_info(logger, "Pausando Planificacion...");
		break;
	case OBTENER_BITACORA:
		log_info(logger, "Obteniendo Bitacora...");
		break;
	case -1:
		log_error(logger, "No entendi el mensaje, intentalo de nuevo");
		return -1; // @suppress("Unused return value")
	default:
		log_warning(logger, "Operacion desconocida");
		break;
	}
	return codigo;
}

t_paquete* armar_paquete_segun_codigo(t_paquete* paquete, int codigo) {

	char* operacion = codigo_to_string(codigo);
	agregar_a_paquete(paquete, operacion, strlen(operacion) + 1);

	switch (codigo) {
	case EXPULSAR_TRIPULANTE:
		paquete = armar_paquete_expulsar_tripulante(paquete);
		break;
	case OBTENER_BITACORA:
		paquete = armar_paquete_obtener_bitacora(paquete);
		break;
	case COMENZAR_PROTOCOLO_FSCK:
		paquete = armar_paquete_comenzar_protocolo_fcsk(paquete);
		break;
	case -1:
		log_error(logger, "No entendi el mensaje, intentalo denuevo");
		return EXIT_FAILURE;
	default:
		log_warning(logger, "Operacion desconocida");
		break;
	}
	return paquete;
}

void iniciar_patota() {
	log_info(logger, "Iniciando Patota");
	t_paquete* paquete = crear_paquete();
	t_paquete* paquete2 = crear_paquete();

	char* leido;
	agregar_a_paquete(paquete,"INICIAR_TRIPULANTES", strlen("INICIAR_TRIPULANTES") + 1);
	agregar_a_paquete(paquete2,"INICIAR_PATOTA", strlen("INICIAR_PATOTA") + 1);

	printf("Ingresar cantidad de tripulantes");
	leido = readline(">");
	int cantidad_tripulantes = atoi(leido);
	agregar_a_paquete(paquete, leido, strlen(leido) + 1);
	agregar_a_paquete(paquete2, leido, strlen(leido) + 1);
	free(leido);

	printf("Ingresar nombre del archivo de tareas");
	leido = readline(">");
	agregar_a_paquete(paquete, leido, strlen(leido) + 1);
	agregar_a_paquete(paquete2, leido, strlen(leido) + 1);
	free(leido);

	//pthread_mutex_lock(&mutex_comunicacion);
	int una_conexion;
	una_conexion = enviar_paquete_por_conexion_especial(paquete2,ip_mi_ram_hq,puerto_mi_ram_hq);
	eliminar_paquete(paquete2);

	recibir_operacion(una_conexion);
	t_list* confirmacion = recibir_paquete(una_conexion);
	liberar_conexion(una_conexion);
	//pthread_mutex_unlock(&mutex_comunicacion);

	//3|3, 4|4, etc
	if(strcmp(list_get(confirmacion, 0), "MEMORIA DISPONIBLE") == 0){
			id_primer_tripulante_de_ultima_patota = id_ultimo_tripulante + 1;
			int contador = 1;
			incrementar_id_ultima_patota();
			while (contador <= cantidad_tripulantes) {
				t_tripulante* tripulante = malloc(sizeof(t_tripulante));
				t_posicion posicion;
				int i = 0;

				printf("Ingresar posicion en X|Y del tripulante %d", contador);
				leido = readline(">");
				if (strcmp(leido, "") == 0) {
					strcpy(leido, "0|0");
					i = 1;
				}
				incrementar_id_ultimo_tripulante();

				char* tokenx = get_token_at(leido, '|', 0);
				char* tokeny = get_token_at(leido, '|', 1);

				posicion.x = atoi(tokenx);
				posicion.y = atoi(tokeny);
				free(tokenx);
				free(tokeny);
				*tripulante = crear_tripulante(id_ultimo_tripulante, 'N', posicion,
						0, id_ultima_patota);

				add_tripulante(tripulante);
				agregar_a_paquete(paquete, leido, strlen(leido) + 1);
				sem_init(&sem_tripulantes[id_ultimo_tripulante - 1],0,0);
				sem_init(&sem_bloqueados[id_ultimo_tripulante - 1],0,0);
				sem_init(&sem_sabotaje[id_ultimo_tripulante - 1],0,1);
				contador++;

				while (i == 1 && contador <= cantidad_tripulantes) {
					t_tripulante* tripulante = malloc(sizeof(t_tripulante));
					posicion.x = 0;
					posicion.y = 0;
					incrementar_id_ultimo_tripulante();
					*tripulante = crear_tripulante(id_ultimo_tripulante, 'N',
							posicion, 0, id_ultima_patota);


					add_tripulante(tripulante);
					agregar_a_paquete(paquete, "0|0", strlen(leido) + 1);
					sem_init(&sem_tripulantes[id_ultimo_tripulante - 1],0,0);
					sem_init(&sem_bloqueados[id_ultimo_tripulante - 1],0,0);
					sem_init(&sem_sabotaje[id_ultimo_tripulante - 1],0,1);
					contador++;
				}
				free(leido);
			}

			log_info(logger, "Se ha creado la patota %d con %d tripulantes", id_ultima_patota, cantidad_tripulantes);

			//pthread_mutex_lock(&mutex_comunicacion);
				enviar_paquete_por_conexion(paquete,ip_mi_ram_hq, puerto_mi_ram_hq);
				enviar_paquete_por_conexion(paquete,ip_i_mongo_store, puerto_i_mongo_store);
			//pthread_mutex_unlock(&mutex_comunicacion);

			eliminar_paquete(paquete);
			hay_tripulantes_new = 1;
	}else{
		log_info(logger, "No hay memoria disponible para iniciar dicha patota. Reintentar mas tarde");
		eliminar_paquete(paquete);
	}
	list_destroy_and_destroy_elements(confirmacion,free);
}
char* obtener_estado_segun_letra(char caracter) {
	char* str = malloc(12);
	switch (caracter) {
	case 'N':
		strcpy(str, "NEW");
		break;
	case 'B':
		strcpy(str, "BLOCKED");
		break;
	case 'R':
		strcpy(str, "READY");
		break;
	case 'E':
		strcpy(str, "EXEC");
		break;
	case 'F':
		strcpy(str, "FINAL");
		break;
	case 'Z':
		strcpy(str, "EXPULSADO");
		break;
	case 'W':
		strcpy(str, "EMERGENCIA");
		break;
	}
	return str;
}

t_paquete* armar_paquete_expulsar_tripulante(t_paquete* paquete) {
	log_info(logger, "Se ha pedido expulsar un tripulante");
	char* leido;
	printf("Ingresar id del impostor");
	leido = readline(">");
	agregar_a_paquete(paquete, leido, strlen(leido) + 1);
	uint32_t id_tripulante = atoi(leido);

	t_tripulante * impostor = get_tripulante_given_id(id_tripulante);
	impostor->est = 'Z';
	impostor->expulsado = 1;

	log_info(logger, "Se ha expulsado al tripulante %s", leido);

	free(leido);
	log_info(logger,"Se completo la expulsion perfectamente!");

	cantidad_tripulantes_en_z ++;
	pthread_mutex_lock(&se_puede_expulsar);


	return paquete;
}


t_paquete* armar_paquete_obtener_bitacora(t_paquete* paquete) {
	log_info(logger, "Se ha pedido por consola obtener la bitácora");
	char* leido;
	printf("Ingresar id del tripulante");
	leido = readline(">");
	agregar_a_paquete(paquete, leido, strlen(leido) + 1);
	free(leido);
	return paquete;
}

t_paquete* armar_paquete_comenzar_protocolo_fcsk(t_paquete* paquete){
	log_info(logger, "Comienza el protocolo FSCK en i Mongo Store");
	agregar_a_paquete(paquete, "COMENZAR_PROTOCOLO_FSCK", strlen("COMENZAR_PROTOCOLO_FSCK") + 1);
	return paquete;
}

void* protocolo_anti_sabotaje(){

	while(1){

		sem_wait(&sem_sabotaje_resuelto);
		pausar_planificacion();

		double distancia_minima = 400;
		int id_reparador;

		t_tripulante* tripulante_ejecutor;

		sleep(0.1);

		for (int i = 0; i < list_size(lista_tripulantes); i++){
			t_tripulante* tripulante = get_tripulante(i);

			if(tripulante->est == 'E' || tripulante->est == 'R'){

				tripulante->estado_pre_sabotaje = tripulante->est;
				tripulante->est = 'W';
				avisar_mi_ram_estado(tripulante->id, 'B');

				log_warning(logger, "diagonal hasta del tripulante %d es %d", tripulante->id ,diagonal_hasta(tripulante, posicion_sabotaje ) );
				if(diagonal_hasta(tripulante, posicion_sabotaje ) < distancia_minima){
					distancia_minima = diagonal_hasta(tripulante, posicion_sabotaje );

					id_reparador = tripulante->id;
				}
				if(tripulante->estado_pre_sabotaje == 'E')queue_push(cola_emergency, tripulante);
			}
		}
	//todos los EXEC en la cola
		int largo_cola = queue_size(cola_ready);

		for(int j = 0; j < largo_cola; j++){
			t_planificando* planificando = desencolar_tripulante(cola_ready);
			queue_push(cola_emergency, planificando->tripulante);
			encolar_tripulante(planificando, cola_ready);
		}
	//todos los READY en la cola
		for(int k = 0; k < queue_size(cola_emergency); k++){
			t_tripulante* tripulante = queue_pop(cola_emergency);
			if(tripulante->id == id_reparador){
				tripulante_ejecutor = tripulante;
				k--;
			}else{
				queue_push(cola_emergency, tripulante);
			}
		}
		tripulante_ejecutor->atiende_sabotaje ++;
		tripulante_ejecutor->est = 'E';
		avisar_mi_ram_estado(tripulante_ejecutor->id, 'E');

		log_info(logger, "atiende el sabotaje %d", tripulante_ejecutor->id);

	//Tripulante se pone a resolver el sabotaje;

	char* posicion_sabotaje_actual = posicion_to_string(posicion_sabotaje);
	avisar_imongo_bitacora("ATENDER_SABOTAJE", tripulante_ejecutor->id, posicion_sabotaje_actual);
	mover_tripulante_a_destino(tripulante_ejecutor, posicion_sabotaje);
		tripulante_ejecutor->est = 'W';
		avisar_mi_ram_estado(tripulante_ejecutor->id, 'B');
		sleep(duracion_sabotaje-1);
		preparar_y_enviar_paquete(ip_i_mongo_store, puerto_i_mongo_store, COMENZAR_PROTOCOLO_FSCK);
		log_info(logger, "tripulante %d arregla el sabotaje",tripulante_ejecutor->id);
	sleep(1);
	avisar_imongo_bitacora("RESOLVER_SABOTAJE", tripulante_ejecutor->id, posicion_sabotaje_actual);
	free(posicion_sabotaje_actual);

	queue_push(cola_emergency, tripulante_ejecutor);

		log_info(logger, "se vuelve a la normalidad");

		while(!queue_is_empty(cola_emergency)){
			t_tripulante* tripulante = queue_pop(cola_emergency);
			tripulante->est = tripulante->estado_pre_sabotaje;
			avisar_mi_ram_estado(tripulante->id, tripulante->est);
		}

		reanudar_planificacion();

	}
	return EXIT_SUCCESS;
}

double distancia_hasta(t_tripulante* tripulante, t_posicion destino){
	return abs(tripulante->posicion.x - destino.x) + abs(tripulante->posicion.y - destino.y);
	//double a = pow(tripulante->posicion.x - destino.x, 2);
	//double b = pow(tripulante->posicion.y - destino.y, 2);

	//return sqrt(a + b);
}

double diagonal_hasta(t_tripulante* tripulante, t_posicion destino){
	//return abs(tripulante->posicion.x - destino.x) + abs(tripulante->posicion.y - destino.y);
	double a = pow((double)(tripulante->posicion.x - destino.x), 2);
	double b = pow((double)(tripulante->posicion.y - destino.y), 2);

	return sqrt(a + b);
}

char* posicion_to_string(t_posicion posicion){
	char* coordenadas = (char*) string_new();
	char* pos_x = string_itoa(posicion.x);
	char* pos_y = string_itoa(posicion.y);

	string_append(&coordenadas, pos_x);
	string_append(&coordenadas, "|");
	string_append(&coordenadas, pos_y );

	free(pos_x);
	free(pos_y);
	return coordenadas;
}

void listar_tripulantes() {
	log_info(logger, "Se ha pedido por consola listar los tripulantes");
	t_tripulante* tripulante_actual;
	uint32_t tamanio_lista = list_size(lista_tripulantes);
	char* dia_hora_etc = temporal_get_string_time("%d/%m/%y %H:%M:%S");
	printf( "\n--------------------------------------------------------\n" "Estado de la nave: %s\nCantidad de tripulantes: %d", dia_hora_etc, tamanio_lista);
	for (int i = 0; i < tamanio_lista; i++) {
		tripulante_actual = get_tripulante(i);
		//tripulante_actual = *((t_tripulante*)list_get(lista_tripulantes, i));
		printf("\nTripulante: %d", tripulante_actual->id);
		printf("\tPatota: %d", tripulante_actual->patota_id);
		char* estado = obtener_estado_segun_letra(tripulante_actual->est);
		printf("\tStatus: %s", estado);
		free(estado);
	}
	printf("\n--------------------------------------------------------\n");
	free(dia_hora_etc);
}


t_tarea* pedir_proxima_tarea(t_tripulante* tripulante) {
	t_paquete* paquete = crear_paquete();

	log_info(logger, "hola 1 tripulante %d", tripulante->id);
	char* tripulante_id = (char*) string_itoa(tripulante->id);
	agregar_a_paquete(paquete, "ENVIAR_PROXIMA_TAREA", strlen("ENVIAR_PROXIMA_TAREA") + 1);
	agregar_a_paquete(paquete, tripulante_id, strlen(tripulante_id) + 1);

	log_info(logger, "hola 2 tripulante %d", tripulante->id);

	//pthread_mutex_lock(&mutex_comunicacion);
	int una_conexion;
	log_info(logger, "hola Mutex tripulante %d", tripulante->id);
	una_conexion = enviar_paquete_por_conexion_especial(paquete,ip_mi_ram_hq,puerto_mi_ram_hq);
	eliminar_paquete(paquete);

	log_info(logger, "hola mutex0 tripulante %d", tripulante->id);
	recibir_operacion(una_conexion);

	log_info(logger, "hola mutex1 tripulante %d", tripulante->id);
	t_list* lista_de_tarea = recibir_paquete(una_conexion);

	log_info(logger, "hola mutex2 tripulante %d", tripulante->id);
	liberar_conexion(una_conexion);

	log_info(logger, "hola mutex3 tripulante %d", tripulante->id);
	//pthread_mutex_unlock(&mutex_comunicacion);

	log_info(logger, "hola 3 tripulante %d", tripulante->id);

	char* cadena_tokenizada = list_get(lista_de_tarea, 0);
	t_tarea* tarea;

	char* token_nombre;
	char* token0;
	char* token1;
	char* token2;
	char* token3;

	if(tripulante->id == 7 || tripulante->id == 8)log_info(logger, "hola 4 tripulante %d", tripulante->id);

	if(char_count(cadena_tokenizada, ' ') == 1){ // es tarea bloqueante
		token_nombre = get_token_at(cadena_tokenizada, ' ', 0);
		char* parametros = get_token_at(cadena_tokenizada, ' ', 1);
		token0 = get_token_at(parametros, ';', 0);
		token1 = get_token_at(parametros, ';', 1);
		token2 = get_token_at(parametros, ';', 2);
		token3 = get_token_at(parametros, ';', 3);

		tarea = crear_tarea(token_nombre,
				atoi(token0),
				atoi(token1),
				atoi(token2),
				atoi(token3));
		free(parametros);
	}else{ // no es
		token_nombre = get_token_at(cadena_tokenizada, ';', 0);
		token0 = malloc(2);
		token1 = get_token_at(cadena_tokenizada, ';', 1);
		token2 = get_token_at(cadena_tokenizada, ';', 2);
		token3 = get_token_at(cadena_tokenizada, ';', 3);

		tarea = crear_tarea(token_nombre, 0,
				atoi(token1),
				atoi(token2),
				atoi(token3));
	}

	log_info(logger, "-------------------");
	log_info(logger, "TRIPULANTE  %d", tripulante->id);
	log_info(logger, "pos x  = %d", tripulante->posicion.x);
	log_info(logger, "pos y  = %d", tripulante->posicion.y);
	log_info(logger, "-------------------");
	log_info(logger, "la tarea pedida es: %s", token_nombre);
	log_info(logger, "pos x  = %d", atoi(token1));
	log_info(logger, "pos y  = %d", atoi(token2));
	log_info(logger, "el tiempo es: %d ", atoi(token3));
	log_info(logger, "-------------------");

	free(token0);
	free(token1);
	free(token2);
	free(token3);
	free(tripulante_id);
	list_destroy_and_destroy_elements(lista_de_tarea,free);



	return tarea;
}

void reanudar_planificacion(){
	planificacion_esta_pausada = 0;

	for (int i = 0; i < list_size(lista_tripulantes); i++){
		sem_post(&sem_planificacion_pausada);
	}
}
void pausar_planificacion(){
	planificacion_esta_pausada = 1;

	for (int i = 0; i < list_size(lista_tripulantes); i++){
		sem_wait(&sem_planificacion_pausada);
	}
	log_info(logger,"Se ha pausado la planificacion!");
}

/*----Funciones de planificacion
 **----------------------------
 */


void wait_signal_pausar_durante_tarea(t_planificando* planificando){
	sem_wait(&sem_planificacion_pausada);
	sem_post(&sem_planificacion_pausada);
	if(planificando->tripulante->atiende_sabotaje){
		planificando->tripulante->atiende_sabotaje--;
		t_posicion destino = {planificando->tarea->pos_x, planificando->tarea->pos_y};
		if(strcmp(algoritmo, "FIFO") == 0){
			mover_planificando_a_destino(planificando, destino);
		}else{ //si es RR
			planificando->tripulante->rafaga_faltante += distancia_hasta(planificando->tripulante, destino);
			mover_al_estilo_round_robin(planificando, destino);
		}
	}
}

void wait_signal_pausar_planificando(t_planificando* planificando){
	sem_wait(&sem_planificacion_pausada);
	sem_post(&sem_planificacion_pausada);
	if(planificando->tripulante->atiende_sabotaje){
		planificando->tripulante->atiende_sabotaje--;
		t_posicion destino = {planificando->tarea->pos_x, planificando->tarea->pos_y};
		setear_rafaga_faltante(planificando, destino);
	}
}

void wait_signal_pausar(){
	sem_wait(&sem_planificacion_pausada);
	sem_post(&sem_planificacion_pausada);
}

void* iniciar_planificacion() {
	pthread_t hilo_bloqueante;
	se_esta_planificando = 1;
	log_info(logger, "Se ha pedido por consola iniciar la planificación");
	poner_todos_los_tripulantes_en_ready();

	sem_init(&sem_hay_tripulantes_en_ready, 0, queue_size(cola_ready));
	sem_init(&sem_hay_tripulantes_en_block, 0, 0);

	pthread_create(&hilo_sabotaje, NULL, protocolo_anti_sabotaje, NULL);
	pthread_create(&hilo_bloqueante, NULL, iniciar_planificacion_tareas_bloqueantes, NULL);

	if (strcmp(algoritmo, "FIFO") == 0) {
		while (1) {
			pthread_t hilo;
			if(hay_tripulantes_new)
			poner_ultima_patota_en_ready();

			sem_wait(&sem_grado_multiprocesamiento);
			sem_wait(&sem_hay_tripulantes_en_ready);

			t_planificando* planificando = desencolar_tripulante(cola_ready);

			if (!planificando->tripulante->esta_ejecutando){
				planificando->tripulante->esta_ejecutando = 1;
				pthread_create(&hilo, NULL, ejecutar_tripulante_fifo, (void*) planificando);
				set_hilo(planificando->tripulante, hilo);
			}else{
				sem_post(&sem_tripulantes[planificando->tripulante->id - 1]);
			}
		}
	} else { //si es RR
		while (1) {
			pthread_t hilo;
			if(hay_tripulantes_new)
			poner_ultima_patota_en_ready();

			sem_wait(&sem_grado_multiprocesamiento);
			sem_wait(&sem_hay_tripulantes_en_ready);

			t_planificando* planificando = desencolar_tripulante(cola_ready);

			if (!planificando->tripulante->esta_ejecutando){
				planificando->tripulante->esta_ejecutando = 1;
				pthread_create(&hilo, NULL, ejecutar_tripulante_rr, (void*) planificando);
				set_hilo(planificando->tripulante, hilo);
			}else{
				sem_post(&sem_tripulantes[planificando->tripulante->id - 1]);
			}
		}
	}
	return EXIT_SUCCESS;
}

void* ejecutar_tripulante_fifo(void* planificando_void) {
	t_planificando* planificando = planificando_void;
	int tid = planificando->tripulante->id;
	int estuvo_bloqueado;

	wait_signal_pausar_planificando(planificando);
	if(planificando->tripulante->expulsado){ echar(planificando); return EXIT_SUCCESS; }


	while(tarea_not_null(planificando->tarea)){

		estuvo_bloqueado = 0;

		planificando->tripulante->est = 'E';
		avisar_mi_ram_estado(planificando->tripulante->id, 'E');
		t_posicion destino;
		destino.x = planificando->tarea->pos_x;
		destino.y = planificando->tarea->pos_y;

		mover_planificando_a_destino(planificando, destino);
		wait_signal_pausar_durante_tarea(planificando);

		if(es_tarea_bloqueante(planificando->tarea)){

			planificando->tripulante->est = 'B';
			avisar_mi_ram_estado(planificando->tripulante->id, 'B');
			estuvo_bloqueado = 1;
			queue_push(cola_block, (void*) tid);
			sem_post(&sem_grado_multiprocesamiento);
			sem_post(&sem_hay_tripulantes_en_block);
			sem_wait(&sem_bloqueados[tid-1]);

			ejecutar_tarea_bloqueante(planificando);
			sem_post(&sem_dispositivo_i_o);
		}else{
			ejecutar_tarea(planificando);
		}

		log_info(logger, "El tripulante %d termino de %s.", tid, planificando->tarea->nombre);

		wait_signal_pausar_planificando(planificando);
		if(planificando->tripulante->expulsado){ echar(planificando); return EXIT_SUCCESS; }

		free(planificando->tarea->nombre);
		free(planificando->tarea);
		log_info(logger, "El tripulante %d pide la proxima tarea", planificando->tripulante->id);
		planificando->tarea = (t_tarea*) pedir_proxima_tarea(planificando->tripulante);
		log_info(logger, "La proxima tarea del tripulante %d es %s", planificando->tripulante->id, planificando->tarea->nombre);

		if(estuvo_bloqueado)replanifica_por_bloqueo(planificando);
	}
	log_info(logger, "El tripulante %d TERMINO", planificando->tripulante->id);
	wait_signal_pausar();
	planificando->tripulante->est = 'F';
	avisar_mi_ram_estado(planificando->tripulante->id, 'F');
	sem_post(&sem_grado_multiprocesamiento);
	incrementar_tripulantes_finished();

	log_info(logger, "El tripulante %d termino todas las tareas.", tid);
	free(planificando->tarea->nombre);
	free(planificando->tarea);
	free(planificando);

	if(todos_en_f()){
		se_esta_planificando = 0;
		pthread_cancel((pthread_t) &hilo_planificacion);
		log_info(logger, "La planificacion ha finalizado!");
	}

	return EXIT_SUCCESS;
}

void* ejecutar_tripulante_rr(void* planificando_void) {
	t_planificando* planificando = planificando_void;
	int tid = planificando->tripulante->id;
	int estuvo_bloqueado;

	wait_signal_pausar_planificando(planificando);
	if(planificando->tripulante->expulsado){ echar(planificando); return EXIT_SUCCESS; }

	while(tarea_not_null(planificando->tarea)){

		estuvo_bloqueado = 0;
		planificando->tripulante->quantum_disponible = quantum;

		planificando->tripulante->est = 'E';
		avisar_mi_ram_estado(planificando->tripulante->id, 'E');
		t_posicion destino;
		destino.x = planificando->tarea->pos_x;
		destino.y = planificando->tarea->pos_y;

		mover_al_estilo_round_robin(planificando, destino);
		wait_signal_pausar_durante_tarea(planificando);

		if(es_tarea_bloqueante(planificando->tarea)){

			planificando->tripulante->est = 'B';
			avisar_mi_ram_estado(planificando->tripulante->id, 'B');
			estuvo_bloqueado = 1;
			queue_push(cola_block,(void*) tid);
			sem_post(&sem_grado_multiprocesamiento);
			sem_post(&sem_hay_tripulantes_en_block);
			sem_wait(&sem_bloqueados[tid-1]);

			ejecutar_tarea_bloqueante(planificando);
			sem_post(&sem_dispositivo_i_o);
		}else{
			ejecutar_tarea_round_robin(planificando);
		}

		log_info(logger, "El tripulante %d termino de %s.", tid, planificando->tarea->nombre);

		wait_signal_pausar_planificando(planificando);
		if(planificando->tripulante->expulsado){ echar(planificando); return EXIT_SUCCESS; }

		free(planificando->tarea->nombre);
		free(planificando->tarea);
		planificando->tarea = pedir_proxima_tarea(planificando->tripulante);

		destino.x = planificando->tarea->pos_x;
		destino.y = planificando->tarea->pos_y;
		setear_rafaga_faltante(planificando, destino);

		if(estuvo_bloqueado)replanifica_por_bloqueo(planificando);
	}
	wait_signal_pausar();
	planificando->tripulante->est = 'F';
	avisar_mi_ram_estado(planificando->tripulante->id, 'F');
	sem_post(&sem_grado_multiprocesamiento);
	incrementar_tripulantes_finished();

	log_info(logger, "El tripulante %d termino todas las tareas.", tid);
	free(planificando->tarea->nombre);
	free(planificando->tarea);
	free(planificando);

	if(todos_en_f()){
		se_esta_planificando = 0;
		pthread_cancel( (pthread_t) &hilo_planificacion);
		log_info(logger, "La planificacion ha finalizado!");
	}

	return EXIT_SUCCESS;
}

void echar(t_planificando* planificando){
	planificando->tripulante->est = 'Z';
	free(planificando->tarea->nombre);
	free(planificando->tarea);
	free(planificando);
	pthread_mutex_unlock(&se_puede_expulsar);
	sem_post(&sem_grado_multiprocesamiento);
}


void* iniciar_planificacion_tareas_bloqueantes(){

	while (1) {

		sem_wait(&sem_dispositivo_i_o);
		sem_wait(&sem_hay_tripulantes_en_block);
		int tid = (int) queue_pop(cola_block);
		sem_post(&sem_bloqueados[tid -1]);
	}
	return EXIT_SUCCESS;
}

void poner_ultima_patota_en_ready(){
	int i = id_primer_tripulante_de_ultima_patota;
	while(i <= list_size(lista_tripulantes)){
			t_tripulante* tripulante = get_tripulante_given_id(i);
			tripulante->est = 'R';
			avisar_mi_ram_estado(tripulante->id, 'R');
			t_planificando* planificando = malloc(sizeof(t_planificando));
			t_tarea* tarea = pedir_proxima_tarea(tripulante);

			planificando->tarea = tarea;
			planificando->tripulante = tripulante;

			t_posicion destino;
			destino.x = planificando->tarea->pos_x;
			destino.y = planificando->tarea->pos_y;

			setear_rafaga_faltante(planificando, destino);
			encolar_tripulante(planificando, cola_ready);

			sem_post(&sem_hay_tripulantes_en_ready);
			i ++;
	}
	hay_tripulantes_new = 0;
}

void ejecutar_tarea(t_planificando* planificando) {

	avisar_imongo_bitacora_tarea("COMENZAR" , planificando->tripulante->id, planificando->tarea->nombre, planificando->tarea->cantidad);

	while(planificando->tarea->tiempo) {
		sleep(retardo_ciclo_cpu);
		planificando->tarea->tiempo--;
		wait_signal_pausar_durante_tarea(planificando);
		if(planificando->tripulante->expulsado)return;
	}

	avisar_imongo_bitacora_tarea("FINALIZAR" , planificando->tripulante->id, planificando->tarea->nombre, planificando->tarea->cantidad);
}

void ejecutar_tarea_bloqueante(t_planificando* planificando) {

	avisar_imongo_bitacora_tarea("COMENZAR" , planificando->tripulante->id, planificando->tarea->nombre, planificando->tarea->cantidad);

	for (int i = 1; i <= planificando->tarea->tiempo; i++) {
		sleep(retardo_ciclo_cpu);

		wait_signal_pausar();
		if(planificando->tripulante->expulsado)return;
	}

	avisar_imongo_bitacora_tarea("FINALIZAR" , planificando->tripulante->id, planificando->tarea->nombre, planificando->tarea->cantidad);
}

void ejecutar_tarea_round_robin(t_planificando* planificando){

	avisar_imongo_bitacora_tarea("COMENZAR" , planificando->tripulante->id, planificando->tarea->nombre, planificando->tarea->cantidad);

	while(planificando->tripulante->rafaga_faltante) {
		sleep(retardo_ciclo_cpu);

		planificando->tripulante->quantum_disponible --;
		planificando->tripulante->rafaga_faltante --;

		wait_signal_pausar_durante_tarea(planificando);
		if(planificando->tripulante->expulsado)break;
		if(!planificando->tripulante->quantum_disponible)replanifica(planificando);
	}

	avisar_imongo_bitacora_tarea("FINALIZAR" , planificando->tripulante->id, planificando->tarea->nombre, planificando->tarea->cantidad);
}

void replanifica(t_planificando* planificando){
	wait_signal_pausar();
	planificando->tripulante->est = 'R';
	avisar_mi_ram_estado(planificando->tripulante->id, 'R');
	encolar_tripulante(planificando, cola_ready);
	sem_post(&sem_hay_tripulantes_en_ready);
	sem_post(&sem_grado_multiprocesamiento);
	sem_wait(&sem_tripulantes[planificando->tripulante->id-1]);
	wait_signal_pausar();
	planificando->tripulante->est = 'E';
	avisar_mi_ram_estado(planificando->tripulante->id, 'E');
	planificando->tripulante->quantum_disponible = quantum;
}

void replanifica_por_bloqueo(t_planificando* planificando){
	wait_signal_pausar();
	planificando->tripulante->est = 'R';
	avisar_mi_ram_estado(planificando->tripulante->id, 'R');
	encolar_tripulante(planificando, cola_ready);
	sem_post(&sem_hay_tripulantes_en_ready);
	sem_wait(&sem_tripulantes[planificando->tripulante->id-1]);
}

int todos_en_f(){
	return cantidad_tripulantes_en_f + cantidad_tripulantes_en_z == list_size(lista_tripulantes);
}
void set_hilo(t_tripulante* tripulante, pthread_t hilo) {
	tripulante->hilo = hilo;
}
void poner_todos_los_tripulantes_en_ready() {

	for (int i = 0; i < list_size(lista_tripulantes); i++) {
		t_tripulante* tripulante = get_tripulante(i);

		if(!tripulante->expulsado){

			tripulante->est = 'R';

			avisar_mi_ram_estado(tripulante->id, 'R');

			t_planificando* planificando = malloc(sizeof(t_planificando));
			t_tarea* tarea = pedir_proxima_tarea(tripulante);

			planificando->tarea = tarea;
			planificando->tripulante = tripulante;

			t_posicion destino;
			destino.x = planificando->tarea->pos_x;
			destino.y = planificando->tarea->pos_y;

			setear_rafaga_faltante(planificando, destino);
			encolar_tripulante(planificando, cola_ready);
		}
	}
	hay_tripulantes_new = 0;
}
int cola_de_ready_tiene_tripulantes() {
	return queue_size(cola_ready);
}
int es_tarea_bloqueante(t_tarea* tarea) {
	return obtener_codigo_de_operacion(tarea->nombre) + 1;

	//si es bloqueante, retorna [2;7] -> true
	//si no lo es, retorna 0 -> false
}
int sigue_en_execute(t_tripulante* tripulante) {
	return tripulante->est == 'E' ? 1 : 0;
}
int fue_expulsado(t_tripulante* tripulante) {
	return tripulante->est == 'Z' ? 1 : 0;
}
int tarea_not_null(t_tarea* tarea){
	return strcmp(tarea->nombre, "FIN_DE_TAREAS")== 0 ? 0 : 1;
}

void setear_rafaga_faltante(t_planificando* planificando, t_posicion destino){
	int rafaga_faltante = abs(planificando->tripulante->posicion.x - destino.x) +
			              abs(planificando->tripulante->posicion.y - destino.y) ;
	if(!es_tarea_bloqueante(planificando->tarea)){
		rafaga_faltante += planificando->tarea->tiempo;
	}
	planificando->tripulante->rafaga_faltante = rafaga_faltante;
}

/*
 *  FUNCIONES aviso de servidores
 */


void avisar_servidores_movimiento_del_tripulante(uint32_t id, uint32_t direccion) {

	char* tid = (char*) string_itoa(id);
	t_paquete* paquete = crear_paquete();
	t_paquete* paquete_i_mongo = crear_paquete();
	t_tripulante* tripulante = get_tripulante_given_id(id);
	char* posx = (char*) string_itoa(tripulante->posicion.x);
	char* posy = (char*) string_itoa(tripulante->posicion.y);


	agregar_a_paquete(paquete, "MOVER", strlen("MOVER") + 1);
	agregar_a_paquete(paquete_i_mongo, "MOVER", strlen("MOVER") + 1);
	agregar_a_paquete(paquete, tid, strlen(tid) + 1);
	agregar_a_paquete(paquete_i_mongo, tid, strlen(tid) + 1);
	agregar_a_paquete(paquete, codigo_to_string(direccion), strlen(codigo_to_string(direccion)) + 1);
	agregar_a_paquete(paquete_i_mongo, posx , strlen(posx) + 1);
	agregar_a_paquete(paquete_i_mongo, posy , strlen(posx) + 1);

	if(id == 7 || id == 8)log_info(logger, "tripulante %d se mueve", id);
	//pthread_mutex_lock(&mutex_comunicacion);
	enviar_paquete_por_conexion(paquete,ip_mi_ram_hq,puerto_mi_ram_hq);
	enviar_paquete_por_conexion(paquete_i_mongo, ip_i_mongo_store, puerto_i_mongo_store);
	//pthread_mutex_unlock(&mutex_comunicacion);
	if(id == 7 || id == 8)log_info(logger, "tripulante %d se movio", id);

	eliminar_paquete(paquete);
	eliminar_paquete(paquete_i_mongo);
	free(tid);
	free(posx);
	free(posy);
}

void avisar_imongo_bitacora(char* operacion, uint32_t id, char* posicion) {

	char* tid = (char*) string_itoa(id);
	t_paquete* paquete = crear_paquete();

	agregar_a_paquete(paquete, operacion, strlen(operacion) + 1);
	agregar_a_paquete(paquete, tid, strlen(tid) + 1);
	agregar_a_paquete(paquete, posicion, strlen(posicion) + 1);

	//pthread_mutex_lock(&mutex_comunicacion);
	enviar_paquete_por_conexion(paquete, ip_i_mongo_store, puerto_i_mongo_store);
	//pthread_mutex_unlock(&mutex_comunicacion);
	eliminar_paquete(paquete);
	free(tid);
}

void avisar_imongo_bitacora_tarea(char* operacion, uint32_t id, char* tarea, uint32_t cantidad) {

	char* tid = (char*) string_itoa(id);
	char* cant = (char*) string_itoa(cantidad);

	t_paquete* paquete = crear_paquete();

	agregar_a_paquete(paquete, operacion, strlen(operacion) + 1);
	agregar_a_paquete(paquete, tarea, strlen(tarea) + 1);
	agregar_a_paquete(paquete, tid, strlen(tid) + 1);
	agregar_a_paquete(paquete, cant, strlen(cant) + 1);

	//pthread_mutex_lock(&mutex_comunicacion);
	enviar_paquete_por_conexion(paquete, ip_i_mongo_store, puerto_i_mongo_store);
	//pthread_mutex_unlock(&mutex_comunicacion);
	eliminar_paquete(paquete);
	free(tid);
	free(cant);
}

void avisar_mi_ram_estado(uint32_t id, char estado) {

	char* tid = (char*) string_itoa(id);
	char* est = malloc(2);
	est[0] = estado;
	est[1] = '\0';

	t_paquete* paquete = crear_paquete();

	agregar_a_paquete(paquete, "ACTUALIZAR_ESTADO", strlen("ACTUALIZAR_ESTADO") + 1);
	agregar_a_paquete(paquete, tid, strlen(tid) + 1);
	agregar_a_paquete(paquete, est, 2);

	if(id == 7 || id == 8)log_info(logger, "tripulante %d avisa cambio de estado", id);
	//pthread_mutex_lock(&mutex_comunicacion);
	enviar_paquete_por_conexion(paquete,ip_mi_ram_hq,puerto_mi_ram_hq);
	//pthread_mutex_unlock(&mutex_comunicacion);
	if(id == 7 || id == 8)log_info(logger, "tripulante %d cambio de estado", id);
	eliminar_paquete(paquete);
	free(tid);
	free(est);
}

void mover_tripulante_a_destino(t_tripulante* tripulante, t_posicion destino) {
	while (tripulante->posicion.x != destino.x) {

		if (tripulante->posicion.x < destino.x) {
			mover_tripulante(tripulante, DERECHA);
		} else {
			mover_tripulante(tripulante, IZQUIERDA);
		}
	}
	while (tripulante->posicion.y != destino.y) {

		if (tripulante->posicion.y < destino.y) {
			mover_tripulante(tripulante, ABAJO);
		} else {
			mover_tripulante(tripulante, ARRIBA);
		}
	}
}

void mover_planificando_a_destino(t_planificando* planificando, t_posicion destino) {
	while (planificando->tripulante->posicion.x != destino.x || planificando->tripulante->posicion.y != destino.y) {
		while (planificando->tripulante->posicion.x != destino.x) {

			wait_signal_pausar_planificando(planificando);
			if(planificando->tripulante->expulsado){
				break;
			}

			if (planificando->tripulante->posicion.x < destino.x) {
				mover_tripulante(planificando->tripulante, DERECHA);
			} else {
				mover_tripulante(planificando->tripulante, IZQUIERDA);
			}
		}
		while (planificando->tripulante->posicion.y != destino.y) {

			wait_signal_pausar_planificando(planificando);
			if(planificando->tripulante->expulsado){
				break;
			}

			if (planificando->tripulante->posicion.y < destino.y) {
				mover_tripulante(planificando->tripulante, ABAJO);
			} else {
				mover_tripulante(planificando->tripulante, ARRIBA);
			}
		}
	}
}

void mover_tripulante(t_tripulante* tripulante, uint32_t direccion) {

	switch (direccion) {
	case ARRIBA:
		tripulante->posicion.y--;
		break;
	case ABAJO:
		tripulante->posicion.y++;
		break;
	case DERECHA:
		tripulante->posicion.x++;
		break;
	case IZQUIERDA:
		tripulante->posicion.x--;
		break;
	default:
		break;
	}
	sleep(retardo_ciclo_cpu);

	avisar_servidores_movimiento_del_tripulante(tripulante->id, direccion);
}

void mover_al_estilo_round_robin(t_planificando * planificando, t_posicion destino){
	while (planificando->tripulante->posicion.x != destino.x || planificando->tripulante->posicion.y != destino.y) {
		while (planificando->tripulante->posicion.x != destino.x) {

			wait_signal_pausar_planificando(planificando);
			if(planificando->tripulante->expulsado)break;

			if (planificando->tripulante->posicion.x < destino.x) {

				mover_tripulante(planificando->tripulante, DERECHA);
			} else {
				mover_tripulante(planificando->tripulante, IZQUIERDA);
			}

			planificando->tripulante->quantum_disponible --;
			planificando->tripulante->rafaga_faltante --;

			if(!planificando->tripulante->quantum_disponible)replanifica(planificando);
		}
		while (planificando->tripulante->posicion.y != destino.y) {

			wait_signal_pausar_planificando(planificando);
			if(planificando->tripulante->expulsado)break;

			if (planificando->tripulante->posicion.y < destino.y) {
				mover_tripulante(planificando->tripulante, ABAJO);
			} else {
				mover_tripulante(planificando->tripulante, ARRIBA);
			}

			planificando->tripulante->quantum_disponible --;
			planificando->tripulante->rafaga_faltante --;

			if(!planificando->tripulante->quantum_disponible)replanifica(planificando);
		}
	}
}

int recibir_operacion(int socket_cliente) {
	int cod_op;
	if (recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) != 0)
		return cod_op;
	else {
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
	//TODO: Necesitamos hacer un free de esta lista creada.
	return valores;
	return NULL;
}
