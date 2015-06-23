#ifndef ___AAAAA
#define ___AAAAA

#ifdef __WIN32__
#include <io.h>
#else
#include <unistd.h>
#include <sys/types.h>
#endif

#include "erl_interface.h"

typedef unsigned char byte;

int read_cmd(byte *buf);
int write_cmd(byte *buf, int len);
int read_exact(byte *buf, int len);

int write_exact(byte *buf, int len);

#endif
