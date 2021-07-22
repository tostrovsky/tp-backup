/*
 * token.h
 *
 *  Created on: 16 jun. 2021
 *      Author: utnso
 */

#ifndef INCLUDES_TOKEN_H_
#define INCLUDES_TOKEN_H_

#include <string.h>
#include <stdlib.h>

int token_count(char*, char);
char* get_token_at(char*, char, int);
int char_count(char*, char);
int index_of_n(char*, char, int);

#endif /* INCLUDES_TOKEN_H_ */
