#include "../commands.h"

void hsm_import(dongleHandle dongle, ETERM* args){
	if (dongle == NULL) {
		ERL_WRITE_ERROR("not_found");
		return;
	}
	return;
}
