#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include<commons/bitarray.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include "recursos.h"
#include "bitacoras.h"
#include "utils.h"

FILE* file_blocks;
void* copia_mapeo;

// SEMAFOROS //

sem_t sem_sync;
sem_t sem_sleep;

// ARCHIVOS //

int crear_todos_archivos();
void crear_archivo_superbloque();
int crear_blocks();

// DIRECTORIOS //

void crear_punto_montaje();
void crear_carpeta_files();
void crear_carpeta_bitacoras();

// PATHS //

char* generar_path_superbloque();
char* generar_path_blocks();
char* generar_path_files();
char* generar_path_metadata(int);
char* generar_path_bitacoras();
char* generar_path_bitacora(uint32_t);

// SUPERBLOQUE //

typedef struct {
	uint32_t block_size;
	uint32_t blocks;
	t_bitarray* bitmap;
}t_superbloque;

t_superbloque superbloque;

void actualizar_metadata_superbloque(int);
void des_actualizar_metadata_superbloque(int);

// Des/Asignacion de bloques //

int buscar_primer_bloque_libre();
int asignar_bloque_en_metadata_bitacora(int);
int asignar_bloque_en_metadata_recurso(int);
void des_asignar_bloque_en_metadata_recurso(int);

// BLOCKS //

//int blocks_fd;
void* blocks_mapeado;
void* copia_mapeo;

int mapear_blocks();
void copiar_mapeo();
void inicializar_semaforos();
void* sincronizar_file_blocks();
void* esperar_sincronizacion();

// INICIO - FSCK //

typedef enum{
	// SUPERBLOQUE //
	SABOTAJE_BITMAP = 0,
	SABOTAJE_BLOCKS = 1,
	// FILES //
	SABOTAJE_BLOCK_COUNT_O = 2,
	SABOTAJE_BLOCK_COUNT_C = 3,
	SABOTAJE_BLOCK_COUNT_B = 4,

	SABOTAJE_BLOCKS_O = 5,
	SABOTAJE_BLOCKS_B = 6,
	SABOTAJE_BLOCKS_C = 7,

	SABOTAJE_SIZE_O = 8,
	SABOTAJE_SIZE_C = 9,
	SABOTAJE_SIZE_B = 10
}t_sabotaje;

void protocolo_fsck();

int identificar_sabotaje();
void resolver_sabotaje(t_sabotaje);

bool bitmap_incosistente();
bool size_files_incosistente(int);
bool block_count_incosistente(int);
bool file_blocks_inconsistente(int);

// 					capaz no van mas					//
bool blocks_incosistente(int);
char* generar_md5_inconsistente(char*);
char* leer_bloques_inconsistentes(char*);
//////////////////////////////////////////////////////////

void corregir_bitmap();
void sobreescribir_superbloque();

void corregir_size();
void corregir_block_count();
void restaurar_blocks(int);
void borrar_bloque(int);

// FIN - FSCK //

/* Auxiliares */

bool existe_archivo(char*);
bool existe_FS();

char* bitarray_to_chararray();
t_bitarray* chararray_to_bitarray(char*);

void recuperar_archivo_si_y_solo_si_hay_que_recuperarlos(int);


#endif /* FILESYSTEM_H_ */
