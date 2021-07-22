#ifndef RECURSOS_H_
#define RECURSOS_H_

#include "filesystem.h"
#include "utils.h"
#include <stdio.h>
#include <openssl/md5.h>

typedef struct{
	uint32_t size; //size = (block_count -1)* block_size + offset
	uint32_t block_count;
	char* blocks;
	char caracter_llenado;
	char* md5;
	int offset; //en bytes
	int nro_ultimo_bloque;
	int posicion_bloque;
}metadata_recurso;

metadata_recurso* metadata_o;
metadata_recurso* metadata_c;
metadata_recurso* metadata_b;

metadata_recurso* struct_metadata_recurso(int);

void crear_archivo_recurso(int);

/* Bloque asignado/desasignado*/
void actualizar_metadata_recurso(int,int);
int des_actualizar_metadata_recurso(int);

/* Carga y descarga de recurso */
void cargar_recurso(int, int);
void descargar_recurso(int, int);

/* Bloques a string */
char* leer_bloques_recurso(metadata_recurso*);
char* leer_bloque_numero(int nro);

/* MD5 */
void generar_md5(metadata_recurso*);
char* generar_path_md5();

pthread_mutex_t mutex_recurso[3];

/* Auxiliares */
char* caracter_mensaje(int);
char* cadena_de_blocks(metadata_recurso*);

#endif /* RECURSOS_H_ */
