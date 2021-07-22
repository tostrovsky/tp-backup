#include "filesystem.h"

/* INICIO - Auxiliares */

char* bitarray_to_chararray() {
	char* chararray = string_new();

	for (int i = 0; i < superbloque.blocks; i++) {
		if (bitarray_test_bit(superbloque.bitmap, i)) {
			string_append(&chararray, "1");
		} else {
			string_append(&chararray, "0");
		}
	}
	return chararray;
}


t_bitarray* chararray_to_bitarray(char* chararray){
	char array[superbloque.blocks];
	strcpy(array, chararray);
	char* inicial = calloc(superbloque.blocks, sizeof(char));
	t_bitarray* bitarray = bitarray_create_with_mode(inicial, superbloque.blocks / 8, MSB_FIRST);

	for(int i=0;i<superbloque.blocks;i++){
		if (array[i] == '1') {
			bitarray_set_bit(bitarray, i);
		} else {
			bitarray_clean_bit(bitarray, i);
		}
	}
	return bitarray;
}

bool existe_FS() {
	bool ret;
	char* path_sb = generar_path_superbloque();
	char* path_bs = generar_path_blocks();
	ret = existe_archivo(path_sb) && existe_archivo(path_bs);
	free(path_sb);
	free(path_bs);
	return ret;
}

bool existe_archivo(char* path) {
	FILE * archivo = fopen(path, "rb");
	if (archivo != NULL) {
		fclose(archivo);
		return true;
	}
	//fclose(archivo);
	return false;
}

/* FIN - Auxiliares */
/*
int crear_todos_archivos() {
	// PARA PROBAR:

	char* iniciar = string_new();
	char* id = string_new();
	string_append(&iniciar, "INICIAR_TRIPULANTE");
	string_append(&id, "1");

	t_list* inicio = list_create();
	list_add(inicio,iniciar );
	list_add(inicio, id);

	t_list* ob_bit = list_create();
	list_add(ob_bit, "OBTENER_BITACORA");
	list_add(ob_bit, "1");

	t_list* g_basura = list_create();
	list_add(g_basura, "COMENZAR");
	list_add(g_basura, "GENERAR_BASURA");
	list_add(g_basura, "1");
	list_add(g_basura, "13");

	t_list* g_oxigeno = list_create();
	list_add(g_oxigeno, "COMENZAR");
	list_add(g_oxigeno, "GENERAR_OXIGENO");
	list_add(g_oxigeno, "1");
	list_add(g_oxigeno, "14");

	t_list* d_basura = list_create();
	list_add(d_basura, "COMENZAR");
	list_add(d_basura, "DESCARTAR_BASURA");
	list_add(d_basura, "1");
	list_add(d_basura, "0");

	t_list* c_oxigeno = list_create();
	list_add(c_oxigeno, "COMENZAR");
	list_add(c_oxigeno, "CONSUMIR_OXIGENO");
	list_add(c_oxigeno, "1");
	list_add(c_oxigeno, "5");

	crear_punto_montaje();
	log_info(logger, "hola-4");
	crear_archivo_superbloque();
	log_info(logger, "hola-3");
	int blocks_fd = crear_blocks();

	log_info(logger, "hola-2");
	crear_carpeta_files();
	log_info(logger, "hola-1");
	crear_carpeta_bitacoras();

	log_info(logger, "hola0");
	crear_archivo_bitacora(1);
	//crear_archivo_bitacora(2);
	log_info(logger, "holahola");
	cargar_en_bitacora(inicio);
	log_info(logger, "hola1");

	sleep(12);
	realizar_tarea(g_basura);
	log_info(logger, "hola2");
	realizar_tarea(g_oxigeno);
	log_info(logger, "hola3");
	realizar_tarea(d_basura);
	log_info(logger, "hola4");
	realizar_tarea(g_oxigeno);
	log_info(logger, "hola5");
	sleep(50);
	protocolo_fsck();
	realizar_tarea(g_oxigeno);
	realizar_tarea(c_oxigeno);
	realizar_tarea(g_basura);
	cargar_en_bitacora(g_basura);
	return blocks_fd;
}
*/
// la funcion postaposta

int crear_todos_archivos() {

	crear_punto_montaje();
	crear_archivo_superbloque();
	int blocks_fd = crear_blocks();

	crear_carpeta_files();
	crear_carpeta_bitacoras();

	return blocks_fd;
}



// INICIO - CREAR PATHS //

char* generar_path_superbloque() {
	char* path_superbloque = string_new();
	string_append(&path_superbloque, i_mongo_store.punto_montaje);
	string_append(&path_superbloque, "/SuperBloque.ims");

	return path_superbloque;
}

char* generar_path_blocks() {
	char* path_blocks = string_new();
	string_append(&path_blocks, i_mongo_store.punto_montaje);
	string_append(&path_blocks, "/Blocks.ims");

	return path_blocks;
}

char* generar_path_files() {
	char* path_files = string_new();
	string_append(&path_files, i_mongo_store.punto_montaje);
	string_append(&path_files, "/Files");

	return path_files;
}

char* generar_path_metadata(int recurso) {
	char* path_recurso = string_new();
	string_append(&path_recurso, i_mongo_store.punto_montaje);
	string_append(&path_recurso, "/Files");

	switch (recurso) {
	case GENERAR_OXIGENO:
	case CONSUMIR_OXIGENO:
		string_append(&path_recurso, "/Oxigeno.ims");
		break;
	case GENERAR_COMIDA:
	case CONSUMIR_COMIDA:
		string_append(&path_recurso, "/Comida.ims");
		break;
	case GENERAR_BASURA:
	case DESCARTAR_BASURA:
		string_append(&path_recurso, "/Basura.ims");
		break;
	default:
		break;
	}

	return path_recurso;
}

char* generar_path_bitacoras() {
	char* path_bitacoras = string_new();
	char* path_files = generar_path_files();
	string_append(&path_bitacoras, path_files);
	string_append(&path_bitacoras, "/Bitacoras");

	free(path_files);
	return path_bitacoras;
}

char* generar_path_bitacora(uint32_t tid) {
	char* path_bitacora = string_new();
	char* path_generado = generar_path_bitacoras();
	string_append(&path_bitacora, path_generado);
	string_append(&path_bitacora, "/Tripulante");
	char* tripulante = string_itoa(tid);
	string_append(&path_bitacora, tripulante);
	string_append(&path_bitacora, ".ims");
	free(tripulante);
	free(path_generado);

	return path_bitacora;
}

// INICIO - CREAR DIRECTORIOS //

void crear_punto_montaje() {
	char* path = string_new();
	string_append(&path, i_mongo_store.punto_montaje);
	if(!existe_archivo(path)){
		mkdir(path, 0777);
	}
	free(path);
}

void crear_carpeta_files() {
	char* path_files = generar_path_files();
	if(!existe_archivo(path_files)){
		mkdir(path_files, 0777);
	}
	free(path_files);
}

void crear_carpeta_bitacoras() {
	char* path_bitacoras = generar_path_bitacoras();
	if(!existe_archivo(path_bitacoras)){
		mkdir(path_bitacoras, 0777);
	}
	free(path_bitacoras);
}

// FIN - CREAR DIRECTORIOS //


// INICIO - SUPERBLOQUE.IMS //

void crear_archivo_superbloque() {
	char* path = generar_path_superbloque();
	FILE* file_superbloque;

	if (!existe_archivo(path)) {
		file_superbloque = fopen(path, "wb");

		if (file_superbloque == NULL){
			log_warning(logger, "No fue posible crear el archivo %s", path);
		}
		else {
			fputs("BLOCK_SIZE=\n", file_superbloque);
			fputs("BLOCKS=\n", file_superbloque);
			fputs("BITMAP=\n", file_superbloque); // bitmap deberia ser distinto creo
		}

		fclose(file_superbloque);

		/// mapeo info de SUPERBLOQUE ///
		superbloque.block_size = config_get_int_value(config, "BLOCK_SIZE");
		superbloque.blocks = config_get_int_value(config, "BLOCKS");
		char* bitarray = calloc(superbloque.blocks, sizeof(char));
		superbloque.bitmap = bitarray_create_with_mode(bitarray, superbloque.blocks / 8, MSB_FIRST);


		t_config* superbloque_file = config_create(path);

		char* size = string_itoa(superbloque.block_size);
		config_set_value(superbloque_file, "BLOCK_SIZE", size);
		char* blocks = string_itoa(superbloque.blocks);
		config_set_value(superbloque_file, "BLOCKS", blocks);
		char* bicharey = bitarray_to_chararray();
		config_set_value(superbloque_file, "BITMAP", bicharey);
		config_save(superbloque_file);
		config_destroy(superbloque_file);

		free(bicharey);
		free(size);
		free(blocks);

	}else{
		t_config* superbloque_file = config_create(path);
		superbloque.block_size = config_get_int_value(superbloque_file, "BLOCK_SIZE");
		superbloque.blocks = config_get_int_value(superbloque_file, "BLOCKS");

		char* bismaps = config_get_string_value(superbloque_file, "BITMAP");
		superbloque.bitmap = chararray_to_bitarray(bismaps);
		config_destroy(superbloque_file);

	}
	free(path);
}

void actualizar_metadata_superbloque(int nro_bloque) {
	bitarray_set_bit(superbloque.bitmap, nro_bloque);
	char* martu_mattioli12 = bitarray_to_chararray();
	char* path = generar_path_superbloque();
	t_config* metadata_s = config_create(path);
	config_set_value(metadata_s, "BITMAP", martu_mattioli12);
	config_save(metadata_s);
	config_destroy(metadata_s);
	free(martu_mattioli12);
	free(path);
}

void des_actualizar_metadata_superbloque(int nro_bloque) {
	bitarray_clean_bit(superbloque.bitmap, nro_bloque);
	char* path = generar_path_superbloque();
	char* martu_mattioli12 = bitarray_to_chararray();
	t_config* metadata_s = config_create(path);
	config_set_value(metadata_s, "BITMAP", bitarray_to_chararray());
	config_save(metadata_s);
	config_destroy(metadata_s);
	free(martu_mattioli12);
	free(path);
}

// FIN - SUPERBLOQUE.IMS //

// INICIO - Des/Asignacion de bloques //

int buscar_primer_bloque_libre() {
	int i = 0;
	while (i < superbloque.blocks) {

		if (!bitarray_test_bit(superbloque.bitmap, i)) {
			return i;
		}
		i++;
	}

	return -1;
}

int asignar_bloque_en_metadata_bitacora(int tid) {
	int nro_bloque = buscar_primer_bloque_libre();
	actualizar_metadata_bitacora(nro_bloque, tid);
	actualizar_metadata_superbloque(nro_bloque);
	return nro_bloque;
}

int asignar_bloque_en_metadata_recurso(int recurso) {
	int nro_bloque = buscar_primer_bloque_libre();
	actualizar_metadata_recurso(nro_bloque, recurso);
	actualizar_metadata_superbloque(nro_bloque);
	return nro_bloque;
}

void des_asignar_bloque_en_metadata_recurso(int recurso) {
	int nro_bloque = des_actualizar_metadata_recurso(recurso);
	des_actualizar_metadata_superbloque(nro_bloque);
}

// FIN - Des/Asignacion de bloques //


// BLOCKS.IMS //

void recuperar_archivo_si_y_solo_si_hay_que_recuperarlos(int mensaje){

	char* path = generar_path_metadata(mensaje);

	if(existe_archivo(path)){
			t_config* file = config_create(path);
			metadata_recurso* metadata_r = struct_metadata_recurso(mensaje);

			string_append(&metadata_r->blocks, config_get_string_value(file, "BLOCKS"));
			//log_info(logger, "Bloques recuperados: %s", metadata_r->blocks );

			metadata_r->size = config_get_int_value(file, "SIZE");

			metadata_r->block_count = config_get_int_value(file, "BLOCK_COUNT");

			strcpy(metadata_r->md5, config_get_string_value(file, "MD5_ARCHIVO"));

			metadata_r->offset = metadata_r->size - (metadata_r->block_count - 1) * superbloque.block_size ;
			char* nro_ult_bloque = get_token_at(metadata_r->blocks, ',', token_count(metadata_r->blocks, ',')-1);
			metadata_r->nro_ultimo_bloque = atoi(nro_ult_bloque);
			free(nro_ult_bloque);

			config_destroy(file);
	}

	free(path);
}


int crear_blocks() {
	int blocks_fd = mapear_blocks();
	copiar_mapeo();

	inicializar_semaforos();

	pthread_t hilo_sync;
	pthread_t hilo_sleep;
	pthread_create(&hilo_sync, NULL, sincronizar_file_blocks, NULL);
	pthread_create(&hilo_sleep, NULL, esperar_sincronizacion, NULL);
	return blocks_fd;
}

int mapear_blocks() {
	char* path = generar_path_blocks();
	size_t size = superbloque.block_size * superbloque.blocks;
	int blocks_fd;

	if(!existe_archivo(path)){
	    blocks_fd = open(path, O_CREAT | O_RDWR, (mode_t) 0777);
		ftruncate(blocks_fd, size);
			char* espacio = " ";
			for (int i = 0; i < size; i++) {
				write(blocks_fd, espacio, 1);
			}
	}else{
		blocks_fd = open(path, O_RDWR, (mode_t) 0777);
	}

	blocks_mapeado = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, blocks_fd, 0);

	if (blocks_mapeado == MAP_FAILED) log_warning(logger, "No fue posible mapear el archivo blocks");

	free(path);
	return blocks_fd;
}

void copiar_mapeo() {
	size_t size = superbloque.block_size * superbloque.blocks;
	copia_mapeo = malloc(size);
	memcpy(copia_mapeo, blocks_mapeado, size);
}

void inicializar_semaforos() {
	sem_init(&sem_sleep, 0, 1);
	sem_init(&sem_sync, 0, 0);
}

void* sincronizar_file_blocks() {
	size_t size = superbloque.block_size * superbloque.blocks;

	while (1) {
		sem_wait(&sem_sync);
		log_info(logger, "SINCRONIZANDO..");

		memcpy(blocks_mapeado, copia_mapeo, size);
		//*(char*)blocks_mapeado = ' ';
		int i = msync(blocks_mapeado, size, MS_SYNC);
		if (!(i + 1))log_warning(logger, "No fue posible realizar la sincronizacion");

		//close(blocks_fd);
		sem_post(&sem_sleep);
	}

	return EXIT_SUCCESS;
}

void* esperar_sincronizacion() {
	while (1) {
		sem_wait(&sem_sleep);
		sleep(i_mongo_store.tiempo_sincronizacion);
		sem_post(&sem_sync);
	}
	return EXIT_SUCCESS;
}

// INICIO - FSCK //

void protocolo_fsck(){
	log_info(logger,"Comenzo protocolo FSCK");
	int sabotaje = identificar_sabotaje();
	resolver_sabotaje(sabotaje);
	log_info(logger,"Protocolo FSCK finalizo exitosamente");
}

int identificar_sabotaje(){
	char* path = generar_path_superbloque();
	int tipo_sabotaje;
	t_config* superbloque_file = config_create(path);
	char* un_bitmap = bitarray_to_chararray(superbloque.bitmap);
	char* otro_bitmap = config_get_string_value(superbloque_file, "BITMAP");

	if(file_blocks_inconsistente(GENERAR_OXIGENO)){
		log_warning(logger, "SABOTAJE DE BLOCKS en Oxigeno.ims");
		tipo_sabotaje = SABOTAJE_BLOCKS_O;
	}else
	if(file_blocks_inconsistente(GENERAR_COMIDA)){
		log_warning(logger, "SABOTAJE DE BLOCKS en Comida.ims");
		tipo_sabotaje = SABOTAJE_BLOCKS_C;
	}else
	if(file_blocks_inconsistente(GENERAR_BASURA)){
		log_warning(logger, "SABOTAJE DE BLOCKS en Basura.ims");
		tipo_sabotaje = SABOTAJE_BLOCKS_B;
	}else
	if(size_files_incosistente(GENERAR_OXIGENO)){
		log_warning(logger, "SABOTAJE DE SIZE en Oxigeno.ims");
		tipo_sabotaje = SABOTAJE_SIZE_O;
	}else
	if(size_files_incosistente(GENERAR_COMIDA)){
		log_warning(logger, "SABOTAJE DE SIZE en Comida.ims");
		tipo_sabotaje = SABOTAJE_SIZE_C;
	}else
	if(size_files_incosistente(GENERAR_BASURA)){
		log_warning(logger, "SABOTAJE DE SIZE en Basura.ims");
		tipo_sabotaje = SABOTAJE_SIZE_B;
	}else
	if(block_count_incosistente(GENERAR_OXIGENO)){
		log_warning(logger, "SABOTAJE DE BLOCK COUNT en Oxigeno.ims");
		tipo_sabotaje = SABOTAJE_BLOCK_COUNT_O;
	}else
	if(block_count_incosistente(GENERAR_COMIDA)){
		log_warning(logger, "SABOTAJE DE BLOCK COUNT en Comida.ims");
		tipo_sabotaje = SABOTAJE_BLOCK_COUNT_C;
	}else
	if(block_count_incosistente(GENERAR_BASURA)){
		log_warning(logger, "SABOTAJE DE BLOCK COUNT en Basura.ims");
		tipo_sabotaje = SABOTAJE_BLOCK_COUNT_B;
	}else
	if(superbloque.blocks != config_get_int_value(superbloque_file, "BLOCKS")){
		log_warning(logger, "SABOTAJE EN BLOCKS en Superbloque");
		tipo_sabotaje = SABOTAJE_BLOCKS;
	}else
	if(strcmp(un_bitmap, otro_bitmap) != 0){
		log_warning(logger, "SABOTAJE EN BITMAP en Superbloque");
		tipo_sabotaje = SABOTAJE_BITMAP;
	}else{
		log_info(logger, "NO HUBO SABOTAJE");
		tipo_sabotaje = -1;
	}

	config_destroy(superbloque_file);
	free(path);
	free(un_bitmap);
	return tipo_sabotaje;
}

void resolver_sabotaje(t_sabotaje sabotaje){
	switch(sabotaje){
		case SABOTAJE_BITMAP: corregir_bitmap();
		break;
		case SABOTAJE_BLOCKS: sobreescribir_superbloque();
		break;
		case SABOTAJE_SIZE_O: corregir_size(GENERAR_OXIGENO);
		break;
		case SABOTAJE_SIZE_C: corregir_size(GENERAR_COMIDA);
		break;
		case SABOTAJE_SIZE_B: corregir_size(GENERAR_BASURA);
		break;
		case SABOTAJE_BLOCK_COUNT_O: corregir_block_count(GENERAR_OXIGENO);
		break;
		case SABOTAJE_BLOCK_COUNT_C: corregir_block_count(GENERAR_COMIDA);
		break;
		case SABOTAJE_BLOCK_COUNT_B: corregir_block_count(GENERAR_BASURA);
		break;
		case SABOTAJE_BLOCKS_O: restaurar_blocks(GENERAR_OXIGENO);
		break;
		case SABOTAJE_BLOCKS_C: restaurar_blocks(GENERAR_COMIDA);
		break;
		case SABOTAJE_BLOCKS_B: restaurar_blocks(GENERAR_BASURA);
		break;
		default:
		break;
	}
}

// INICIO - IDENTIFICAR SABOTAJE //

bool size_files_incosistente(int recurso){
	bool ret = false;
	metadata_recurso* metadata_r = struct_metadata_recurso(recurso);
	char* path = generar_path_metadata(recurso);
	if(!existe_archivo(path)){
		free(path);
		return ret;
	}

	t_config * config_recurso = config_create(path);
	ret =  metadata_r->size != config_get_int_value(config_recurso, "SIZE");
	config_destroy(config_recurso);

	free(path);
	return ret;
}

bool block_count_incosistente(int recurso){
	bool ret = false;
	metadata_recurso* metadata_r = struct_metadata_recurso(recurso);
	char* path = generar_path_metadata(recurso);
	if(!existe_archivo(path)){
		free(path);
		return ret;
	}

	t_config * config_recurso = config_create(path);
	ret =  metadata_r->block_count != config_get_int_value(config_recurso, "BLOCK_COUNT");
	config_destroy(config_recurso);

	free(path);
	return ret;
}
// TODO volar toto?
bool file_blocks_inconsistente(int recurso){
	bool ret = false;
	metadata_recurso* metadata_r = struct_metadata_recurso(recurso);
	char* path = generar_path_metadata(recurso);
	if(!existe_archivo(path) || (metadata_r->size % superbloque.block_size == 0)){
		free(path);
		return ret;
	}
	t_config* config_recurso = config_create(path);
	char* blocks = config_get_string_value(config_recurso, "BLOCKS");

	char* ultimo_bloque_file = get_token_at(metadata_r->blocks, ',', token_count(metadata_r->blocks, ',')-1);
	char* ultimo_bloque_post_sabotaje = get_token_at(blocks, ',', token_count(blocks, ',')-1);

	ret =  strcmp(ultimo_bloque_file, ultimo_bloque_post_sabotaje) != 0;
	config_destroy(config_recurso);

	free(ultimo_bloque_file);
	//TODO: MEMORY LEAK
	free(ultimo_bloque_post_sabotaje);
	free(path);
	return ret;
}



bool blocks_incosistente(int recurso){
	char* path = generar_path_metadata(recurso);
	metadata_recurso* metadata_r = struct_metadata_recurso(recurso);
	bool ret = false;
	if(!existe_archivo(path))return ret;

	t_config* file_recurso = config_create(path);
	char* lista_de_bloques = string_new();
	string_append(&lista_de_bloques, config_get_string_value(file_recurso,"BLOCKS"));
	config_destroy(file_recurso);

	char* md5_aux = generar_md5_inconsistente(lista_de_bloques);

	//free(lista_de_bloques);
	if(strcmp(metadata_r->md5,md5_aux) != 0){
		ret = true;
	}
	free(md5_aux);
	free(path);
	return ret;
}

char* generar_md5_inconsistente(char* lista_de_bloques){
	char* path = generar_path_md5();
	char* md5 = string_new();
	char* blocks_string = leer_bloques_inconsistentes(lista_de_bloques);

	FILE* md5_file_aux;

	char* md5_aux = malloc(33);

	if(!existe_archivo(path)){
		md5_file_aux = fopen(path,"wb");
	}else md5_file_aux = fopen(path,"r+b");

	string_append(&md5,"echo ");
	string_append(&md5, "\"");
	string_append(&md5,blocks_string);
	string_append(&md5, "\"");
	string_append(&md5," | md5sum");
	string_append(&md5," > ");
	string_append(&md5,path);
	system(md5);

	fgets(md5_aux,33,md5_file_aux);
	fclose(md5_file_aux);

	free(md5);
	free(blocks_string);
	free(path);
	return md5_aux;
}

char* leer_bloques_inconsistentes(char* cadena_tokenizada){
	char* bloque_to_string = string_new();
	char* bloque_numero;
	char* leer_bloque_nro;

	for(int i=0;i<token_count(cadena_tokenizada, ',');i++){
		bloque_numero= get_token_at(cadena_tokenizada,',',i);
		int num = atoi(bloque_numero);
		free(bloque_numero);
		leer_bloque_nro = malloc(superbloque.block_size);
		memcpy(leer_bloque_nro, copia_mapeo + superbloque.block_size * num, superbloque.block_size);
		string_trim(&leer_bloque_nro);
		string_append(&bloque_to_string,leer_bloque_nro);
		free(leer_bloque_nro);
	}
	free(cadena_tokenizada);
	return bloque_to_string;
}

// FIN - IDENTIFICAR SABOTAJE //

// INICIO - RECUPERAR SUPERBLOQUE //

void corregir_bitmap(){
	char* hola_i_mongo_store = generar_path_superbloque();
	t_config* superbloque_file = config_create(hola_i_mongo_store);
	char* un_bitmap = bitarray_to_chararray(superbloque.bitmap);
	config_set_value(superbloque_file,"BITMAP",un_bitmap);
	config_save(superbloque_file);
	config_destroy(superbloque_file);

	log_info(logger,"Superbloque corregido exitosamente");

	free(un_bitmap);
	free(hola_i_mongo_store);
}

void sobreescribir_superbloque(){
	char* paht = generar_path_superbloque();
	t_config* superbloque_file = config_create(paht);
	char* blocks = string_itoa(superbloque.blocks);
	config_set_value(superbloque_file, "BLOCKS", blocks);
	config_save(superbloque_file);
	config_destroy(superbloque_file);

	log_info(logger,"Superbloque corregido exitosamente");

	free(blocks);
	free(paht);
}

// FIN - RECUPERAR SUPERBLOQUE //

// INICIO - RECUPERAR FILES //

void corregir_size(t_mensaje recurso){
	char* path = generar_path_metadata(recurso);
	metadata_recurso* metadata_r = struct_metadata_recurso(recurso);
	char* size = string_itoa(metadata_r->size);

	t_config* file_recurso = config_create(path);
	config_set_value(file_recurso,"SIZE",size);
	config_save(file_recurso);
	config_destroy(file_recurso);

	log_info(logger,"Archivo %s corregido exitosamente",path);

	free(path);
	free(size);
}

void corregir_block_count(t_mensaje recurso){
	char* path = generar_path_metadata(recurso);
	metadata_recurso* metadata_r = struct_metadata_recurso(recurso);
	char* block_count = string_itoa(metadata_r->block_count);

	t_config* file_recurso = config_create(path);
	config_set_value(file_recurso,"BLOCK_COUNT",block_count);
	config_save(file_recurso);
	config_destroy(file_recurso);

	log_info(logger,"Archivo %s corregido exitosamente",path);

	free(path);
	free(block_count);
}

void restaurar_blocks(int recurso){
	char* path = generar_path_metadata(recurso);
	metadata_recurso* metadata_r = struct_metadata_recurso(recurso);
	t_config* file_recurso = config_create(path);
	char* cadena_blocks = string_new();
	string_append(&cadena_blocks, config_get_string_value(file_recurso, "BLOCKS"));
	config_destroy(file_recurso);

	int nro_bloque;
	for(int i=0;i<token_count(cadena_blocks, ',');i++){

		char* numerito = get_token_at(cadena_blocks,',',i);
		nro_bloque = atoi(numerito);
		borrar_bloque(nro_bloque);
		free(numerito);
	}

	char* caracter_llenado = caracter_mensaje(recurso);
	int cantidad = metadata_r->size;

	free(metadata_r->blocks);
	metadata_r->blocks = string_new();
	metadata_r->block_count = 0;
	metadata_r->size = 0;
	metadata_r->offset = 0;
	int j = 0;

	pthread_mutex_lock(&mutex_recurso[recurso]);
	for(int i = 1; i <= cantidad; i++){

		if(metadata_r->offset % superbloque.block_size == 0){

			char* numerito1 = get_token_at(cadena_blocks,',',j);
			nro_bloque = atoi(numerito1);
			free(numerito1);

			t_config* metadata = config_create(path);

			if(!j) string_append_with_format(&metadata_r->blocks, "%d", nro_bloque);
			else string_append_with_format(&metadata_r->blocks, ",%d", nro_bloque);

			metadata_r->block_count ++;
			metadata_r->offset = 0;
			metadata_r->nro_ultimo_bloque = nro_bloque;

			char* numerito2 = string_itoa(metadata_r->block_count);
			config_set_value(metadata, "BLOCK_COUNT", numerito2);
			free(numerito2);

			config_set_value(metadata, "BLOCKS", metadata_r->blocks);
			config_save(metadata);
			config_destroy(metadata);
			j++;
		}

		metadata_r->posicion_bloque = metadata_r->nro_ultimo_bloque*superbloque.block_size + metadata_r->offset;
		metadata_r->offset++;
		metadata_r->size++;
		generar_md5(metadata_r);

		t_config* metadata = config_create(path);
		char* numerito3 = string_itoa(metadata_r->size);
		config_set_value(metadata, "SIZE", numerito3);
		free(numerito3);

		config_set_value(metadata, "MD5_ARCHIVO", metadata_r->md5);
		config_save(metadata);
		config_destroy(metadata);

		memcpy(copia_mapeo+metadata_r->posicion_bloque,caracter_llenado,1);
	}

	pthread_mutex_unlock(&mutex_recurso[recurso]);
	log_info(logger,"Archivo %s corregido exitosamente",path);

	free(cadena_blocks);
	free(path);
}

void borrar_bloque(int nro_bloque){
	for(int i = 0; i < superbloque.block_size; i++){
		memcpy(copia_mapeo+nro_bloque*superbloque.block_size + i," ",1);
	}
	log_info(logger, "Bloque numero %d borrado", nro_bloque);
}

// FIN - RECUPERAR FILES //











