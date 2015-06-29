#include "../commands.h"

void hsm_pubkey(dongleHandle dongle, ETERM* args){
	unsigned char in[260];
	unsigned char out[260];
	ETERM *encodedKeyp;
	int encodedKeyLength;
	int result;
	int sw;
	int apduSize;
	encodedKeyp = erl_element(2, args);

	encodedKeyLength = ERL_BIN_SIZE(encodedKeyp);

	if (encodedKeyLength < 0) {
		ERL_WRITE_ERROR("badarg");
		return;
	}

	if (dongle == NULL) {
		ERL_WRITE_ERROR("not_found");
		return;
	}

	apduSize = 0;
	in[apduSize++] = BTCHIP_CLA;
	in[apduSize++] = BTCHIP_INS_GET_PUBLIC_KEY;
	in[apduSize++] = 0x00;
	in[apduSize++] = 0x00;
	in[apduSize++] = 0x00;
	in[apduSize++] = encodedKeyLength;
	memcpy(in + apduSize, (unsigned char *)ERL_BIN_PTR(encodedKeyp), encodedKeyLength);

	apduSize += encodedKeyLength;
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
	apduSize = 0;

	if (out[0] == 0x01) {
		binreply = erl_mk_binary((char*)(out + 2), out[1]);
		reply = erl_format("{ok, [{public_key, ~w}]}", binreply);
		reply_bytes = erl_term_len(reply);
		unsigned char reply_buffer[reply_bytes];
		erl_encode(reply, reply_buffer);
		write_cmd(reply_buffer, reply_bytes);
		erl_free_compound(reply);
		return;
	}
	else
	if (out[0] == 0x02) {
        	ETERM *public_key;
        	ETERM *chain_code;
        	int depth;
        	ETERM *fingerprint;
        	ETERM *child_number;

		public_key = erl_mk_binary((char *)(out + 2), out[1]);
		apduSize = 2 + out[1];
		chain_code = erl_mk_binary((char *)(out + apduSize), 32);
		apduSize += 32;
		depth = out[apduSize++];
		fingerprint = erl_mk_binary((char *)(out + apduSize), 4);
		apduSize += 4;
		child_number = erl_mk_binary((char *)(out + apduSize), 4);
		apduSize += 4;

        	reply = erl_format("{ok, [{public_key, ~w}, {chain_code, ~w}, {depth, ~i}, {fingerprint, ~w}, {child_number, ~w}]}",
			public_key, chain_code, depth, fingerprint, child_number);

		reply_bytes = erl_term_len(reply);
		unsigned char reply_buffer[reply_bytes];
		erl_encode(reply, reply_buffer);
		write_cmd(reply_buffer, reply_bytes);
		erl_free_compound(reply);

		return;
	}
}
