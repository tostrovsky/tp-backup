/*
 * token.c
 *
 *  Created on: 16 jun. 2021
 *      Author: utnso
 */

#include "token.h"


int token_count(char* s, char sep)
{
	if (strlen(s) == 0) {
		return 0;
	} else {
		return char_count(s, sep) + 1;
	}
}

char* get_token_at(char* s, char sep, int posicion)
{
	int desde;
	int hasta;
	
	if (posicion == 0) {
		desde = 0;
		if(token_count(s, sep) == 1){
			return s;
		}
		hasta = index_of_n(s, sep, 0);
	}
	else if (posicion == token_count(s, sep)-1){
		desde = index_of_n(s, sep, posicion-1)+1;
		hasta = strlen(s);
	}
	else {
		desde = index_of_n(s, sep, posicion-1)+1;
		hasta = index_of_n(s, sep, posicion);
	}
	
	int nuevo_largo = hasta-desde;
	
	char* token = malloc(nuevo_largo+1);
	memcpy(token, s+desde, nuevo_largo);
	token[nuevo_largo] = '\0';
	return token;
}

int char_count(char* s, char c) {
	int m;
	int charcount;

	charcount = 0;
	for (m = 0; s[m] != '\0'; m++) {
		if (s[m] == c) {
			charcount++;
		}
	}
	return charcount;
}

int index_of_n(char* s, char c, int ocurrencia) {
	for (int i=0; i < strlen(s); i++) {
		if (s[i] == c) {
			if (ocurrencia > 0)
				ocurrencia--;
			else
				return i;
		}
	}
	return -1;
}

char* remove_token_at(char** string, char sep, int index){

	char* ret = string_new();
	char* ns = string_new();
	int n = token_count(*string, sep);
	for(int i=0;i<n;i++){
		char* tk = get_token_at(*string, sep, i);
		if(i != index){
			if(i){
				string_append(&ns, ",");
			}
			string_append(&ns, tk);
		}else{
			string_append(&ret, tk);
		}
	}
	*string = ns;
	free(ns);
	return ret;
}
