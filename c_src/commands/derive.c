#include "../commands.h"

void hsm_derive(dongleHandle dongle, ETERM* args){
	unsigned char in[260];
	unsigned char out[260];
	int encodedKeyLength;
	uint32_t index;
	int result;
	int sw;
	int apduSize;
	ETERM* encodedKeyp;
	ETERM *indexp;
	ETERM *reply;
	int reply_bytes;
	ETERM *binreply;

	encodedKeyp = erl_element(2, args);
	indexp = erl_element(3, args);

	encodedKeyLength = ERL_BIN_SIZE(encodedKeyp);
	if (encodedKeyLength < 0) {
		ERL_WRITE_ERROR("badarg");
		return;
	}

	index = (uint32_t)ERL_INT_UVALUE(indexp);

	apduSize = 0;
	in[apduSize++] = BTCHIP_CLA;
	in[apduSize++] = BTCHIP_INS_DERIVE_BIP32_KEY;
	in[apduSize++] = 0x00;
	in[apduSize++] = 0x00;
	in[apduSize++] = 0x00;
	in[apduSize++] = encodedKeyLength;
	memcpy(in + apduSize, ERL_BIN_PTR(encodedKeyp), encodedKeyLength);
	apduSize += encodedKeyLength;
	writeUint32BE(in + apduSize, index);
	apduSize += sizeof(index);
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

	binreply = erl_mk_binary((char*)out, result);
	reply = erl_format("{ok, ~w}", binreply);
	reply_bytes = erl_term_len(reply);
	unsigned char reply_buffer[reply_bytes];
	erl_encode(reply, reply_buffer);
	write_cmd(reply_buffer, reply_bytes);
	erl_free_compound(reply);

	return;
}
