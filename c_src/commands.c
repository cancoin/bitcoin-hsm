#include "commands.h"

void port_reply_error(ETERM* reply) {
	port_reply(erl_format("{error, ~w}", reply));
	return;
}

void port_reply_ok(ETERM* reply) {
	port_reply(erl_format("{error, ~w}", reply));
	return;
}

void port_reply(ETERM* reply) {
	int reply_bytes;
	reply_bytes = erl_term_len(reply);
	unsigned char reply_buffer[reply_bytes];
	erl_encode(reply, reply_buffer);
	write_cmd(reply_buffer, reply_bytes);
	erl_free_compound(reply);
	return;
}
