#include "../commands.h"

void hsm_pin(dongleHandle dongle, ETERM* args){
	unsigned char in[260];
	unsigned char out[260];
	int result;
	int sw;
	int apduSize;
	ETERM *pin;
	int pinLength;
	ETERM *reply;
	int replyBytes;

	pin = erl_element(2, args);

	pinLength = ERL_BIN_SIZE(pin);
	if (pinLength < 0) {
		ERL_WRITE_ERROR("badarg");
		return;
	}

	apduSize = 0;
	in[apduSize++] = BTCHIP_CLA;
	in[apduSize++] = BTCHIP_INS_VERIFY_PIN;
	in[apduSize++] = 0x00;
	in[apduSize++] = 0x00;
	in[apduSize++] = pinLength;
	memcpy(in + apduSize, ERL_BIN_PTR(pin), pinLength);
	apduSize += pinLength;
	result = sendApduDongle(dongle, in, apduSize, out, sizeof(out), &sw);

	if (result < 0) {
		ERL_WRITE_ERROR("ioerror");
		return;
	}
	if (sw != SW_OK) {
		ERL_WRITE_ERROR("dongle_error");
		return;
	}

	reply = erl_format("verified");
	replyBytes = erl_term_len(reply);
	unsigned char replyBuffer[replyBytes];
	erl_encode(reply, replyBuffer);
	write_cmd(replyBuffer, replyBytes);
	erl_free_compound(reply);
	return;
}
