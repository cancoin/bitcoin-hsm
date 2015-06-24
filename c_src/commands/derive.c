#include "../commands.h"

void hsm_derive(dongleHandle dongle, ETERM* args){
	if (dongle == NULL) {
		ERL_WRITE_ERROR("not_found");
		return;
	}

	ETERM *keyp;
	ETERM *indexp;

	keyp = erl_element(1, args);
	indexp = erl_element(2, args);

	unsigned char in[260];
	unsigned char out[260];
	unsigned char encodedKey[100];
	int encodedKeyLength;
	uint32_t index;
	int result;
	int sw;
	int apduSize;

	int key_len = ERL_BIN_SIZE(keyp);
	char *key;
	key = malloc((key_len + 1) * sizeof(char));
	memcpy(key, (char *) ERL_BIN_PTR(keyp), key_len);
	key[key_len + 1] = '\0';

	int derive_index_len = ERL_BIN_SIZE(indexp);
	char *derive_index;
	derive_index = malloc((derive_index_len + 1) * sizeof(char));
	memcpy(derive_index, (char *) ERL_BIN_PTR(indexp), derive_index_len);
	derive_index[derive_index_len + 1] = '\0';

	encodedKeyLength = hexToBin(key, encodedKey, sizeof(encodedKey));
	if (encodedKeyLength < 0) {
		ERL_WRITE_ERROR("invalid_key");
		return;
	}

	errno = 0;
	index = strtoll(derive_index, NULL, 16);
	if (errno != 0) {
		ERL_WRITE_ERROR("invalid_index");
		return;
	}

	apduSize = 0;
	in[apduSize++] = BTCHIP_CLA;
	in[apduSize++] = BTCHIP_INS_DERIVE_BIP32_KEY;
	in[apduSize++] = 0x00;
	in[apduSize++] = 0x00;
	in[apduSize++] = 0x00;
	in[apduSize++] = encodedKeyLength;
	memcpy(in + apduSize, encodedKey, encodedKeyLength);
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

	byte buf[1000];
	char path[sizeof(out)*2+1];
	formatBinary(path, out, sizeof(out));

	ETERM *resp = erl_mk_binary(path, result*2);

	erl_encode(resp, buf);
	write_cmd(buf, erl_term_len(resp));
	erl_free_term(resp);
}
