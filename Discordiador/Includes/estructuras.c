/*
 * estructuras.c
 *
 *  Created on: 22 jun. 2021
 *      Author: utnso
 */

#include "estructuras.h"


t_tripulante crear_tripulante(uint32_t id, char est, t_posicion posicion, uint32_t expulsado, uint32_t p_pcb) {
	t_tripulante tripulante;
	tripulante.id = id;
	tripulante.est = est;
	tripulante.posicion = posicion;
	tripulante.expulsado = expulsado;
	tripulante.patota_id = p_pcb;
	tripulante.quantum_disponible = quantum;
	tripulante.esta_ejecutando = 0;
	tripulante.atiende_sabotaje = 0;

	log_info(logger, "Se ha creado el tripulante %d", id);
	return tripulante;
}
t_tarea* crear_tarea(char* nombre, uint32_t cantidad, uint32_t pos_x, uint32_t pos_y, uint32_t tiempo) {
	t_tarea* tarea = malloc(sizeof(t_tarea));
	//tarea->nombre = (char*) string_new();
	//string_append(&tarea->nombre, nombre);
	tarea->nombre = nombre;
	tarea->cantidad = cantidad;
	tarea->pos_x = pos_x;
	tarea->pos_y = pos_y;
	tarea->tiempo = tiempo;
	return tarea;
}

void mapear_valores_del_config(t_config* config) {
	logger_discordiador = config_get_string_value(config, "LOG_FILE");
	ip_i_mongo_store = config_get_string_value(config, "IP_I_MONGO_STORE");
	ip_mi_ram_hq = config_get_string_value(config, "IP_MI_RAM_HQ");
	puerto_i_mongo_store = config_get_string_value(config, "PUERTO_I_MONGO_STORE");
	puerto_mi_ram_hq = config_get_string_value(config, "PUERTO_MI_RAM_HQ");
	grado_mutitarea = config_get_int_value(config, "GRADO_MULTITAREA");
	algoritmo = config_get_string_value(config, "ALGORITMO");
	quantum = config_get_int_value(config, "QUANTUM");
	duracion_sabotaje = config_get_int_value(config, "DURACION_SABOTAJE");
	retardo_ciclo_cpu = config_get_int_value(config, "RETARDO_CICLO_CPU")*0.5;
	ip_discordiador = config_get_string_value(config, "IP_DISCORDIADOR");
	puerto_discordiador = config_get_string_value(config, "PUERTO_DISCORDIADOR");
}

