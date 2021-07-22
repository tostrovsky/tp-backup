#ifndef DISCORDIADOR_H_
#define DISCORDIADOR_H_

#include "utils.h"

t_log* iniciar_logger(char*);
t_config* leer_config(void);
void leer_consola(t_log*);
void crear_servidor();
//void paquete(int);
void terminar_programa(t_log*, t_config*);




#endif /* DISCORDIADOR_H_ */
