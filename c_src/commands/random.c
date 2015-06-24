#include "../commands.h"

void hsm_random(dongleHandle dongle, ETERM* args){
	if (dongle == NULL) {
		ERL_WRITE_ERROR("not_found");
		return;
	}

	unsigned char in[260];
	unsigned char out[260];
	int result;
	int sw;
	int apduSize;
	ETERM *size;

	size = erl_element(2, args);

	if (!ERL_IS_INTEGER(size)) {
		ERL_WRITE_ERROR("badarg");
		return;
	}

	apduSize = 0;
	in[apduSize++] = BTCHIP_CLA;
	in[apduSize++] = BTCHIP_INS_GET_RANDOM;
	in[apduSize++] = 0x00;
	in[apduSize++] = 0x00;
	in[apduSize++] = (int)ERL_INT_VALUE(size);
	result = sendApduDongle(dongle, in, apduSize, out, sizeof(out), &sw);
	if (result < 0) {
		ERL_WRITE_ERROR("ioerror");
		return;
	}
	if (sw != SW_OK) {
		ERL_WRITE_ERROR("dongle_error");
		return;
	}

	ETERM *reply;
	int reply_bytes;

	ETERM *binreply;
	binreply = erl_mk_binary((char*)out, (int)ERL_INT_VALUE(size));
	reply = erl_format("{ok, ~w}", binreply);
	reply_bytes = erl_term_len(reply);
	byte reply_buffer[reply_bytes];
	erl_encode(reply, reply_buffer);
	write_cmd(reply_buffer, reply_bytes);
	erl_free_compound(reply);

	return;
}
