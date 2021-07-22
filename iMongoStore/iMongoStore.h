#ifndef I_MONGO_STORE_H_
#define I_MONGO_STORE_H_

#include "utils.h"
#include "filesystem.h"
#include "signal.h"

char* ip_i_mongo_store;
char* puerto_i_mongo_store;
char* logger_i_mongo_store;

void iniciar_config_ims();
void rutina(int);
void dar_comienzo_a_la_inicializacion_de_semaforos_habidos();
void liberar_metadata_t(char* key , void* value);

#endif /* I_MONGO_STORE_H_ */
