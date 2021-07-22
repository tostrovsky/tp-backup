/*
 * memoria.c
 *
 *  Created on: 16 jun. 2021
 *      Author: utnso
 */

#include"memoria.h"


void inicializar_memoria(void* memoria, uint32_t tamanio) {
	char* aux = (char*) memoria;
	for (uint32_t i=0; i<tamanio; i++) {
		*aux = 0;
		aux++;
	}
}
//TODO ver lo de liberar las lista de patotas , cofig ,etc

t_memoria_principal* crear_memoria(t_config* config) {

	uint32_t tamanio = config_get_int_value(config, "TAMANIO_MEMORIA");

	t_memoria_principal* memoria = malloc(sizeof(t_memoria_principal));
	memoria->bloque_memoria_principal = malloc(tamanio);
	memoria->tipo_memoria = config_get_string_value(config, "ESQUEMA_MEMORIA");
	memoria->criterio_fit = config_get_string_value(config, "CRITERIO_FIT");
	memoria->algoritmo_reemplazo = config_get_string_value(config, "ALGORITMO_REEMPLAZO");
	memoria->tamanio_memoria = tamanio;
	memoria->memoria_disponible = tamanio;
	memoria->tamanio_pagina = config_get_int_value(config, "TAMANIO_PAGINA");
	memoria->tamanio_swap = config_get_int_value(config, "TAMANIO_SWAP");
	memoria->path_swap = config_get_string_value(config, "PATH_SWAP");

	archivo_swap = fopen(memoria->path_swap, "wb");
	fclose(archivo_swap);

	if (strcmp(memoria->tipo_memoria, "PAGINACION") == 0) {
		uint32_t cantidad_de_frames = tamanio/(memoria->tamanio_pagina);
		uint32_t tamanio_de_bits = cantidad_de_frames/8;
		if (cantidad_de_frames % 8 > 0)
			tamanio_de_bits++;
		memoria_a_mapear = malloc(tamanio_de_bits);
		bitarray = bitarray_create_with_mode(memoria_a_mapear, tamanio_de_bits, MSB_FIRST);
		for (uint32_t i=0; i<cantidad_de_frames; i++) {
			bitarray_clean_bit(bitarray, i);
		}

	//--------------------swap-------------------------------------------------

		uint32_t cantidad_de_frames_swap = memoria->tamanio_swap/(memoria->tamanio_pagina);
		uint32_t tamanio_de_bits_swap = cantidad_de_frames_swap/8;
		if (cantidad_de_frames_swap % 8 > 0)
			tamanio_de_bits_swap++;
			memoria_a_mapear_swap = malloc(tamanio_de_bits_swap);
			bitarray_swap = bitarray_create_with_mode(memoria_a_mapear_swap, tamanio_de_bits_swap, MSB_FIRST);
		for (uint32_t i=0; i<cantidad_de_frames_swap; i++) {
			bitarray_clean_bit(bitarray_swap, i);
		}
	}

	inicializar_memoria(memoria->bloque_memoria_principal, tamanio);

	return memoria;

}

tcb_t* crear_tripulante(uint32_t id, char est, uint32_t x, uint32_t y, uint32_t p_i, uint32_t p_pcb) {
	tcb_t* tripulante = malloc(sizeof(tcb_t));
	tripulante->tid = id;
	tripulante->estado = est;
	tripulante->x_pos = x;
	tripulante->y_pos = y;
	tripulante->proxima_instruccion = p_i;
	tripulante->puntero_pcb = p_pcb;

	return tripulante;

}

pcb_t* crear_patota(uint32_t id) {
	pcb_t* patota = malloc(sizeof(pcb_t));
	patota->pid = id;

	return patota;

}

char indice_a_simbolo(uint32_t indice_tripulante) {
	char retorno;
	if (indice_tripulante >= 0 && indice_tripulante <= 9) {
		retorno = indice_tripulante + '0';
	}
	else if (indice_tripulante > 9) {
		retorno = indice_tripulante - 10 + 'A';
	}
	return retorno;
}


//-----Para realizar el Dump--------------------------------------------
//----------------------------------------------------------------------
char* string_tabla_segmentos() {
	char* info = string_new();
	uint32_t cant_segmentos = list_size(tablas_segmentos);
	string_append(&info, "--------------------------------------------------------------------------------------------------------------------------\nDump: ");
	string_append(&info, temporal_get_string_time("%d/%m/%y %H:%M:%S"));
	for (int i=0; i<cant_segmentos; i++) {
		t_tabla_patota* tabla_actual = (t_tabla_patota*)list_get(tablas_segmentos, i);
		for (int j=0; j<list_size(tabla_actual->filas); j++) {
			t_fila_tabla_de_segmentos* fila_actual = (t_fila_tabla_de_segmentos*)list_get(tabla_actual->filas, j);
			string_append(&info, "\nProceso: ");
			string_append(&info, string_itoa(tabla_actual->pid_asociado));
			string_append(&info, "\t\tSegmento: ");
			string_append(&info, string_itoa(fila_actual->numero_segmento));
			string_append(&info, "\t\tInicio: ");
			string_append_with_format(&info, "0x0%x", fila_actual->inicio);
			string_append(&info, "\t\tTamanio: ");
			string_append(&info, string_itoa(fila_actual->tamanio));
			string_append(&info, " bytes");
		}
	}
	string_append(&info, "\n--------------------------------------------------------------------------------------------------------------------------");
	return info;

}

char* string_tabla_paginas() {
	char* info = string_new();
	uint32_t cant_frames = memoria_principal->tamanio_memoria/memoria_principal->tamanio_pagina;
	uint32_t estado;
	uint32_t pid;
	uint32_t pagina;

	string_append(&info, "--------------------------------------------------------------------------------------------------------------------------\nDump: ");
	string_append(&info, temporal_get_string_time("%d/%m/%y %H:%M:%S"));

	for (uint32_t i=0; i<cant_frames; i++) {
		estado = bitarray_test_bit(bitarray, i);

		string_append(&info, "\nMarco: ");
		string_append(&info, string_itoa(i));
		string_append(&info, "\t\tEstado: ");
		string_append(&info, bool_a_string(estado));

		if (estado == 0) {
			string_append(&info, "\t\t\tProceso: -\t\tPagina: -");
		}
		else {
			pid = get_pid_por_marco(i);
			if (pid != -1) {
				pagina = get_fila_por_marco(i)->numero_de_pagina;
				string_append(&info, "\t\tProceso: ");
				string_append(&info, string_itoa(pid));
				string_append(&info, "\t\tPagina: ");
				string_append(&info, string_itoa(pagina));
			}
		}

	}

	string_append(&info, "\n--------------------------------------------------------------------------------------------------------------------------");
	return info;

}

char* string_tripulantes_de_patota() {
	uint32_t cant_patotas = list_size(tablas_segmentos);
	uint32_t cant_tripus;

	t_tabla_patota* patota_actual;
	t_datos_tripulante* datos_actuales;

	char* info = string_new();
	string_append(&info, "\n");

	for (uint32_t i=0; i<cant_patotas; i++) {
		patota_actual = (t_tabla_patota*)list_get(tablas_segmentos, i);
		cant_tripus = list_size(patota_actual->datos_tripulantes);
		string_append_with_format(&info, "Patota: %d\n", patota_actual->pid_asociado);

		for (uint32_t j=0; j<cant_tripus; j++) {
			datos_actuales = (t_datos_tripulante*)list_get(patota_actual->datos_tripulantes, j);
			string_append_with_format(&info, "Tripulante: %d\t", datos_actuales->id);
			string_append_with_format(&info, "Offset: %d\t", datos_actuales->offset);
			string_append_with_format(&info, "C. Pags: %d\t\n", list_size(datos_actuales->paginas_que_ocupa));
		}
		string_append(&info, "\n");
	}
	return info;

}

t_fila_tabla_de_paginas* get_fila_por_marco(uint32_t numero_marco) {
	t_tabla_patota* tabla_actual;
	t_fila_tabla_de_paginas* fila_actual;
	uint32_t cantidad_filas;
	uint32_t cantidad_patotas = list_size(tablas_segmentos);
	for (uint32_t j=0; j<cantidad_patotas; j++) {
		tabla_actual = list_get(tablas_segmentos, j);
		cantidad_filas = list_size(tabla_actual->filas);
		for (uint32_t k=0; k<cantidad_filas; k++) {
			fila_actual = list_get(tabla_actual->filas, k);
			if (fila_actual->frame_asignado == numero_marco&& fila_actual->bit_de_presencia==1) {
				return fila_actual;
			}
		}
	}
	return NULL;
}

uint32_t get_pid_por_marco(uint32_t numero_marco) {
	t_tabla_patota* tabla_actual;
	t_fila_tabla_de_paginas* fila_actual;
	uint32_t cantidad_filas;
	uint32_t cantidad_patotas = list_size(tablas_segmentos);
	for (uint32_t j=0; j<cantidad_patotas; j++) {
		tabla_actual = list_get(tablas_segmentos, j);
		cantidad_filas = list_size(tabla_actual->filas);
		for (uint32_t k=0; k<cantidad_filas; k++) {
			fila_actual = list_get(tabla_actual->filas, k);
			if (fila_actual->frame_asignado == numero_marco && fila_actual->bit_de_presencia==1) {
				return tabla_actual->pid_asociado;
			}
		}
	}
	return -1;
}

char* bool_a_string(uint32_t b) {
	char* retorno;
	if (b == 0)
		retorno = "Libre";
	else
		retorno = "Ocupado";

	return retorno;
}

char* string_tcb(tcb_t* tcb) {
	char* info = string_new();
	string_append(&info, "Tripulante ID: ");
	string_append_with_format(&info,"%d",tcb->tid);
	string_append(&info, "\nEstado: ");
	string_append_with_format(&info,"%d", tcb->estado);
	string_append(&info, "\nX: ");
	string_append_with_format(&info,"%d",tcb->x_pos);
	string_append(&info, "\nY: ");
	string_append_with_format(&info,"%d",tcb->y_pos);
	string_append(&info, "\nID: proxima instruccion: ");
	string_append_with_format(&info,"%d",tcb->proxima_instruccion);
	string_append(&info, "\nPuntero a la patota: ");
	string_append_with_format(&info,"%d",tcb->puntero_pcb);


	return info;

}
//----------------------------------------------------------------------


t_fila_tabla_de_segmentos* crear_fila(uint32_t numero, uint32_t inicio, uint32_t tamanio) {
	t_fila_tabla_de_segmentos* fila = malloc(sizeof(t_fila_tabla_de_segmentos));
	fila->numero_segmento = numero;
	fila->inicio = inicio;
	fila->tamanio = tamanio;

	return fila;

}

t_fila_tabla_de_paginas* crear_fila_paginas(uint32_t numero_de_pagina, uint32_t frame_asignado){
	t_fila_tabla_de_paginas* fila = malloc(sizeof(t_fila_tabla_de_paginas));
	fila->peso_actual=0;
	fila->numero_de_pagina = numero_de_pagina;
	fila->frame_asignado = frame_asignado;
	fila->bit_de_uso = 1;
	fila->bit_de_presencia = 1;
	char* temporal_get_string_time2 = temporal_get_string_time("%H%M%S");
	fila->ultima_referencia = atoi(temporal_get_string_time2);
	free(temporal_get_string_time2);
	return fila;


}

t_tabla_patota* crear_tabla(uint32_t pid) {
	t_tabla_patota* tabla = malloc(sizeof(t_tabla_patota));
	tabla->pid_asociado = pid;
	tabla->filas = list_create();
	tabla->cantidad_de_tripulantes = 0;

	return tabla;

}

uint32_t encontrar_fit(uint32_t tamanio) {
	uint32_t retorno;

	if (strcmp(memoria_principal->criterio_fit, "FIRST_FIT") == 0) {
		retorno = encontrar_first_fit(tamanio);
	}

	if (strcmp(memoria_principal->criterio_fit, "BEST_FIT") == 0) {
		retorno = encontrar_best_fit(tamanio);
	}

	return retorno;
}

uint32_t encontrar_first_fit(uint32_t tamanio) {

	t_tabla_patota* tabla_actual;
	t_fila_tabla_de_segmentos* fila_actual;
	uint32_t cant_tablas = list_size(tablas_segmentos);

	if (cant_tablas == 0) {
		return 0;
	}

	if (!existe_direccion(0) && entra(0, tamanio)) {
		return 0;
	}

	uint32_t direccion_candidata = -1;
	uint32_t direccion_actual;


	for(uint32_t i=0; i<cant_tablas; i++) {
		tabla_actual = list_get(tablas_segmentos, i);
		uint32_t cant_segmentos = list_size(tabla_actual->filas);

		for (uint32_t j=0; j<cant_segmentos; j++) {
			fila_actual = list_get(tabla_actual->filas, j);

			direccion_actual = fila_actual->inicio + fila_actual->tamanio;

			if ((direccion_actual < direccion_candidata || direccion_candidata == -1) && !existe_direccion(direccion_actual) && entra(direccion_actual, tamanio)) {
				direccion_candidata = direccion_actual;
			}
		}
	}
	return direccion_candidata;
}

uint32_t encontrar_best_fit(uint32_t tamanio) {
	t_tabla_patota* tabla_actual;
	t_fila_tabla_de_segmentos* fila_actual;
	uint32_t cant_tablas = list_size(tablas_segmentos);

	if (cant_tablas == 0) {
		return 0;
	}


	uint32_t direccion_candidata = -1;

	if (!existe_direccion(0) && entra(0, tamanio)) {
		direccion_candidata = 0;
	}

	uint32_t direccion_actual;


	for(uint32_t i=0; i<cant_tablas; i++) {
		tabla_actual = list_get(tablas_segmentos, i);
		uint32_t cant_segmentos = list_size(tabla_actual->filas);

		for (uint32_t j=0; j<cant_segmentos; j++) {
			fila_actual = list_get(tabla_actual->filas, j);

			direccion_actual = fila_actual->inicio + fila_actual->tamanio;

			if ((calcular_tamanio_espacio_libre(direccion_actual) < calcular_tamanio_espacio_libre(direccion_candidata) || direccion_candidata == -1) && !existe_direccion(direccion_actual) && entra(direccion_actual, tamanio)) {
				direccion_candidata = direccion_actual;
			}
		}
	}
	return direccion_candidata;
}


uint32_t calcular_tamanio_espacio_libre(uint32_t direccion_espacio) {
	t_fila_tabla_de_segmentos* proximo_segmento = buscar_segmento_mas_cercano(direccion_espacio);

	uint32_t espacio_disponible = memoria_principal->tamanio_memoria - direccion_espacio;

	if (proximo_segmento != NULL) {
		espacio_disponible = proximo_segmento->inicio - direccion_espacio;
	}

	return espacio_disponible;
}

uint32_t cantidad_frames_disponibles() {
	uint32_t cantidad_disponibles = 0;
	uint32_t cantidad_frames = memoria_principal->tamanio_memoria / memoria_principal->tamanio_pagina;
	for (uint32_t i=0; i<cantidad_frames; i++) {
		if (bitarray_test_bit(bitarray, i) == 0)
			cantidad_disponibles++;
	}
	return cantidad_disponibles;
}

uint32_t alcanza_memoria(uint32_t tamanio) {
	uint32_t hay_memoria = 0;

	if (strcmp(memoria_principal->tipo_memoria, "SEGMENTACION") == 0) {
		hay_memoria = tamanio <= memoria_principal->memoria_disponible;
	}

	else if (strcmp(memoria_principal->tipo_memoria, "PAGINACION") == 0) {
		uint32_t frames_necesarios = tamanio / memoria_principal->tamanio_pagina;
		if (tamanio % memoria_principal->tamanio_pagina > 0)
			frames_necesarios++;
		hay_memoria = cantidad_frames_disponibles() >= frames_necesarios;

		if (!hay_memoria) {
			if (alcanza_memoria_swap(tamanio)) {
				uint32_t paginas_a_sacar_de_memoria = frames_necesarios - cantidad_frames_disponibles();
				t_list* paginas_a_sacar = seleccionar_victimas(paginas_a_sacar_de_memoria);
				t_list* contenidos = contenido_de_paginas(paginas_a_sacar);
				meter_paginas_a_swap(paginas_a_sacar, contenidos);
				hay_memoria = 1;
			}
		}
	}
	return hay_memoria;
}



uint32_t  buscar_frame_disponible_en_disco(){
	uint32_t cantidad_de_frames_swap = memoria_principal->tamanio_swap/(memoria_principal->tamanio_pagina);
		for(int i=0;i<cantidad_de_frames_swap;i++){
			if(bitarray_test_bit(bitarray_swap,i)==0){
				return i;
			}
		}
		return -1;
}

uint32_t alcanza_memoria_swap(uint32_t tamanio) {

	uint32_t libre = frames_libres_en_swap();
	uint32_t frames_necesarios=tamanio/memoria_principal->tamanio_pagina;
	if(tamanio%memoria_principal->tamanio_pagina>0){
		frames_necesarios++;
	}
	return libre >= frames_necesarios;
}

uint32_t frames_libres_en_swap() {
	uint32_t cantidad_de_frame=0;
	uint32_t cantidad_de_frames_swap = memoria_principal->tamanio_swap/(memoria_principal->tamanio_pagina);
	for(int i=0;i<cantidad_de_frames_swap;i++){
				if(bitarray_test_bit(bitarray_swap,i)==0){
					cantidad_de_frame++;
				}
			}
	return cantidad_de_frame;
}

uint32_t existe_direccion(uint32_t direccion) {

	t_tabla_patota* tabla_actual;
	t_fila_tabla_de_segmentos* fila_actual;
	uint32_t cant_tablas = list_size(tablas_segmentos);
	uint32_t cant_segmentos;

	for (uint32_t i=0; i<cant_tablas; i++) {
		tabla_actual = (t_tabla_patota*)list_get(tablas_segmentos, i);
		cant_segmentos = list_size(tabla_actual->filas);

		for (uint32_t j=0; j<cant_segmentos; j++) {
			fila_actual = (t_fila_tabla_de_segmentos*)list_get(tabla_actual->filas, j);

			if (fila_actual->inicio == direccion) {
				return 1;
			}
		}
	}
	return 0;
}

void compactar_memoria(){
	uint32_t espacio_vacio;
	t_fila_tabla_de_segmentos* proximo_segmento;
	espacio_vacio = encontrar_first_fit(1);
	if(espacio_vacio != -1){
		proximo_segmento = buscar_segmento_mas_cercano(espacio_vacio);

		while(proximo_segmento != NULL){
			memcpy(memoria_principal->bloque_memoria_principal+espacio_vacio, memoria_principal->bloque_memoria_principal+proximo_segmento->inicio, proximo_segmento->tamanio);
			proximo_segmento->inicio=espacio_vacio;
			espacio_vacio=(proximo_segmento->inicio)+(proximo_segmento->tamanio);
			proximo_segmento = buscar_segmento_mas_cercano(espacio_vacio);
		}
	}
}


t_fila_tabla_de_segmentos* buscar_segmento_mas_cercano(uint32_t espacio_vacio){

	t_fila_tabla_de_segmentos* segmento_mas_cercano = NULL;
	t_tabla_patota* tabla_actual;
	t_fila_tabla_de_segmentos* fila_actual;
	uint32_t cant_tablas = list_size(tablas_segmentos);
	uint32_t cant_segmentos;

	for (uint32_t i=0; i<cant_tablas; i++) {
		tabla_actual = (t_tabla_patota*)list_get(tablas_segmentos, i);
		cant_segmentos = list_size(tabla_actual->filas);

		for (uint32_t j=0; j<cant_segmentos; j++) {
			fila_actual = (t_fila_tabla_de_segmentos*)list_get(tabla_actual->filas, j);

			if ((segmento_mas_cercano == NULL || fila_actual->inicio < segmento_mas_cercano->inicio) && fila_actual->inicio > espacio_vacio) {
				segmento_mas_cercano = fila_actual;
			}
		}
	}

	return segmento_mas_cercano;
}

uint32_t entra(uint32_t puntero, uint32_t tamanio) {
	uint32_t entra = 1;
	t_tabla_patota* tabla_actual;
	t_fila_tabla_de_segmentos* fila_actual;
	uint32_t cant_tablas = list_size(tablas_segmentos);
	uint32_t cant_segmentos;

	for (uint32_t i=0; i<cant_tablas; i++) {
		tabla_actual = (t_tabla_patota*)list_get(tablas_segmentos, i);
		cant_segmentos = list_size(tabla_actual->filas);

		for (uint32_t j=0; j<cant_segmentos; j++) {
			fila_actual = (t_fila_tabla_de_segmentos*)list_get(tabla_actual->filas, j);

			if (fila_actual->inicio > puntero && fila_actual->inicio < puntero+tamanio) {
				return 0;
			}
		}
	}

	return entra;
}

void iniciar_tripulante(uint32_t x, uint32_t y, uint32_t pid) {
	char tid = indice_nuevo_tripulante;
	indice_nuevo_tripulante++;

	t_tabla_patota* tabla_patota = get_tabla_por_pid(pid);

	uint32_t direccion_pcb = (uint32_t)((t_fila_tabla_de_segmentos*)list_get(tabla_patota->filas, 0))->inicio;

	tcb_t* tcb_nuevo = crear_tripulante(tid, 'N', x, y, 0, direccion_pcb);
	uint32_t direccion_tcb = encontrar_fit(sizeof(tcb_t));

	if(direccion_tcb == -1) {
		compactar_memoria();
	}

	memcpy(memoria_principal->bloque_memoria_principal+direccion_tcb, tcb_nuevo, sizeof(tcb_t));
	memoria_principal->memoria_disponible -= sizeof(tcb_t);
	t_fila_tabla_de_segmentos* fila_tcb = crear_fila(numero_segmento_actual, direccion_tcb, sizeof(tcb_t));
	numero_segmento_actual++;
	list_add(tabla_patota->filas, fila_tcb);


	personaje_crear(mapa, indice_a_simbolo(tid), x, y);
	nivel_gui_dibujar(mapa);
	free(tcb_nuevo);
}

void cargar_a_memoria(pcb_t* pcb, char* tareas, t_list* lista) {

	if (strcmp(memoria_principal->tipo_memoria, "SEGMENTACION") == 0) {
		cargar_a_memoria_segmentada(pcb, tareas, lista);
	}

	if (strcmp(memoria_principal->tipo_memoria, "PAGINACION") == 0) {
		cargar_a_memoria_paginada(pcb, tareas, lista);
		if (list_size(tablas_segmentos) == 1) {
			t_tabla_patota* tabla = get_tabla_por_pid(pcb->pid);
			puntero_victima_clock = (t_fila_tabla_de_paginas*)list_get(tabla->filas, 0);
		}
	}
}

void cargar_a_memoria_paginada(pcb_t* pcb, char* tareas, t_list* lista){
	t_tabla_patota* tabla_patota = crear_tabla(pcb->pid);
	list_add(tablas_segmentos, tabla_patota);
	uint32_t longitud_tareas = strlen(tareas);
	tabla_patota->longitud_tareas = longitud_tareas;
	uint32_t pagina_restante = memoria_principal->tamanio_pagina;
	uint32_t paginas_utilizadas = 0;
	uint32_t indice_frame_actual = -1;

	t_datos* datos_pcb = malloc(sizeof(t_datos));
	tabla_patota->datos_pcb = datos_pcb;
	datos_pcb->offset = 0;

	/////////
	datos_pcb->paginas_que_ocupa = list_create();


	indice_frame_actual = serializar_en_paginas(pcb, sizeof(pcb_t), &paginas_utilizadas, tabla_patota, &pagina_restante, indice_frame_actual, datos_pcb->paginas_que_ocupa);

	uint32_t direccion_pcb = indice_frame_actual*memoria_principal->tamanio_pagina;

	t_datos* datos_tareas = malloc(sizeof(t_datos));
	tabla_patota->datos_tareas = datos_tareas;
	datos_tareas->offset = memoria_principal->tamanio_pagina - pagina_restante;

	///////
	datos_tareas->paginas_que_ocupa = list_create();

	indice_frame_actual = serializar_en_paginas(tareas, strlen(tareas)+1, &paginas_utilizadas, tabla_patota, &pagina_restante, indice_frame_actual, datos_tareas->paginas_que_ocupa);

	uint32_t tamanio_lista = list_size(lista);
	uint32_t x;
	uint32_t y;

	tabla_patota->datos_tripulantes = list_create();

	for (uint32_t i=3; i<tamanio_lista; i++) {
		tabla_patota->cantidad_de_tripulantes++;
		char* posx = get_token_at((char*)list_get(lista, i), '|', 0);
		char* posy = get_token_at((char*)list_get(lista, i), '|', 1);
		x = atoi(posx);
		y = atoi(posy);
		free(posx);
		free(posy);
		tcb_t* tcb_nuevo = crear_tripulante(indice_nuevo_tripulante, 'N', x, y, 0, direccion_pcb);

		t_datos_tripulante* datos_tcb = malloc(sizeof(t_datos_tripulante));
		datos_tcb->id = indice_nuevo_tripulante;
		datos_tcb->offset = memoria_principal->tamanio_pagina - pagina_restante;

		//////
		datos_tcb->paginas_que_ocupa = list_create();

		indice_frame_actual = serializar_en_paginas(tcb_nuevo, sizeof(tcb_t), &paginas_utilizadas, tabla_patota, &pagina_restante, indice_frame_actual, datos_tcb->paginas_que_ocupa);

		list_add(tabla_patota->datos_tripulantes, datos_tcb);


		personaje_crear(mapa, indice_a_simbolo(indice_nuevo_tripulante), x, y);
		nivel_gui_dibujar(mapa);
		free(tcb_nuevo);
		indice_nuevo_tripulante++;
	}
}


uint32_t serializar_en_paginas(void* puntero, size_t tamanio, uint32_t* paginas_utilizadas, t_tabla_patota* tabla_patota, uint32_t* tamanio_pagina_restante, uint32_t frame_anterior, t_list* paginas) {

	uint32_t tamanio_pagina = memoria_principal->tamanio_pagina;

	uint32_t offset = 0;
	uint32_t frame_actual = frame_anterior;
	uint32_t tamanio_restante = tamanio;
	uint32_t tamanio_a_usar;
	uint32_t direccion_frame;
	char es_primer_pagina = 1;
	t_fila_tabla_de_paginas* fila_actual;

	while (tamanio_restante > 0) {
		if (*tamanio_pagina_restante == tamanio_pagina) {
			frame_actual = buscar_frame_disponible();
			bitarray_set_bit(bitarray, frame_actual);
			fila_actual = crear_fila_paginas(*paginas_utilizadas, frame_actual);
			list_add(tabla_patota->filas, fila_actual);

			if (paginas != NULL)
				list_add(paginas, fila_actual); //GUARDA LA PAG COMO FILA

			(*paginas_utilizadas)++;
		}
		else {
			fila_actual = get_pagina_del_frame(frame_actual);
			if (es_primer_pagina && paginas != NULL)
				list_add(paginas, fila_actual);
		}

		tamanio_a_usar = minimo(tamanio_restante, *tamanio_pagina_restante);
		fila_actual->peso_actual += tamanio_a_usar;

		direccion_frame = (uint32_t)(memoria_principal->bloque_memoria_principal + frame_actual*tamanio_pagina);
		memcpy((void*)direccion_frame + tamanio_pagina - (*tamanio_pagina_restante), puntero+offset, tamanio_a_usar);

		tamanio_restante -= tamanio_a_usar;
		offset += tamanio_a_usar;

		*tamanio_pagina_restante -= tamanio_a_usar;

		if (*tamanio_pagina_restante <= 0) {
			*tamanio_pagina_restante = tamanio_pagina;
		}
		es_primer_pagina = 0;
	}

	return frame_actual;
}

t_fila_tabla_de_paginas* get_pagina_del_frame(uint32_t frame) {
	uint32_t cant_patotas = list_size(tablas_segmentos);
	uint32_t cant_paginas;
	t_tabla_patota* patota_actual;
	t_fila_tabla_de_paginas* pagina_actual;
	for (uint32_t i=0; i<cant_patotas; i++) {
		patota_actual = (t_tabla_patota*)list_get(tablas_segmentos, i);
		cant_paginas = list_size(patota_actual->filas);
		for (uint32_t j=0; j<cant_paginas; j++) {
			pagina_actual = (t_fila_tabla_de_paginas*)list_get(patota_actual->filas, j);
			if (pagina_actual->frame_asignado == frame && pagina_actual->bit_de_presencia) {
				return pagina_actual;
			}
		}
	}
	return NULL;
}

uint32_t minimo(uint32_t un_numero , uint32_t otro_numero){
	if(un_numero<otro_numero){
		return un_numero;
	}
	else{
		return otro_numero;
	}
}

uint32_t maximo(uint32_t un_numero , uint32_t otro_numero){
	if(un_numero>otro_numero){
		return un_numero;
	}
	else{
		return otro_numero;
	}
}

uint32_t buscar_frame_disponible(){
	uint32_t cantidad_de_frames = (memoria_principal->tamanio_memoria/memoria_principal->tamanio_pagina);
	for(uint32_t indice=0; indice<cantidad_de_frames; indice++){
		if(!bitarray_test_bit(bitarray, indice)){
			log_info(logger_pruebas, "Frame encontrado: %d", indice);
			return indice;
		}
	}
	return -1;
}

void cargar_a_memoria_segmentada(pcb_t* pcb, char* tareas, t_list* lista) {
	t_tabla_patota* tabla_patota = crear_tabla(pcb->pid);

	uint32_t direccion_pcb = encontrar_fit(sizeof(pcb_t));

	if(direccion_pcb == -1)
		compactar_memoria();

	t_fila_tabla_de_segmentos* fila_pcb = crear_fila(1, direccion_pcb, sizeof(pcb_t));
	list_add(tabla_patota->filas, fila_pcb);
	memcpy(memoria_principal->bloque_memoria_principal + direccion_pcb, pcb, sizeof(pcb_t));
	memoria_principal->memoria_disponible -= sizeof(pcb_t);


	list_add(tablas_segmentos, tabla_patota); //Tiene que estar aca en el medio si o si.


	uint32_t direccion_tareas = encontrar_fit(strlen(tareas)+1);

	if(direccion_tareas == -1)
		compactar_memoria();

	((pcb_t*)(memoria_principal->bloque_memoria_principal + direccion_pcb))->tareas = (uint32_t)direccion_tareas;

	t_fila_tabla_de_segmentos* fila_tareas = crear_fila(2, direccion_tareas, strlen(tareas)+1);
	list_add(tabla_patota->filas, fila_tareas);
	memcpy(memoria_principal->bloque_memoria_principal + direccion_tareas, tareas, strlen(tareas)+1);
	memoria_principal->memoria_disponible -= strlen(tareas)+1;

	uint32_t largo_lista = list_size(lista);
	uint32_t x, y;

	numero_segmento_actual = 3;

	for (int i = 3; i < largo_lista; i++) {
		tabla_patota->cantidad_de_tripulantes++;
		char* posx = get_token_at((char*) list_get(lista, i), '|', 0);
		char* posy = get_token_at((char*) list_get(lista, i), '|', 1);
		x = atoi(posx);
		y = atoi(posy);
		iniciar_tripulante(x, y, pcb->pid);
		free(posx);
		free(posy);
	}
}


void borrar_tcb_de_memoria(tcb_t* direccion_tcb) {
	if (strcmp(memoria_principal->tipo_memoria, "SEGMENTACION") == 0) {
		borrar_tcb_de_memoria_segmentada(direccion_tcb);
	}
	if (strcmp(memoria_principal->tipo_memoria, "PAGINACION") == 0) {
		//borrar_tcb_de_memoria_paginada(direccion_tcb);
	}
}


void eliminar_tabla(t_tabla_patota* tabla) {
	if (strcmp(memoria_principal->tipo_memoria, "SEGMENTACION") == 0) {
		log_info(logger_pruebas,"ASD INICIO");
		for (uint32_t i=0; i<list_size(tabla->filas); i++) {
			list_remove_and_destroy_element(tabla->filas,0,free);
			log_info(logger_pruebas,"ASD");
		}
	}

	else if (strcmp(memoria_principal->tipo_memoria, "PAGINACION") == 0) {
		log_info(logger_pruebas,"entre al else if PAGINACION");
		t_fila_tabla_de_paginas* fila_actual;
		uint32_t cant_paginas = list_size(tabla->filas);
		log_info(logger_pruebas, "cantidad paginas: %d", cant_paginas);

		for (uint32_t i=0; i<cant_paginas; i++) {
			log_info(logger_pruebas,"entre al for PAGINACION");
			fila_actual = (t_fila_tabla_de_paginas*)list_get(tabla->filas, 0);
			bitarray_clean_bit(bitarray, fila_actual->frame_asignado);
			t_fila_tabla_de_paginas* fila_a_sacar = (t_fila_tabla_de_paginas*)list_remove(tabla->filas, 0);
			log_info(logger_pruebas, "voy a hacer un free");
			free(fila_a_sacar);
			log_info(logger_pruebas, "hice el free");
		}
		log_info(logger_pruebas,"sali del for");

		//list_iterate(tabla->datos_tripulantes,liberar_listas_en_datos_tripulantes);
		log_info(logger_pruebas,"itere");
		//list_destroy_and_destroy_elements(tabla->datos_tripulantes);

		list_destroy(tabla->datos_pcb->paginas_que_ocupa);
		log_info(logger_pruebas,"destruyo paginas que ocupa");


		list_destroy(tabla->datos_tareas->paginas_que_ocupa);
		log_info(logger_pruebas,"destroy and destroy elements");

		free(tabla->datos_pcb);
		free(tabla->datos_tareas);
		free(tabla->datos_tripulantes);
	}
	t_tabla_patota* tabla_a_eliminar = list_remove(tablas_segmentos, get_indice_tabla(tabla));

	list_destroy(tabla_a_eliminar->filas);
	free(tabla_a_eliminar);
}

/*
void liberar_listas_en_datos_tripulantes(void* value){
	t_datos_tripulante* datos_tripulante = value;
	list_destroy(datos_tripulante->paginas_que_ocupa);
	free(datos_tripulante);
	log_info(logger_pruebas,"destroy paginas datos trip");


}*/

void borrar_tcb_de_memoria_segmentada(tcb_t* direccion_tcb) {
	uint32_t pid = ((pcb_t*)(memoria_principal->bloque_memoria_principal + direccion_tcb->puntero_pcb))->pid;
	t_tabla_patota* tabla = get_tabla_por_pid(pid);

	uint32_t cant_filas = list_size(tabla->filas);
	t_fila_tabla_de_segmentos* fila_actual;
	tcb_t* tcb_actual;
	uint32_t tid = direccion_tcb->tid;

	uint32_t indice_borrar = -1;
	tabla->cantidad_de_tripulantes--;

	for (uint32_t i=2; i<cant_filas; i++) {
		fila_actual = (t_fila_tabla_de_segmentos*)list_get(tabla->filas, i);
		tcb_actual = (tcb_t*)(memoria_principal->bloque_memoria_principal + fila_actual->inicio);
		if (tcb_actual->tid == tid) {
			indice_borrar = i;
			break;
		}
	}
	free(list_remove(tabla->filas, indice_borrar));
}

void borrar_tcb_de_memoria_paginada(uint32_t tid) {
	t_tabla_patota* tabla = get_tabla_por_tid(tid);

	uint32_t variable_del_for = 1;
	t_list* paginas = get_paginas_por_tid(tid);
	uint32_t cant_paginas = list_size(paginas);


	t_fila_tabla_de_paginas* pag_actual;

	uint32_t peso_muerto = sizeof(tcb_t);
	uint32_t offset_tcb = get_offset_tid(tid);

	uint32_t bytes_a_sacar = (uint32_t)(memoria_principal->tamanio_pagina - offset_tcb);

	uint32_t indice_tcb = get_indice_tid(tid);

	t_datos_tripulante* datos = list_remove(tabla->datos_tripulantes, indice_tcb);

	tabla->cantidad_de_tripulantes--;

	pag_actual = (t_fila_tabla_de_paginas*)list_get(paginas, 0);
	pag_actual->peso_actual -= bytes_a_sacar;
	peso_muerto -= bytes_a_sacar;
	if (pag_actual->peso_actual == 0) {

		if(pag_actual==puntero_victima_clock){
			avanzar_puntero_victima();
		}
		bitarray_clean_bit(bitarray, pag_actual->frame_asignado);
		list_remove(tabla->filas, get_indice_pagina(pag_actual));
		list_remove_and_destroy_element(paginas,get_indice_pagina_en_tripulante(datos,pag_actual),free);
		cant_paginas=list_size(paginas);
		variable_del_for=0;
	}

	for (uint32_t i=variable_del_for; i<cant_paginas; i++) {
		pag_actual = (t_fila_tabla_de_paginas*)list_get(paginas, i);
		bytes_a_sacar = minimo(peso_muerto, pag_actual->peso_actual);
		pag_actual->peso_actual -= bytes_a_sacar;
		peso_muerto -= bytes_a_sacar;
		if (pag_actual->peso_actual == 0) {
			if(pag_actual==puntero_victima_clock){
				avanzar_puntero_victima();
			}
			bitarray_clean_bit(bitarray, pag_actual->frame_asignado);
			list_remove(tabla->filas, get_indice_pagina(pag_actual));
			list_remove_and_destroy_element(paginas,get_indice_pagina_en_tripulante(datos,pag_actual),free);
			i--;
			cant_paginas = list_size(paginas);
		}
	}

	list_destroy(datos->paginas_que_ocupa);
	free(datos);
}

uint32_t get_indice_pagina_en_tripulante(t_datos_tripulante* datos, t_fila_tabla_de_paginas* pagina){
	uint32_t cant_paginas = list_size(datos->paginas_que_ocupa);
	t_fila_tabla_de_paginas* pagina_actual;

	for(uint32_t i=0; i<cant_paginas; i++){
		pagina_actual = (t_fila_tabla_de_paginas*)list_get(datos->paginas_que_ocupa, i);

		if(pagina_actual == pagina) {
			return i;
		}
	}
	return 0;
}


uint32_t get_indice_pagina(t_fila_tabla_de_paginas* pagina) {
	uint32_t cant_patotas = list_size(tablas_segmentos);
	uint32_t cant_paginas;
	t_tabla_patota* tabla_actual;
	t_fila_tabla_de_paginas* pagina_actual;

	for (uint32_t i=0; i<cant_patotas; i++) {
		tabla_actual = (t_tabla_patota*)list_get(tablas_segmentos, i);
		cant_paginas = list_size(tabla_actual->filas);
		for (uint32_t j=0; j<cant_paginas; j++) {
			pagina_actual = (t_fila_tabla_de_paginas*)list_get(tabla_actual->filas, j);

			if (pagina_actual == pagina) {
				return j;
			}
		}
	}
	return 0;
}

uint32_t get_indice_tid(uint32_t tid) {
	t_tabla_patota* tabla = get_tabla_por_tid(tid);
	uint32_t cant_tcbs = list_size(tabla->datos_tripulantes);

	t_datos_tripulante* dato_actual;
	for (uint32_t j=0; j<cant_tcbs; j++) {
		dato_actual = (t_datos_tripulante*)list_get(tabla->datos_tripulantes, j);
		if (dato_actual->id == tid) {
			return j;
		}
	}
	return 0;
}

uint32_t get_offset_tid(uint32_t tid) {
	t_tabla_patota* tabla = get_tabla_por_tid(tid);
	uint32_t indice = get_indice_tid(tid);

	t_datos_tripulante* datos = (t_datos_tripulante*)list_get(tabla->datos_tripulantes, indice);

	return datos->offset;
}

t_list* get_paginas_por_tid(uint32_t tid){

	log_info(logger_pruebas, "get_paginas_por_id: Empece bien");

	t_tabla_patota* tabla = get_tabla_por_tid(tid);


	if (tabla == NULL) {
		log_info(logger_pruebas, "Tabla NULL");
	}
	else
		log_info(logger_pruebas, "get_paginas_por_id: Tabla: %d", tabla->pid_asociado);

	uint32_t cant_tripulantes = list_size(tabla->datos_tripulantes);
	t_datos_tripulante* tcb_actual;

	for (uint32_t i=0; i<cant_tripulantes; i++) {
		tcb_actual = (t_datos_tripulante*)list_get(tabla->datos_tripulantes, i);
		if (tcb_actual->id == tid) {
			return tcb_actual->paginas_que_ocupa;
		}
	}
	return NULL;
}

t_tabla_patota* get_tabla_por_tid(uint32_t tid){
	t_tabla_patota* tabla = NULL;
	if (strcmp(memoria_principal->tipo_memoria, "SEGMENTACION") == 0) {
		pthread_mutex_lock(&mutex_comunicacion);
		tabla = get_tabla_por_tid_segmentacion(tid);
		pthread_mutex_unlock(&mutex_comunicacion);
	}
	else if (strcmp(memoria_principal->tipo_memoria, "PAGINACION") == 0) {
		tabla = get_tabla_por_tid_paginacion(tid);
	}
	return tabla;
}

t_tabla_patota* get_tabla_por_tid_segmentacion(uint32_t tid){
	uint32_t cant_patotas = list_size(tablas_segmentos);

	t_tabla_patota* tabla_actual;
	t_fila_tabla_de_segmentos* segmento_actual;
	tcb_t* tcb_actual;
	uint32_t cant_segmentos;

	for(uint32_t i=0; i<cant_patotas; i++){
		tabla_actual = (t_tabla_patota*)list_get(tablas_segmentos, i);
		cant_segmentos = list_size(tabla_actual->filas);

		for(uint32_t j=2; j<cant_segmentos; j++){
			segmento_actual = (t_fila_tabla_de_segmentos*)list_get(tabla_actual->filas, j);
			tcb_actual = (tcb_t*)(memoria_principal->bloque_memoria_principal + segmento_actual->inicio);

			if (tcb_actual->tid == tid) {
				return tabla_actual;
			}
		}
	}
	return NULL;
}

t_tabla_patota* get_tabla_por_tid_paginacion(uint32_t tid){
	uint32_t cant_patotas = list_size(tablas_segmentos);

	t_tabla_patota* tabla_actual;
	t_datos_tripulante* datos_t;

	for(uint32_t i=0; i<cant_patotas; i++){
		tabla_actual = (t_tabla_patota*)list_get(tablas_segmentos, i);

		for(uint32_t j=0; j<tabla_actual->cantidad_de_tripulantes; j++){
			datos_t = (t_datos_tripulante*)list_get(tabla_actual->datos_tripulantes, j);



			if (datos_t->id == tid) {
				log_info(logger_pruebas, "get_tabla_por_tid: voy a retornar la tabla %d", tabla_actual->pid_asociado);
				log_info(logger_pruebas, "get_tabla_por_tid: cant_tripulantes=%d", tabla_actual->cantidad_de_tripulantes);
				log_info(logger_pruebas, "get_tabla_por_tid: cant_tripulantes(calc)=%d", list_size(tabla_actual->datos_tripulantes));

				return tabla_actual;
			}
		}
	}
	return NULL;
}

uint32_t get_indice_tabla(t_tabla_patota* tabla) {
	t_tabla_patota* tabla_actual;
	uint32_t cant_patotas = list_size(tablas_segmentos);
	for (uint32_t i=0; i<cant_patotas; i++) {
		tabla_actual = (t_tabla_patota*)list_get(tablas_segmentos, i);
		if (tabla_actual->pid_asociado == tabla->pid_asociado) {
			return i;
		}
	}
	return -1;
}

tcb_t* get_tripulante_por_id(uint32_t tid) {
	tcb_t* tcb = NULL;
	if (strcmp(memoria_principal->tipo_memoria, "SEGMENTACION") == 0) {
		pthread_mutex_lock(&mutex_comunicacion);
		tcb = get_tripulante_por_id_segmentacion(tid);
		pthread_mutex_unlock(&mutex_comunicacion);

	}
	else if (strcmp(memoria_principal->tipo_memoria, "PAGINACION") == 0) {
		tcb = get_tripulante_por_id_paginacion(tid);
	}
	return tcb;
}

tcb_t* get_tripulante_por_id_segmentacion(uint32_t tid) {
	uint32_t cant_patotas = list_size(tablas_segmentos);
	uint32_t cant_filas;
	tcb_t* tcb_actual;
	t_fila_tabla_de_segmentos* fila_actual;
	t_tabla_patota* tabla_actual;
	for (uint32_t i=0; i<cant_patotas; i++) {
		tabla_actual = (t_tabla_patota*)list_get(tablas_segmentos, i);
		cant_filas = list_size(tabla_actual->filas);
		for(uint32_t j=2; j<cant_filas; j++) {
			fila_actual = (t_fila_tabla_de_segmentos*)list_get(tabla_actual->filas, j);
			tcb_actual = (tcb_t*)(memoria_principal->bloque_memoria_principal+fila_actual->inicio);
			if (tcb_actual->tid == tid) {
				return tcb_actual;
			}
			//log_info(logger, "for chiquito %d", j);
		}
	}
	return NULL;
}

tcb_t* get_tripulante_por_id_paginacion(uint32_t tid){
	t_tabla_patota* tabla = get_tabla_por_tid(tid);
	uint32_t indice = get_indice_tid(tid);
	t_datos_tripulante* datos_tripulante=(t_datos_tripulante*)list_get(tabla->datos_tripulantes, indice);
	return recuperar_tcb(datos_tripulante);
}

void actualizar_tripulante(tcb_t* tripulante){
	t_tabla_patota* tabla = get_tabla_por_tid(tripulante->tid);
	uint32_t indice = get_indice_tid(tripulante->tid);
	t_datos_tripulante* datos = (t_datos_tripulante*)list_get(tabla->datos_tripulantes, indice);
	t_list* paginas = datos->paginas_que_ocupa;
	t_fila_tabla_de_paginas* pagina_actual;
	//void* auxiliar_de_tripulante=malloc(sizeof(tcb_t));
	//memcpy(auxiliar_de_tripulante,tripulante,sizeof(tcb_t));
	uint32_t offset = datos->offset;
	uint32_t bytes_recorridos = 0;
	uint32_t bytes_a_recorrer;

	uint32_t direccion_a_copiar;

	for (uint32_t i=0; bytes_recorridos<sizeof(tcb_t); i++) {
		pagina_actual = (t_fila_tabla_de_paginas*)list_get(paginas, i);
		bytes_a_recorrer = minimo(memoria_principal->tamanio_pagina - offset, sizeof(tcb_t) - bytes_recorridos);
		direccion_a_copiar = (uint32_t)(memoria_principal->bloque_memoria_principal + (pagina_actual->frame_asignado)*(memoria_principal->tamanio_pagina) + offset);

		memcpy((void*)direccion_a_copiar, (void*)tripulante+bytes_recorridos, bytes_a_recorrer);

		bytes_recorridos += bytes_a_recorrer;
		offset = 0;
	}
	actualizar_paginas(paginas);

	free(tripulante);
}

pcb_t* get_patota_por_id(uint32_t pid) {
	uint32_t cant_patotas = list_size(tablas_segmentos);
	t_tabla_patota* tabla_actual;
	t_fila_tabla_de_segmentos* fila_actual;
	for (uint32_t i=0; i<cant_patotas; i++) {
		tabla_actual = (t_tabla_patota*)list_get(tablas_segmentos, i);
		if (tabla_actual->pid_asociado == pid) {
			fila_actual = (t_fila_tabla_de_segmentos*)list_get(tabla_actual->filas, 0);
			return (pcb_t*)(memoria_principal->bloque_memoria_principal + fila_actual->inicio);
		}
	}
	return NULL;
}

t_tabla_patota* get_tabla_por_pid(uint32_t pid) {
	uint32_t cant_tablas = list_size(tablas_segmentos);
	for (uint32_t i=0; i<cant_tablas; i++) {
		t_tabla_patota* tabla_actual = (t_tabla_patota*)list_get(tablas_segmentos, i);
		if (tabla_actual->pid_asociado == pid) {
			return tabla_actual;
		}
	}
	return NULL;
}

char* tareas_del_pcb(pcb_t* direccion_pcb) {
	char* ret;
	ret = (char*)(memoria_principal->bloque_memoria_principal + direccion_pcb->tareas);
	return ret;
}

char* tareas_del_pcb_por_tid(uint32_t tid) {
	if (strcmp(memoria_principal->tipo_memoria, "SEGMENTACION") == 0) {
		tcb_t* tcb = get_tripulante_por_id(tid);
		uint32_t pid = ((pcb_t*)(memoria_principal->bloque_memoria_principal+tcb->puntero_pcb))->pid;
		pthread_mutex_lock(&mutex_comunicacion);
		pcb_t* pcb_asociado = (pcb_t*)get_patota_por_id(pid);
		pthread_mutex_unlock(&mutex_comunicacion);

		char* ret = string_new();
		string_append(&ret, tareas_del_pcb(pcb_asociado));
		string_append(&ret, "@FIN_DE_TAREAS 0;0;0;0");
		return ret;
	}
	if (strcmp(memoria_principal->tipo_memoria, "PAGINACION") == 0) {
		t_tabla_patota* tabla = get_tabla_por_tid(tid);
		return recuperar_tareas(tabla->datos_tareas, tabla->longitud_tareas);
	}
	return NULL;
}


t_list* paginas_en_swap_tcb(t_datos_tripulante* datos_tripulante){
	t_list* lista_de_paginas = list_create();
	t_fila_tabla_de_paginas* pagina_actual;
	uint32_t cant_paginas = list_size(datos_tripulante->paginas_que_ocupa);

	for(int i=0; i<cant_paginas; i++){
		pagina_actual = (t_fila_tabla_de_paginas*)list_get(datos_tripulante->paginas_que_ocupa, i);
		if(pagina_actual->bit_de_presencia == 0){
			list_add(lista_de_paginas, pagina_actual);
		}
	}
	return lista_de_paginas;

}

t_list* paginas_en_swap_pcb_o_tareas(t_datos* datos_tareas){
	t_list* lista_de_paginas = list_create();
	for(int i=0; i<list_size(datos_tareas->paginas_que_ocupa); i++){
		t_fila_tabla_de_paginas* pagina = (t_fila_tabla_de_paginas*)list_get(datos_tareas->paginas_que_ocupa,i);
		if(pagina->bit_de_presencia == 0){
			list_add(lista_de_paginas,pagina);
		}
	}
	return lista_de_paginas;
}

void meter_paginas_a_swap(t_list* paginas, t_list* contenidos) {
	uint32_t cantidad_de_paginas = list_size(paginas);
	uint32_t frame_libre;
	void* contenido;
	t_fila_tabla_de_paginas* pagina;
	archivo_swap = fopen(memoria_principal->path_swap, "r+b");
	for(int i=0; i<cantidad_de_paginas; i++){
		frame_libre = buscar_frame_disponible_en_disco();
		fseek(archivo_swap, frame_libre*memoria_principal->tamanio_pagina, SEEK_SET);
		contenido = list_get(contenidos,i);
		pagina = ((t_fila_tabla_de_paginas*)list_get(paginas,i));
		pagina->bit_de_presencia = 0;
		pagina->frame_asignado_en_swap = frame_libre;
		//bitarray_clean_bit(bitarray, pagina->frame_asignado); se libera en la otra funcion (contenido_de_paginas())
		bitarray_set_bit(bitarray_swap, frame_libre);
		fwrite(contenido, memoria_principal->tamanio_pagina, 1, archivo_swap);
	}
	free(contenido);
	fclose(archivo_swap);
	list_destroy(contenidos);
}

// IMPORTANTE: Libera el/los frame/s asignados
t_list* contenido_de_paginas(t_list* paginas){
	t_list* lista_a_devolver=list_create();
	t_fila_tabla_de_paginas* pagina_actual;
	for(int i=0; i < list_size(paginas) ;i++){
		pagina_actual = (t_fila_tabla_de_paginas*)list_get(paginas,i);
		void* variable_auxiliar = malloc(memoria_principal->tamanio_pagina);
		memcpy(variable_auxiliar, memoria_principal->bloque_memoria_principal+(pagina_actual->frame_asignado*memoria_principal->tamanio_pagina), memoria_principal->tamanio_pagina);
		list_add(lista_a_devolver, variable_auxiliar);
		bitarray_clean_bit(bitarray, pagina_actual->frame_asignado);
	}
	return lista_a_devolver;
}

void pasar_de_swap_a_memoria(t_list* paginas){
	void* memoria_a_copiar = malloc(memoria_principal->tamanio_pagina);
	uint32_t cant_paginas = list_size(paginas);
	t_fila_tabla_de_paginas* pagina_actual;
	uint32_t frame_a_asignar;
	uint32_t direccion_fisica;

	archivo_swap = fopen(memoria_principal->path_swap, "r+b");

	for (uint32_t i=0; i<cant_paginas; i++) {
		frame_a_asignar = buscar_frame_disponible();

		pagina_actual = (t_fila_tabla_de_paginas*)list_get(paginas, i);
		pagina_actual->bit_de_presencia = 1;
		pagina_actual->frame_asignado = frame_a_asignar;

		fseek(archivo_swap, pagina_actual->frame_asignado_en_swap*memoria_principal->tamanio_pagina, SEEK_SET);
		fread(memoria_a_copiar, memoria_principal->tamanio_pagina, 1, archivo_swap);

		direccion_fisica = (uint32_t)(memoria_principal->bloque_memoria_principal + frame_a_asignar*memoria_principal->tamanio_pagina);
		memcpy((void*)direccion_fisica, memoria_a_copiar, memoria_principal->tamanio_pagina);

		bitarray_clean_bit(bitarray_swap, pagina_actual->frame_asignado_en_swap);
		bitarray_set_bit(bitarray, frame_a_asignar);
	}
	free(memoria_a_copiar);
	fclose(archivo_swap);
}


tcb_t* recuperar_tcb(t_datos_tripulante* datos_tripulante) {

	actualizar_paginas(datos_tripulante->paginas_que_ocupa);
	t_list* lista_de_paginas_en_swap = paginas_en_swap_tcb(datos_tripulante); // Si hay alguna pagina en swap....
	uint32_t cant_pags_en_swap = list_size(lista_de_paginas_en_swap);
	if(cant_pags_en_swap > 0){
		t_list* victimas = seleccionar_victimas(cant_pags_en_swap);
		t_list* contenido = contenido_de_paginas(victimas);
		pasar_de_swap_a_memoria(lista_de_paginas_en_swap);
		meter_paginas_a_swap(victimas, contenido);
		list_destroy(victimas);

	}
	list_destroy(lista_de_paginas_en_swap);
	uint32_t offset = datos_tripulante->offset;
	t_list* paginas = datos_tripulante->paginas_que_ocupa;

	t_fila_tabla_de_paginas* pag_actual;
	uint32_t bytes_recorridos = 0;
	uint32_t bytes_a_recorrer;
	uint32_t direccion_actual;

	void* tcb_reconstruido = malloc(sizeof(tcb_t));
	list_add(lista_de_tcb_reconstruidos,tcb_reconstruido);

	for (uint32_t i=0; bytes_recorridos < sizeof(tcb_t); i++) {
		bytes_a_recorrer = minimo(memoria_principal->tamanio_pagina - offset, sizeof(tcb_t) - bytes_recorridos);
		pag_actual = (t_fila_tabla_de_paginas*)list_get(paginas, i);

		direccion_actual = (uint32_t)(memoria_principal->bloque_memoria_principal + (pag_actual->frame_asignado)*(memoria_principal->tamanio_pagina) + offset);

		memcpy(tcb_reconstruido+bytes_recorridos, (void*)direccion_actual, bytes_a_recorrer);

		bytes_recorridos += bytes_a_recorrer;
		offset = 0;

	}

	return (tcb_t*)tcb_reconstruido;
}

pcb_t* recuperar_pcb(t_datos* datos_pcb) {
	actualizar_paginas(datos_pcb->paginas_que_ocupa);
	t_list* lista_de_paginas_en_swap = paginas_en_swap_pcb_o_tareas(datos_pcb); // Si hay alguna pagina en swap....
		uint32_t cant_pags_en_swap = list_size(lista_de_paginas_en_swap);
		if(cant_pags_en_swap > 0){
			t_list* victimas = seleccionar_victimas(cant_pags_en_swap);
			t_list* contenido=contenido_de_paginas(victimas);
			pasar_de_swap_a_memoria(lista_de_paginas_en_swap);
			meter_paginas_a_swap(victimas,contenido);
			list_destroy(victimas);
			//list_destroy(contenido);
		}

	uint32_t offset = datos_pcb->offset;
	t_list* paginas = datos_pcb->paginas_que_ocupa;

	t_fila_tabla_de_paginas* pag_actual;
	uint32_t bytes_recorridos = 0;
	uint32_t bytes_a_recorrer;
	uint32_t direccion_actual;

	pcb_t* pcb_reconstruido = malloc(sizeof(pcb_t));

	for (uint32_t i=0; bytes_recorridos < sizeof(pcb_t); i++) {
		bytes_a_recorrer = minimo(memoria_principal->tamanio_pagina - offset, sizeof(pcb_t) - bytes_recorridos);
		pag_actual = (t_fila_tabla_de_paginas*)list_get(paginas, i);
		direccion_actual = (uint32_t)(memoria_principal->bloque_memoria_principal + (pag_actual->frame_asignado)*(memoria_principal->tamanio_pagina) + offset);

		memcpy(pcb_reconstruido+bytes_recorridos, (void*)direccion_actual, bytes_a_recorrer);

		bytes_recorridos += bytes_a_recorrer;
		offset = 0;
	}
	return pcb_reconstruido;
	//todo: hay que hacer un free de este pcb, donde este llamando a esta funcion, una vez que lo termine de usar
}

char* recuperar_tareas(t_datos* datos_tareas, uint32_t length) {
	actualizar_paginas(datos_tareas->paginas_que_ocupa);
	t_list* lista_de_paginas_en_swap = paginas_en_swap_pcb_o_tareas(datos_tareas); // Si hay alguna pagina en swap....
	uint32_t cant_pags_en_swap = list_size(lista_de_paginas_en_swap);
	if(cant_pags_en_swap > 0){

		if(cantidad_frames_disponibles()<cant_pags_en_swap){
			t_list* victimas = seleccionar_victimas(cant_pags_en_swap - cantidad_frames_disponibles());
			t_list* contenido=contenido_de_paginas(victimas);
			pasar_de_swap_a_memoria(lista_de_paginas_en_swap);
			meter_paginas_a_swap(victimas,contenido);
			list_destroy(victimas);
		}else{
			pasar_de_swap_a_memoria(lista_de_paginas_en_swap);
		}
		//list_destroy(contenido);
	}
	list_destroy(lista_de_paginas_en_swap);
	uint32_t offset = datos_tareas->offset;
	t_list* paginas = datos_tareas->paginas_que_ocupa;

	t_fila_tabla_de_paginas* pag_actual;
	uint32_t bytes_recorridos = 0;
	uint32_t bytes_a_recorrer;
	uint32_t direccion_actual;

	char* tareas_reconstruidas = malloc(length+1);

	for (uint32_t i=0; bytes_recorridos < length+1; i++) {
		bytes_a_recorrer = minimo(memoria_principal->tamanio_pagina - offset, length+1 - bytes_recorridos);
		pag_actual = (t_fila_tabla_de_paginas*)list_get(paginas, i);
		direccion_actual = (uint32_t)(memoria_principal->bloque_memoria_principal + (pag_actual->frame_asignado)*(memoria_principal->tamanio_pagina) + offset);

		memcpy(tareas_reconstruidas+bytes_recorridos, (void*)direccion_actual, bytes_a_recorrer);

		bytes_recorridos += bytes_a_recorrer;
		offset = 0;
	}
	return tareas_reconstruidas;

}

// Recibe lista de paginas, les pone el bit de uso en 1 y el tiempo actual
void actualizar_paginas(t_list* paginas) {
	uint32_t cant_paginas = list_size(paginas);
	t_fila_tabla_de_paginas* pagina_actual;

	for (uint32_t i=0; i<cant_paginas; i++) {
		pagina_actual = (t_fila_tabla_de_paginas*)list_get(paginas, i);

		char* string_time = temporal_get_string_time("%H%M%S");
		pagina_actual->bit_de_uso = 1;
		pagina_actual->ultima_referencia = atoi(string_time);
		free(string_time);
	}
}


t_fila_tabla_de_paginas* obtener_victima() {
	t_fila_tabla_de_paginas* victima = NULL;
	if (strcmp(memoria_principal->algoritmo_reemplazo, "LRU") == 0) {
		victima = obtener_victima_lru();
	}
	else if (strcmp(memoria_principal->algoritmo_reemplazo, "CLOCK") == 0) {
		victima = obtener_victima_clock();
	}

	return victima;
}

t_list* seleccionar_victimas(uint32_t cantidad_paginas) {
	t_list* paginas_victimas = list_create();
	t_fila_tabla_de_paginas* pagina_actual;

	for (uint32_t i=0; i<cantidad_paginas; i++) {
		pagina_actual = obtener_victima();
		list_add(paginas_victimas, pagina_actual);
	}

	return paginas_victimas;

}

t_fila_tabla_de_paginas* obtener_victima_lru() {
	uint32_t cant_patotas = list_size(tablas_segmentos);
	uint32_t cant_paginas;
	t_tabla_patota* tabla_actual;
	t_fila_tabla_de_paginas* pagina_actual;
	t_fila_tabla_de_paginas* victima_actual = NULL;
	for (uint32_t i=0; i<cant_patotas; i++) {
		tabla_actual = (t_tabla_patota*)list_get(tablas_segmentos, i);
		cant_paginas = list_size(tabla_actual->filas);
		for (uint32_t j=0; j<cant_paginas; j++) {
			pagina_actual = (t_fila_tabla_de_paginas*)list_get(tabla_actual->filas, j);
			log_info(logger_pruebas, "pag_actual=%d", pagina_actual->numero_de_pagina);
			log_info(logger_pruebas, "pag_actual_TUR=%d", pagina_actual->ultima_referencia);

			if ((victima_actual == NULL || pagina_actual->ultima_referencia < victima_actual->ultima_referencia) && pagina_actual->bit_de_presencia == 1) {
				victima_actual = pagina_actual;
				log_info(logger_pruebas, "cheto");
			}
		}
	}
	victima_actual->bit_de_presencia=0;
	return victima_actual;
}

t_fila_tabla_de_paginas* obtener_victima_clock() {
	uint32_t cant_patotas = list_size(tablas_segmentos);
	uint32_t cant_paginas;
	t_tabla_patota* tabla_actual;
	uint32_t indice_tabla;
	uint32_t indice_pagina;
	buscar_indices_victima_clock(&indice_tabla, &indice_pagina);
	t_fila_tabla_de_paginas* victima_definitiva;

	for (uint32_t vuelta=0; vuelta<=1; vuelta++) {
		for (uint32_t i=indice_tabla; i<cant_patotas; i++) {
			tabla_actual = (t_tabla_patota*)list_get(tablas_segmentos, i);
			cant_paginas = list_size(tabla_actual->filas);

			for (uint32_t j=indice_pagina; j<cant_paginas; j++) {
				puntero_victima_clock = list_get(tabla_actual->filas, j);

				if (puntero_victima_clock->bit_de_uso == 0 && puntero_victima_clock->bit_de_presencia == 1) {
					victima_definitiva = puntero_victima_clock;
					avanzar_puntero_victima();
					victima_definitiva->bit_de_presencia=0;
					return victima_definitiva;
				}
				else {
					puntero_victima_clock->bit_de_uso = 0;
					avanzar_puntero_victima();
				}
			}
			indice_pagina = 0;
		}
		indice_tabla = 0;
	}
	return NULL;
}

void avanzar_puntero_victima() {
	uint32_t cant_patotas = list_size(tablas_segmentos);
	uint32_t cant_paginas;
	t_tabla_patota* tabla_actual;
	t_fila_tabla_de_paginas* pagina_actual;

	for (uint32_t i=0; i<cant_patotas; i++) {
		tabla_actual = (t_tabla_patota*)list_get(tablas_segmentos, i);
		cant_paginas = list_size(tabla_actual->filas);
		for (uint32_t j=0; j<cant_paginas; j++) {
			pagina_actual = (t_fila_tabla_de_paginas*)list_get(tabla_actual->filas, j);

			if (puntero_victima_clock == pagina_actual) {
				if (j == cant_paginas-1) {
					j = 0;
					if (i == cant_patotas-1) {
						i = 0;
					}
					else {
						i++;
					}
				}
				else {
					j++;
				}
				tabla_actual = (t_tabla_patota*)list_get(tablas_segmentos, i);
				puntero_victima_clock = (t_fila_tabla_de_paginas*)list_get(tabla_actual->filas, j);
				break;
			}
		}
	}
}

void buscar_indices_victima_clock(uint32_t* i_t, uint32_t* i_p) {
	uint32_t cant_patotas = list_size(tablas_segmentos);
	uint32_t cant_paginas;
	t_tabla_patota* tabla_actual;
	t_fila_tabla_de_paginas* pagina_actual;

	for (uint32_t i=0; i<cant_patotas; i++) {
		tabla_actual = (t_tabla_patota*)list_get(tablas_segmentos, i);
		cant_paginas = list_size(tabla_actual->filas);
		for (uint32_t j=0; j<cant_paginas; j++) {
			pagina_actual = (t_fila_tabla_de_paginas*)list_get(tabla_actual->filas, j);

			if (puntero_victima_clock == pagina_actual) {
				*i_t = i;
				*i_p = j;
				break;
			}
		}
	}
}


