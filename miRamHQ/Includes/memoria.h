/*
 * memoria.h
 *
 *  Created on: 16 jun. 2021
 *      Author: utnso
 */

#ifndef INCLUDES_MEMORIA_H_
#define INCLUDES_MEMORIA_H_

#include<stdlib.h>
#include<string.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include<commons/config.h>
#include<commons/memory.h>
#include<commons/temporal.h>
#include<netdb.h>
#include<commons/bitarray.h>
#include<nivel-gui/nivel-gui.h>
#include<nivel-gui/tad_nivel.h>
#include"token.h"
#include"../utils.h"


//void liberar_listas_en_datos_tripulantes(void*);


typedef struct {
	uint32_t pid;
	uint32_t tareas;
} pcb_t;

typedef struct {
	uint32_t tid;
	char estado;
	uint32_t x_pos;
	uint32_t y_pos;
	uint32_t proxima_instruccion;
	uint32_t puntero_pcb;
} tcb_t;

typedef struct {
	void* bloque_memoria_principal;
	char* tipo_memoria;
	char* criterio_fit;
	char* algoritmo_reemplazo;
	uint32_t tamanio_memoria;
	uint32_t memoria_disponible;
	uint32_t tamanio_pagina;
	uint32_t tamanio_swap;
	char* path_swap;
} t_memoria_principal;

typedef struct {
	uint32_t numero_segmento;
	uint32_t inicio;
	uint32_t tamanio;
} t_fila_tabla_de_segmentos;

typedef struct {
	uint32_t numero_de_pagina;
	uint32_t frame_asignado;
	uint32_t ultima_referencia;
	char bit_de_uso;
	uint32_t peso_actual;
	char bit_de_presencia;
	uint32_t frame_asignado_en_swap;
} t_fila_tabla_de_paginas;

typedef struct {
	uint32_t id;
	t_list* paginas_que_ocupa;
	uint32_t offset;
}t_datos_tripulante;

typedef struct {
	t_list* paginas_que_ocupa;
	uint32_t offset;
}t_datos;

typedef struct {
	t_datos* datos_pcb;
	t_datos* datos_tareas;
	t_list* datos_tripulantes; //Estos 3 se utilizan en paginacion

	uint32_t pid_asociado;
	uint32_t longitud_tareas;
	uint32_t cantidad_de_tripulantes;
	t_list* filas; // En Segmentacion: el get(0) te da la fila del PCB, el get(1) te da la fila de sus tareas, y los demas te dan las filas de los tripulantes
} t_tabla_patota;

FILE* archivo_swap;

t_log* logger_pruebas; // TODO borrar despues

t_memoria_principal* memoria_principal;
t_list* tablas_segmentos;
t_list* lista_de_tcb_reconstruidos;
uint32_t indice_nuevo_tripulante;
uint32_t indice_nueva_patota;
uint32_t numero_segmento_actual;

t_fila_tabla_de_paginas* puntero_victima_clock;

char* memoria_a_mapear;
char* memoria_a_mapear_swap;
t_bitarray* bitarray;
t_bitarray* bitarray_swap;
NIVEL* mapa;

t_memoria_principal* crear_memoria(t_config*);
void inicializar_memoria(void*, uint32_t);
tcb_t* crear_tripulante(uint32_t, char, uint32_t, uint32_t, uint32_t, uint32_t);
pcb_t* crear_patota(uint32_t);
t_tabla_patota* crear_tabla(uint32_t);
t_fila_tabla_de_paginas* crear_fila_paginas(uint32_t, uint32_t);
void iniciar_tripulante(uint32_t, uint32_t, uint32_t);
void cargar_a_memoria(pcb_t*, char*, t_list*);
void cargar_a_memoria_segmentada(pcb_t*, char*, t_list*);
void cargar_a_memoria_paginada(pcb_t*, char*, t_list*);
t_fila_tabla_de_segmentos* crear_fila(uint32_t, uint32_t, uint32_t);
uint32_t entra(uint32_t, uint32_t);
uint32_t existe_direccion(uint32_t);
uint32_t encontrar_fit(uint32_t);
uint32_t encontrar_first_fit(uint32_t);
uint32_t encontrar_best_fit(uint32_t);
t_fila_tabla_de_segmentos* buscar_segmento_mas_cercano(uint32_t);
uint32_t calcular_tamanio_espacio_libre(uint32_t);
void compactar_memoria();
void borrar_tcb_de_memoria(tcb_t*);
void borrar_tcb_de_memoria_segmentada(tcb_t*);
void borrar_tcb_de_memoria_paginada(uint32_t);
void eliminar_tabla(t_tabla_patota*);

char* tareas_del_pcb(pcb_t*);
char* tareas_del_pcb_por_tid(uint32_t);
char* string_tripulantes_de_patota();
char* string_tabla_segmentos();
char* string_tabla_paginas();
char* bool_a_string(uint32_t);
char* string_tcb(tcb_t* tcb);
void actualizar_tripulante(tcb_t*);
char indice_a_simbolo(uint32_t);

uint32_t cantidad_frames_disponibles();
uint32_t alcanza_memoria(uint32_t);

uint32_t minimo(uint32_t, uint32_t);
uint32_t maximo(uint32_t, uint32_t);
uint32_t serializar_en_paginas(void*, size_t, uint32_t*, t_tabla_patota*, uint32_t*, uint32_t, t_list*);
t_fila_tabla_de_paginas* get_fila_por_marco(uint32_t);
void actualizar_paginas(t_list*);
t_list* seleccionar_victimas(uint32_t cantidad_paginas);
t_fila_tabla_de_paginas* obtener_victima();
t_fila_tabla_de_paginas* obtener_victima_lru();
t_fila_tabla_de_paginas* obtener_victima_clock();
void avanzar_puntero_victima();
void buscar_indices_victima_clock(uint32_t*, uint32_t*);

//----------------SWAP-----------------------------------------------
uint32_t alcanza_memoria_swap(uint32_t);
void meter_paginas_a_swap(t_list*,t_list*);
uint32_t frames_libres_en_swap();
uint32_t  buscar_frame_disponible_en_disco();
void pasar_de_swap_a_memoria(t_list*);
t_list* contenido_de_paginas(t_list*);

//------------GETTERS------------
uint32_t buscar_frame_disponible();
uint32_t get_indice_tabla(t_tabla_patota*);
uint32_t get_indice_pagina(t_fila_tabla_de_paginas*);
t_list* get_paginas_por_tid(uint32_t);
uint32_t get_offset_tid(uint32_t);
uint32_t get_indice_tid(uint32_t);
tcb_t* get_tripulante_por_id(uint32_t);
tcb_t* get_tripulante_por_id_segmentacion(uint32_t);
tcb_t* get_tripulante_por_id_paginacion(uint32_t);
pcb_t* get_patota_por_id(uint32_t);
t_tabla_patota* get_tabla_por_pid(uint32_t);
uint32_t get_pid_por_marco(uint32_t);
t_tabla_patota* get_tabla_por_tid(uint32_t);
t_tabla_patota* get_tabla_por_tid_segmentacion(uint32_t);
t_tabla_patota* get_tabla_por_tid_paginacion(uint32_t);
t_fila_tabla_de_paginas* get_pagina_del_frame(uint32_t);
pcb_t* recuperar_pcb(t_datos*);
char* recuperar_tareas(t_datos*, uint32_t);
tcb_t* recuperar_tcb(t_datos_tripulante*);


/* funcion que hizo martu */
uint32_t get_indice_pagina_en_tripulante(t_datos_tripulante* , t_fila_tabla_de_paginas* );


#endif /* INCLUDES_MEMORIA_H_ */
