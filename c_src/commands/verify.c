#include "../commands.h"

void hsm_verify(dongleHandle dongle, ETERM* args){
	if (dongle == NULL) {
		ERL_WRITE_ERROR("not_found");
		return;
	}
	return;
}
