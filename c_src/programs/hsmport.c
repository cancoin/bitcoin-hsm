#include <libusb.h>
#include "../commands.h"
#include "hsmport.h"

int main(int argc, char **argv) {
	dongleHandle dongle;
        int option;
        int port = NULL;
        int bus = NULL;
	ETERM *tuplep;
	ETERM *functionp;
	unsigned char buf[1024];
	const char* func_name;

        while ((option = getopt(argc, argv, "p:b:")) != -1) {
                switch (option) {
                        case 'p':
                                port = atoi(optarg);
                                break;
                        case 'b':
                                bus = atoi(optarg);
                                break;
                        default:
                                return 1;
                }
        }

	erl_init(NULL, 0);

	initDongle();
	dongle = getDongle(port, bus);
	if (dongle == NULL) {
		ERL_WRITE_ERROR("not_found");
		exitDongle();
		return 1;
	}

	while (read_cmd(buf) > 0) {
		tuplep = erl_decode(buf);
		functionp = erl_element(1, tuplep);

		func_name = (const char*)ERL_ATOM_PTR(functionp);
		if (strncmp(func_name, "derive", 6) == 0){
			hsm_derive(dongle, tuplep);
		} else if (strncmp(func_name, "import", 6) == 0){
			hsm_import(dongle, tuplep);
		} else if (strncmp(func_name, "pin", 3) == 0){
			hsm_pin(dongle, tuplep);
		} else if (strncmp(func_name, "pubkey", 6) == 0){
			hsm_pubkey(dongle, tuplep);
	 	} else if (strncmp(func_name, "random", 6) == 0){
			hsm_random(dongle, tuplep);
	 	} else if (strncmp(func_name, "sign", 4) == 0){
			hsm_sign(dongle, tuplep);
	 	} else if (strncmp(func_name, "verify", 6) == 0){
			hsm_verify(dongle, tuplep);
	 	} else if (strncmp(func_name, "close", 5) == 0){
			break;
	 	} else {
			ERL_WRITE_ERROR("undef")
		}
		erl_free_compound(tuplep);
		erl_free_term(functionp);
	}

	closeDongle(dongle);
	exitDongle();

	ETERM *atom;
	atom = erl_mk_atom("closed");
	unsigned char closed_buf[erl_term_len(atom)];
	erl_encode(atom, closed_buf);
	write_cmd(closed_buf, erl_term_len(atom));
	erl_free_term(atom);
	fprintf(stderr, "CLOSED");

	return 0;
}


