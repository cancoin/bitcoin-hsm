#ifndef HSMPORT_H
#define HSMPORT_H

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include "erl_interface.h"
#include "ei.h"
#include "../erl_comm.h"
#include "../btchipApdu.h"
#include "../dongleComm.h"
#include "../btchipUtils.h"
#include "../hexUtils.h"

#define ERL_WRITE_ERROR(message) \
  ETERM *return_arr[2]; \
  ETERM *return_tuple; \
  ETERM *error_atom; \
  ETERM *error_message; \
  error_atom = erl_mk_atom("error"); \
  error_message = erl_mk_atom(message); \
  return_arr[0] = error_atom; \
  return_arr[1] = error_message; \
  return_tuple = erl_mk_tuple(return_arr, 2); \
  unsigned char buf[erl_term_len(return_tuple)]; \
  erl_encode(return_tuple, buf); \
  write_cmd(buf, erl_term_len(return_tuple)); \
  erl_free_array(return_arr, 2); \
  erl_free_term(return_tuple); \

#endif
