#include "hsmport.h"
#include "commands.h"

int main(int argc, char **argv) {
	dongleHandle dongle;
	int option;
	int port = NULL;
	int bus = NULL;

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

	struct libusb_context *ctx;
	libusb_init(&ctx);

	dongle = getDongle(ctx, port, bus);
	if (dongle == NULL) {
	ERL_WRITE_ERROR("not_found");
		libusb_exit(ctx);
		return 1;
	}


	ETERM *tuplep;
	ETERM *fnp;
	ETERM *args;
	byte buf[1024];
	const char* func_name;

	while (read_cmd(buf) > 0) {
		tuplep = erl_decode(buf);
		fnp = erl_element(1, tuplep);

		func_name =  (const char*)ERL_ATOM_PTR(fnp);
		args = erl_element(2, tuplep);
		if (strncmp(func_name, "derive", 6) == 0){
			hsm_derive(dongle, args);
		} else if (strncmp(func_name, "import", 6) == 0){
			hsm_import(dongle, args);
		} else if (strncmp(func_name, "pin", 3) == 0){
			hsm_pin(dongle, args);
		} else if (strncmp(func_name, "pubkey", 6) == 0){
			hsm_pubkey(dongle, args);
	 	} else if (strncmp(func_name, "random", 6) == 0){
			hsm_random(dongle, args);
	 	} else if (strncmp(func_name, "sign", 4) == 0){
			hsm_sign(dongle, args);
	 	} else if (strncmp(func_name, "verify", 6) == 0){
			hsm_pubkey(dongle, args);
	 	}	else {
		  ERL_WRITE_ERROR("undef")
    }
		erl_free_compound(tuplep);
		erl_free_term(fnp);
	}

	closeDongle(dongle);
	exitDongle();
	libusb_exit(ctx);

	return 0;
}


