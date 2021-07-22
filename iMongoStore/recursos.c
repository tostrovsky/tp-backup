#include"recursos.h"

void crear_archivo_recurso(int recurso){
	char* path = generar_path_metadata(recurso);

	if(!existe_archivo(path)){
		FILE* file_recurso;
		file_recurso= fopen(path,"wb");
		if (file_recurso == NULL){
			log_warning(logger,"No fue posible crear el archivo %s",path);
		}else{
			fputs("SIZE=0\n",file_recurso);
			fputs("BLOCK_COUNT=0\n",file_recurso);
			fputs("BLOCKS=\n",file_recurso);

			switch(recurso){
				case GENERAR_OXIGENO:
				case CONSUMIR_OXIGENO:
				fputs("CARACTER_LLENADO=O\n",file_recurso);
				break;
				case GENERAR_COMIDA:
				case CONSUMIR_COMIDA:
				fputs("CARACTER_LLENADO=C\n",file_recurso);
				break;
				case GENERAR_BASURA:
				case DESCARTAR_BASURA:
				fputs("CARACTER_LLENADO=B\n",file_recurso);
				break;
			}
			fputs("MD5_ARCHIVO=",file_recurso);
			fclose(file_recurso);

			metadata_recurso* metadata_r = struct_metadata_recurso(recurso);
			metadata_r->size = 0;
			metadata_r->block_count = 0;
			metadata_r->offset = 0;
		}

	}
	/*else{

		t_config* file = config_create(path);
		metadata_recurso* metadata_r = struct_metadata_recurso(recurso);

		string_append(&metadata_r->blocks, config_get_string_value(file, "BLOCKS"));
		log_warning(logger, "entre al else %s", metadata_r->blocks );

		metadata_r->size = config_get_int_value(file, "SIZE");

		metadata_r->block_count = config_get_int_value(file, "BLOCK_COUNT");

		strcpy(metadata_r->md5, config_get_string_value(file, "MD5_ARCHIVO"));

		metadata_r->offset = metadata_r->size - (metadata_r->block_count - 1) * superbloque.block_size ;
		char* nro_ult_bloque = get_token_at(metadata_r->blocks, ',', token_count(metadata_r->blocks, ',')-1);
		metadata_r->nro_ultimo_bloque = atoi(nro_ult_bloque);
		free(nro_ult_bloque);

		config_destroy(file);
		//size = (block_count -1)* block_size + offset
	}*/

	free(path);
// size 33
// 3 bloques llenos + un bloque con 3
}

metadata_recurso* struct_metadata_recurso(int recurso){
	metadata_recurso* metadata;

		switch(recurso){
			case GENERAR_OXIGENO:
			case CONSUMIR_OXIGENO: metadata = metadata_o;
			break;

			case GENERAR_COMIDA:
			case CONSUMIR_COMIDA: metadata = metadata_c;
			break;

			case GENERAR_BASURA:
			case DESCARTAR_BASURA: metadata = metadata_b;
			break;

			default:
			break;
		}

		return metadata;
}


char* caracter_mensaje(int recurso){
	char* caracter;

	switch(recurso){
		case GENERAR_OXIGENO:
		case CONSUMIR_OXIGENO: caracter = "O";
		break;

		case GENERAR_COMIDA:
		case CONSUMIR_COMIDA: caracter = "C";
		break;

		case GENERAR_BASURA:
		case DESCARTAR_BASURA: caracter = "B";
		break;

		default:
		break;
		}

	return caracter;

}

// INICIO - Bloque asignado/desasignado //

void actualizar_metadata_recurso(int nro_bloque,int recurso){
	char* path = generar_path_metadata(recurso);
	metadata_recurso* metadata_r = struct_metadata_recurso(recurso);

	t_config* metadata = config_create(path);

	if(string_is_empty(metadata_r->blocks)) string_append_with_format(&metadata_r->blocks,"%d", nro_bloque);
	else string_append_with_format(&metadata_r->blocks, ",%d", nro_bloque);

	metadata_r->block_count ++;
	metadata_r->offset = 0;
	metadata_r->nro_ultimo_bloque = nro_bloque;

	char* metadata_r_block_count = string_itoa(metadata_r->block_count);
	config_set_value(metadata, "BLOCK_COUNT", metadata_r_block_count);
	config_set_value(metadata, "BLOCKS", metadata_r->blocks);
	config_save(metadata);
	config_destroy(metadata);
	free(metadata_r_block_count);
	free(path);
}

int des_actualizar_metadata_recurso(int recurso){
	char* path = generar_path_metadata(recurso);
	metadata_recurso* metadata_r = struct_metadata_recurso(recurso);
	t_config* metadata = config_create(path);

	int nro_bloque_desasignado = metadata_r->nro_ultimo_bloque;
	char* saca_tokens = remove_token_at(&metadata_r->blocks, ',', token_count(metadata_r->blocks, ',')-1);
	char* nro = get_token_at(metadata_r->blocks, ',', token_count(metadata_r->blocks, ',')-1);

	metadata_r->block_count --;
	metadata_r->offset = superbloque.block_size;
	metadata_r->nro_ultimo_bloque = atoi(nro);

	char* metadata_r_block_count = string_itoa(metadata_r->block_count);
	config_set_value(metadata, "BLOCK_COUNT", metadata_r_block_count);
	config_set_value(metadata, "BLOCKS", metadata_r->blocks);

	config_save(metadata);
	config_destroy(metadata);
	return nro_bloque_desasignado;

	free(path);
	free(nro);
	free(saca_tokens);
}

// FIN - Bloque asignado/desasignado //

// INICIO - Carga y Descarga de recurso //

void cargar_recurso(int mensaje, int cantidad){
	char* path = generar_path_metadata(mensaje);;
	char* caracter_llenado = caracter_mensaje(mensaje);

	pthread_mutex_lock(&mutex_recurso[mensaje]);
	if(!existe_archivo(path)){
		crear_archivo_recurso(mensaje);
	}
	pthread_mutex_unlock(&mutex_recurso[mensaje]);

	metadata_recurso* metadata_r = struct_metadata_recurso(mensaje);

	pthread_mutex_lock(&mutex_recurso[mensaje]);
	for(int i = 1; i <= cantidad; i++){

		if(metadata_r->offset % superbloque.block_size == 0) asignar_bloque_en_metadata_recurso(mensaje);

		metadata_r->posicion_bloque = metadata_r->nro_ultimo_bloque*superbloque.block_size + metadata_r->offset;
		metadata_r->offset++;
		metadata_r->size++;
		generar_md5(metadata_r);

		t_config* metadata = config_create(path);
		char* metadata_r_size = string_itoa(metadata_r->size);
		config_set_value(metadata, "SIZE", metadata_r_size);
		config_set_value(metadata, "MD5_ARCHIVO", metadata_r->md5);
		config_save(metadata);
		config_destroy(metadata);
		free(metadata_r_size);
		memcpy(copia_mapeo+metadata_r->posicion_bloque,caracter_llenado,1);
	}
	pthread_mutex_unlock(&mutex_recurso[mensaje]);
	free(path);
}


void descargar_recurso(int mensaje, int cantidad){
	char* path = generar_path_metadata(mensaje);
	metadata_recurso* metadata_r = struct_metadata_recurso(mensaje);

	pthread_mutex_lock(&mutex_recurso[mensaje-3]);
	for(int i = 1; i <= cantidad; i++){

		if(metadata_r->offset == 0){
			des_asignar_bloque_en_metadata_recurso(mensaje);
		}

		metadata_r->offset--;
		metadata_r->size--;
		generar_md5(metadata_r);

		t_config* metadata = config_create(path);
		char* metadata_r_size = string_itoa(metadata_r->size);
		config_set_value(metadata, "SIZE", metadata_r_size);
		config_set_value(metadata, "MD5_ARCHIVO", metadata_r->md5);
		config_save(metadata);
		config_destroy(metadata);
		free(metadata_r_size);
		metadata_r->posicion_bloque = metadata_r->nro_ultimo_bloque*superbloque.block_size + metadata_r->offset;
		memcpy(copia_mapeo+metadata_r->posicion_bloque," ",1);
	}

	if(metadata_r->offset == 0){
		des_asignar_bloque_en_metadata_recurso(mensaje);
	}
	pthread_mutex_unlock(&mutex_recurso[mensaje-3]);
	free(path);
}

// FIN - Carga y Descarga de recurso //

void eliminar_archivo(char* nombre){
	if(!existe_archivo(nombre))
		log_warning(logger, "No existe Basura.ims");
	else{
		descargar_recurso(DESCARTAR_BASURA, metadata_b->size);
		remove(nombre);
	}
}

// INICIO - MD5 //

void generar_md5(metadata_recurso* metadata_r){
	char* path = generar_path_md5();
	char* md5 = string_new();
	char* blocks_string = cadena_de_blocks(metadata_r);

	FILE* md5_file;

	if(!existe_archivo(path)){
		md5_file = fopen(path,"wb");
	}else md5_file = fopen(path,"r+b");

	string_append(&md5,"echo ");
	string_append(&md5, "\"");
	string_append(&md5,blocks_string);
	string_append(&md5, "\"");
	string_append(&md5," | md5sum");
	string_append(&md5," > ");
	string_append(&md5,path);
	system(md5);

	fgets(metadata_r->md5,33,md5_file);

	free(md5);
	free(path);
	free(blocks_string);

	fclose(md5_file);
}

char* cadena_de_blocks(metadata_recurso* metadata_r){
	return string_repeat(metadata_r->caracter_llenado, metadata_r->size);
}


char* generar_path_md5(){
	char* path_md5 = string_new();
	char* nos_faltaba_este_free = generar_path_files();
	string_append(&path_md5, nos_faltaba_este_free );
	string_append(&path_md5, "/md5.txt");

	free(nos_faltaba_este_free);
	return path_md5;
}

// FIN - MD5 //

// INICIO - BLOQUES A STRING //

char* leer_bloques_recurso(metadata_recurso* metadata_r){
	//char* cadena_tokenizada = string_new();
	//string_append(&cadena_tokenizada, metadata_r->blocks);
	char* bloque_to_string = string_new();
	char* leer_bloque_nro;

	for(int i=0;i<token_count(metadata_r->blocks, ',');i++){

		int num = obtener_numero_de_bloque(metadata_r->blocks, i);
		leer_bloque_nro = malloc(superbloque.block_size);
		memcpy(leer_bloque_nro, copia_mapeo + superbloque.block_size * num, superbloque.block_size);
		string_trim(&leer_bloque_nro);
		string_append(&bloque_to_string,leer_bloque_nro);

		free(leer_bloque_nro);
	}
	//free(cadena_tokenizada);
	return bloque_to_string;
}

int obtener_numero_de_bloque(char* cadena_tokenizada, int posicion){

	char* bloque_numero= get_token_at(cadena_tokenizada,',',posicion);
	int num = atoi(bloque_numero);
	free(bloque_numero);
	return num;
}

// FIN - BLOQUES A STRING //
