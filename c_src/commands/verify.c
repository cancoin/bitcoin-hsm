#include "../commands.h"

void hsm_verify(dongleHandle dongle, ETERM* args){
	unsigned char in[260];
	unsigned char out[260];
	ETERM *publicKey;
	ETERM *hash;
	ETERM *signature;
	int publicKeyLength;
	int hashLength;
	int signatureLength;
	int result;
	int sw;
	int apduSize;
	ETERM *reply;
	int reply_bytes;

	fprintf(stderr, "here");
	publicKey = erl_element(2, args);
	publicKeyLength = ERL_BIN_SIZE(publicKey);
	if (publicKeyLength < 0) {
		ERL_WRITE_ERROR("badarg");
		return;
	}

	hash = erl_element(3, args);
	hashLength = ERL_BIN_SIZE(hash);
	if (hashLength < 0) {
		ERL_WRITE_ERROR("badarg");
		return;
	}

	signature = erl_element(4, args);
	signatureLength = ERL_BIN_SIZE(signature);
	if (signatureLength < 0) {
		ERL_WRITE_ERROR("badarg");
		return;
	}

	apduSize = 0;
	in[apduSize++] = BTCHIP_CLA;
	in[apduSize++] = BTCHIP_INS_SIGNVERIFY_IMMEDIATE;
	in[apduSize++] = 0x80;
	in[apduSize++] = 0x00;
	in[apduSize++] = 0x00;
	in[apduSize++] = publicKeyLength;
	memcpy(in + apduSize, publicKey, publicKeyLength);
	apduSize += publicKeyLength;
	in[apduSize++] = hashLength;
	memcpy(in + apduSize, hash, hashLength);
	apduSize += hashLength;
	memcpy(in + apduSize, signature, signatureLength);
	apduSize += signatureLength;
	in[OFFSET_CDATA] = (apduSize - 5);
	result = sendApduDongle(dongle, in, apduSize, out, sizeof(out), &sw);

	if (result < 0) {
		ERL_WRITE_ERROR("ioerror");
		return;
	}
	displayBinary(out, result);
	if (sw != SW_OK) {
		ERL_WRITE_ERROR("dongle_error");
		return;
	}

	reply = erl_format("{ok, true}");
	reply_bytes = erl_term_len(reply);
	unsigned char reply_buffer[reply_bytes];
	erl_encode(reply, reply_buffer);
	write_cmd(reply_buffer, reply_bytes);
	erl_free_compound(reply);

	return;
}
