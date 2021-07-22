/*
 * archivos.c
 *
 *  Created on: 16 jun. 2021
 *      Author: utnso
 */

#include"archivos.h"

uint32_t leer_linea(char** destino, FILE** archivo) { //retorna 0 si se acabo el archivo
	int ret = 1;
	char* aux = malloc(2);
	char c = fgetc(*archivo);
	while (c != EOF && c != '\n') {
		aux[0] = c;
		aux[1] = '\0';
		string_append(destino, aux);
		c = fgetc(*archivo);
		if (c == EOF)
			ret = 0;
	}
	free(aux);
	return ret;
}

void leer_palabra(char** destino, FILE** archivo) {
	char* aux = malloc(2);
	char c = fgetc(*archivo);
	while (c != EOF && c != ' ') {
		aux[0] = c;
		aux[1] = '\0';
		string_append(destino, aux);
		c = fgetc(*archivo);
	}
	free(aux);
}

uint32_t contar_renglones(char* path) {
	FILE* archivo;
	archivo = fopen(path, "r+");
	char c = fgetc(archivo);
	uint32_t contador = 1;
	while (c != EOF) {
		if (c == '\n') {
			contador++;
		}
		c = fgetc(archivo);
	}
	fclose(archivo);
	return contador;
}
