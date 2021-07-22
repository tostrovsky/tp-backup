/*
 * bitacoras.h
 *
 *  Created on: 27 jun. 2021
 *      Author: utnso
 */

#ifndef BITACORAS_H_
#define BITACORAS_H_

#include"filesystem.h"
#include"utils.h"
#include<commons/string.h>
#include<string.h>

typedef struct{
	uint32_t size;
	char* blocks;
	int offset;
	int nro_ultimo_bloque;

	//prueba
	int posicion_bloque;
	uint32_t block_count;

}metadata_bitacora;

t_dictionary* metadatas_bitacora;
t_dictionary* metadata_bitacora_prueba;

void crear_archivo_bitacora(uint32_t);

void cargar_en_bitacora(t_list* lista);
void cargar_accion(int, char*);

void actualizar_metadata_bitacora(int,int);

/* Loggear bitacora */
void loggear_bloque_numero(int);
void loggear_bitacora_del_tripulante(int);

// no se usa void iniciar_tripulante(int tid);

pthread_mutex_t mutex_bitacora[20];

#endif /* BITACORAS_H_ */
