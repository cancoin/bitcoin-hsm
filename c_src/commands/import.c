#include "../commands.h"

#define FORMAT_BASE58 0x01
#define FORMAT_SEED 0x02

void hsm_import(dongleHandle dongle, ETERM* args){
	if (dongle == NULL) {
		ERL_WRITE_ERROR("not_found");
		return;
	}
	unsigned char in[260];
	unsigned char out[260];
	ETERM *typep;
	ETERM *seedp;
	const char* type;
	int seedLength;
	int result;
	int sw;
	int apduSize;
	int importFormat;

	typep = erl_element(2, args);
	seedp = erl_element(3, args);

	type = (const char*)ERL_ATOM_PTR(typep);

	if (strcasecmp(type, "base58") == 0) {
		importFormat = FORMAT_BASE58;
	}
	else
	if (strcasecmp(type, "bip32seed") == 0) {
		importFormat = FORMAT_SEED;
	}
	else {
		ERL_WRITE_ERROR("badarg")
		return;
	}


	seedLength = ERL_BIN_SIZE(seedp);

	if (importFormat == FORMAT_BASE58) {
		if ((seedLength == 0) || (seedLength > 255)) {
			ERL_WRITE_ERROR("badarg")
			return;
		}
	}
	else {
		if ((seedLength < 0) || (seedLength == 65)) {
			ERL_WRITE_ERROR("badarg")
			return ;
		}
	}

	apduSize = 0;
	in[apduSize++] = BTCHIP_CLA;
	in[apduSize++] = BTCHIP_INS_IMPORT_PRIVATE_KEY;
	in[apduSize++] = importFormat;
	in[apduSize++] = 0x00;
	in[apduSize++] = 0x00;
	memcpy(in + apduSize, (char *)ERL_BIN_PTR(seedp), ERL_BIN_SIZE(seedp));
	apduSize += ERL_BIN_SIZE(seedp);
	in[OFFSET_CDATA] = (apduSize - 5);
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
	binreply = erl_mk_binary((char*)out, sizeof(out));
	reply = erl_format("{ok, ~w}", binreply);
	reply_bytes = erl_term_len(reply);
	byte reply_buffer[reply_bytes];
	erl_encode(reply, reply_buffer);
	write_cmd(reply_buffer, reply_bytes);
	erl_free_compound(reply);

	return;
}
