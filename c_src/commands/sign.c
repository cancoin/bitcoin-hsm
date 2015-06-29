#include "../commands.h"

void hsm_sign(dongleHandle dongle, ETERM* args){
	unsigned char in[260];
	unsigned char out[260];
	ETERM *typep;
	ETERM *encodedKey;
	ETERM *hash;
	int encodedKeyLength;
	int hashLength;
	unsigned char kType;
	const char* type;
	int result;
	int sw;
	int apduSize;
	ETERM *reply;
	int reply_bytes;
	ETERM *binreply;

	typep = erl_element(2, args);
	type = (const char*)ERL_ATOM_PTR(typep);
	if (strcasecmp(type, "random") == 0) {
		kType = 0x00;
	} else if (strcasecmp(type, "deterministic") == 0) {
		kType = 0x80;
	} else {
		ERL_WRITE_ERROR("badarg")
		return;
	}

	encodedKey = erl_element(3, args);
	encodedKeyLength = ERL_BIN_SIZE(encodedKey);
	if (encodedKeyLength < 0) {
		ERL_WRITE_ERROR("badarg");
		return;
	}

	hash = erl_element(4, args);
	hashLength = ERL_BIN_SIZE(hash);
	if (hashLength < 0) {
		ERL_WRITE_ERROR("badarg");
		return;
	}

	apduSize = 0;
	in[apduSize++] = BTCHIP_CLA;
	in[apduSize++] = BTCHIP_INS_SIGNVERIFY_IMMEDIATE;
	in[apduSize++] = 0x00;
	in[apduSize++] = kType;
	in[apduSize++] = 0x00;
	in[apduSize++] = encodedKeyLength;
	memcpy(in + apduSize, ERL_BIN_PTR(encodedKey), 71);
	apduSize += encodedKeyLength;
	in[apduSize++] = hashLength;
	memcpy(in + apduSize, ERL_BIN_PTR(hash), 32);
	apduSize += hashLength;
	in[OFFSET_CDATA] = (apduSize - 5);

	result = sendApduDongle(dongle, in, apduSize, out, sizeof(out), &sw);

	displayBinary(in, apduSize);
	if (result < 0) {
		ERL_WRITE_ERROR("ioerror");
		return;
	}
	displayBinary(out, result);
	if (sw != SW_OK) {
		ERL_WRITE_ERROR("dongle_error");
		return;
	}

	binreply = erl_mk_binary((char*)out, result);
	reply = erl_format("{ok, ~w}", binreply);
	reply_bytes = erl_term_len(reply);
	unsigned char reply_buffer[reply_bytes];
	erl_encode(reply, reply_buffer);
	write_cmd(reply_buffer, reply_bytes);
	erl_free_compound(reply);

	return;
}
