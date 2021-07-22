/*
 * archivos.h
 *
 *  Created on: 17 jun. 2021
 *      Author: utnso
 */

#ifndef INCLUDES_ARCHIVOS_H_
#define INCLUDES_ARCHIVOS_H_

#include<stdio.h>
#include<stdlib.h>
#include<netdb.h>
#include<commons/string.h>

uint32_t leer_linea(char**, FILE**);
void leer_palabra(char**, FILE**);
uint32_t contar_renglones(char*);

#endif /* INCLUDES_ARCHIVOS_H_ */
