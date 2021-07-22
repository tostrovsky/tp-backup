#include "bitacoras.h"

void crear_archivo_bitacora(uint32_t tid){
	char* path = generar_path_bitacora(tid);
	char* id = string_itoa(tid);

	if(!existe_archivo(path)){
		FILE* file_bitacora = fopen(path,"wb");

		if (file_bitacora == NULL) log_warning(logger,"No fue posible crear el archivo %s",path);

			fputs("SIZE=0\n",file_bitacora);
			fputs("BLOCKS=\n",file_bitacora);

			fclose(file_bitacora);

		metadata_bitacora* metadata_t = malloc(sizeof(metadata_bitacora));
		metadata_t->blocks = string_new();
		metadata_t->size = 0;
		metadata_t->offset = 0;
		metadata_t->block_count = 0;
		dictionary_put(metadatas_bitacora, id , metadata_t);

	}else{
		t_config* file = config_create(path);

		metadata_bitacora* metadata_t = malloc(sizeof(metadata_bitacora));
		metadata_t->blocks = string_new();
		string_append(&metadata_t->blocks, config_get_string_value(file, "BLOCKS"));
		metadata_t->size = config_get_int_value(file, "SIZE");
		metadata_t->block_count = token_count(metadata_t->blocks, ',');
		metadata_t->offset = metadata_t->size - (metadata_t->block_count-1)*superbloque.block_size;
		char* nro_ult_bloque = get_token_at(metadata_t->blocks, ',', token_count(metadata_t->blocks, ',')-1);
		metadata_t->nro_ultimo_bloque = atoi(nro_ult_bloque);
		free(nro_ult_bloque);
		dictionary_put(metadatas_bitacora, id , metadata_t);
		config_destroy(file);
	}
	free(id);
	free(path);

}

void cargar_en_bitacora(t_list* lista){
	char* accion = string_new();
	uint32_t codigo_operacion = obtener_codigo_de_operacion(list_get(lista, 0));
	int tid;

	switch (codigo_operacion) {

		case ATENDER_SABOTAJE :
			string_append(&accion, "Atendio sabotaje en ");
			string_append(&accion, list_get(lista, 2));
			tid = (int)atoi(list_get(lista, 1));
			list_clean_and_destroy_elements(lista, free);
			break;

		case RESOLVER_SABOTAJE :
			string_append(&accion, "Resolvio sabotaje en ");
			string_append(&accion, list_get(lista, 2));
			tid = (int)atoi(list_get(lista, 1));
			list_clean_and_destroy_elements(lista, free);
			break;

		case COMENZAR :
			string_append(&accion, "Comenzo a ");
			string_append(&accion, list_get(lista, 1));
			tid = (int)atoi(list_get(lista, 2));
			list_clean_and_destroy_elements(lista, free);
			break;

		case FINALIZAR :
			string_append(&accion, "Termino de ");
			string_append(&accion, list_get(lista, 1));
			tid = (int)atoi(list_get(lista, 2));
			list_clean_and_destroy_elements(lista, free);
			break;

		case MOVER :
			string_append(&accion, "Se movio hacia ");
			string_append(&accion, list_get(lista, 2));
			string_append(&accion, "|");
			string_append(&accion, list_get(lista, 3));
			tid = (int)atoi(list_get(lista, 1));
			list_clean_and_destroy_elements(lista, free);
			break;

		case INICIAR_TRIPULANTE :
			string_append(&accion, "Inicio");
			tid = (int)atoi(list_get(lista, 1));
			free(list_get(lista, 1));
			break;

		case EXPULSAR_TRIPULANTE :
			string_append(&accion, "Fue expulsado");
			tid = (int)atoi(list_get(lista, 1));
			list_clean_and_destroy_elements(lista, free);
			break;
		default:
			log_warning(logger, "Operacion incorrecta");
			return;
	}
	list_destroy(lista);

	string_append(&accion, "\n");

	cargar_accion(tid, accion);
}

void cargar_accion(int tid, char* accion){
	char* path = generar_path_bitacora(tid);
	char* id = string_itoa(tid);

	pthread_mutex_lock(&mutex_bitacora[tid-1]);
	metadata_bitacora* metadata_t = dictionary_get(metadatas_bitacora, id);

	for(int i = 0; i < strlen(accion); i++){
		if(metadata_t->offset % superbloque.block_size == 0)asignar_bloque_en_metadata_bitacora(tid);

		metadata_t->posicion_bloque = metadata_t->nro_ultimo_bloque*superbloque.block_size + metadata_t->offset;

		t_config* metadata = config_create(path);
		metadata_t->offset++;
		metadata_t->size++;
		char* metadata_size = string_itoa(metadata_t->size);

		config_set_value(metadata, "SIZE", metadata_size);
		config_save(metadata);
		config_destroy(metadata);
		free(metadata_size);
		memcpy(copia_mapeo+metadata_t->posicion_bloque,(accion + i),1);

	}
	pthread_mutex_unlock(&mutex_bitacora[tid-1]);
	free(id);
	free(path);
	free(accion);

}

void actualizar_metadata_bitacora(int nro_bloque,int tid){
	char* path = generar_path_bitacora(tid);
	char* id = string_itoa(tid);

	metadata_bitacora* metadata_t = dictionary_get(metadatas_bitacora, id);
	t_config* metadata = config_create(path);

	if(string_is_empty(metadata_t->blocks)){
		string_append_with_format(&metadata_t->blocks,"%d",nro_bloque);
	}else {
		string_append_with_format(&metadata_t->blocks,",%d",nro_bloque);
	}

	metadata_t->offset = 0;
	metadata_t->nro_ultimo_bloque = nro_bloque;
	metadata_t->block_count ++;

	config_set_value(metadata, "BLOCKS", metadata_t->blocks);
	config_save(metadata);
	config_destroy(metadata);
	free(path);
	free(id);
}

void loggear_bitacora_del_tripulante(int tid){
	char* id = string_itoa(tid);

	metadata_bitacora* metadata_t = dictionary_get(metadatas_bitacora, id);
	pthread_mutex_lock(&mutex_bitacora[tid-1]);
	char* cadena_tokenizada = metadata_t->blocks;
	char* bloque_numero;
	for(int i=0;i<token_count(cadena_tokenizada, ',');i++){
		bloque_numero = get_token_at(cadena_tokenizada,',',i);
		loggear_bloque_numero(atoi(bloque_numero));
		free(bloque_numero);
	}
	pthread_mutex_unlock(&mutex_bitacora[tid-1]);
	free(id);
}

void loggear_bloque_numero(int nro){
	char* loggeable = malloc(superbloque.block_size + 1);
	memcpy(loggeable, copia_mapeo + superbloque.block_size * nro, superbloque.block_size);
	loggeable[superbloque.block_size] = '\0';
	log_info(logger,loggeable);
	free(loggeable);
}





