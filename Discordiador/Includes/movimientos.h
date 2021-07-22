/*
 * movimientos.h
 *
 *  Created on: 22 jun. 2021
 *      Author: utnso
 */

#ifndef INCLUDES_MOVIMIENTOS_H_
#define INCLUDES_MOVIMIENTOS_H_

#include "estructuras.h"

void mover_tripulante_a_destino(t_tripulante* tripulante, t_posicion destino);
void mover_planificando_a_destino(t_planificando* planificando, t_posicion destino);
void mover_tripulante(t_tripulante* tripulante, uint32_t direccion);
void mover_al_estilo_round_robin(t_planificando * planificando, t_posicion destino);

#endif /* INCLUDES_MOVIMIENTOS_H_ */
