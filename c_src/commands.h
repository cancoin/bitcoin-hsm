
#ifndef __COMMANDS_H__

#define __COMMANDS_H__

#include "hsmport.h"

void hsm_derive(dongleHandle dongle, ETERM* args);
void hsm_import(dongleHandle dongle, ETERM* args);
void hsm_pin(dongleHandle dongle, ETERM* args);
void hsm_pubkey(dongleHandle dongle, ETERM* args);
void hsm_random(dongleHandle dongle, ETERM* args);
void hsm_sign(dongleHandle dongle, ETERM* args);
void hsm_verify(dongleHandle dongle, ETERM* args);

#endif
