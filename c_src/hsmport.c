#include "hsmport.h"

void derive(dongleHandle dongle, ETERM* args){
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
  erl_free_term(resp); // free resp
}

void sign(dongleHandle dongle, ETERM* args){
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


void pubkey(dongleHandle dongle, ETERM* args){
	if (dongle == NULL) {
    ERL_WRITE_ERROR("not_found");
		return;
	}
  return;
}

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
			derive(dongle, args);
	 	} else if (strncmp(func_name, "sign", 4) == 0){
			sign(dongle, args);
	 	} else if (strncmp(func_name, "pubkey", 6) == 0){
			pubkey(dongle, args);
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


