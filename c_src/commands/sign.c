#include "../commands.h"

void hsm_sign(dongleHandle dongle, ETERM* args){
	if (dongle == NULL) {
		ERL_WRITE_ERROR("not_found");
		return;
	}

	ETERM *keyp;
	ETERM *sighashp;

	keyp = erl_element(1, args);
	sighashp = erl_element(2, args);

	unsigned char in[260];
	unsigned char out[260];
	unsigned char encodedKey[100];
	unsigned char hash[32];
	unsigned char kType = 0x80;
	int encodedKeyLength;
	int hashLength;
	int result;
	int sw;
	int apduSize;


	int key_len = ERL_BIN_SIZE(keyp);
	char *key;
	key = malloc((key_len + 1) * sizeof(char));
	memcpy(key, (char *) ERL_BIN_PTR(keyp), key_len);
	key[key_len + 1] = '\0';

	int sighash_len = ERL_BIN_SIZE(sighashp);
	char *sighash;
	sighash = malloc((sighash_len + 1) * sizeof(char));
	memcpy(sighash, (char *) ERL_BIN_PTR(sighashp), sighash_len);
	sighash[sighash_len + 1] = '\0';

	encodedKeyLength = hexToBin(key, encodedKey, sizeof(encodedKey));
	if (encodedKeyLength < 0) {
		ERL_WRITE_ERROR("invalid_encoded_index");
		return;
	}

	hashLength = hexToBin(sighash, hash, sizeof(hash));
	if (hashLength < 0) {
		ERL_WRITE_ERROR("invalid_hash_length");
		return;
	}

	apduSize = 0;
	in[apduSize++] = BTCHIP_CLA;
	in[apduSize++] = BTCHIP_INS_SIGNVERIFY_IMMEDIATE;
	in[apduSize++] = 0x00;
	in[apduSize++] = kType;
	in[apduSize++] = 0x00;
	in[apduSize++] = encodedKeyLength;
	memcpy(in + apduSize, encodedKey, encodedKeyLength);
	apduSize += encodedKeyLength;
	in[apduSize++] = hashLength;
	memcpy(in + apduSize, hash, hashLength);
	apduSize += hashLength;
	in[OFFSET_CDATA] = (apduSize - 5);
	result = sendApduDongle(dongle, in, apduSize, out, sizeof(out), &sw);
	displayBinary(out, sizeof(out));

	if (result < 0) {
		ERL_WRITE_ERROR("io_error");
		return;
	}
	if (sw != SW_OK) {
		ERL_WRITE_ERROR("application_error");
		return;
	}

	byte buf[1000];
	char path[1000];
	formatBinary(path, out, sizeof(out));

	ETERM *resp = erl_mk_binary(path, result*2);
	erl_encode(resp, buf);
	write_cmd(buf, erl_term_len(resp));
	erl_free_term(resp);

	free(key);
	free(sighash);
}
