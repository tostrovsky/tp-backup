#ifndef MI_RAM_HQ_H_
#define MI_RAM_HQ_H_

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<commons/log.h>
#include "utils.h"
#include<signal.h>

void liberar_memoria();
void enviar_senial(int);
void liberar_tablas_patotas(void* value);
pthread_t hilo_server;
sem_t terminar_prog;
void liberar_tablas_extranias();
void liberar_listas_en_datos_tripulantes(void* value);
#endif /* MI_RAM_HQ_H_ */
