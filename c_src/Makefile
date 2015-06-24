# This Makefile is automatically generated with "mix cure.bootstrap"!
# Please do not remove the following variables as they point to 
# the necessary source-files.
CC:=gcc
LIBUSB = /usr/include/libusb-1.0
USB = usb-1.0
EI:= $(shell erl -noshell -eval 'io:format("~s", [code:lib_dir(erl_interface)])' -s erlang halt)

DEBUG = -DEXTRA_DEBUG -DDEBUG_COMM
WARN ?= -Wall -Werror
CFLAGS = $(WARN) -Os -DHAVE_LIBUSB $(DEBUG) -I$(LIBUSB) -I$(EI)/include
LIBS = -L$(EI)/lib -L/usr/local/lib
COMMANDS = commands/import.o commands/derive.o commands/pubkey.o commands/pin.o commands/random.o commands/sign.o commands/verify.o
OBJS   = hexUtils.o btchipUtils.o ledgerLayer.o $(COMMANDS)
OBJS_COMM = $(OBJS) dongleCommHid.o dongleComm.o erl_comm.o

all: hsmport hsmlist

%.o: %.c %.h
	$(CC) $(CFLAGS) -o $@ -c $<

commands/%.o: %.c %.h
	$(CC) $(CFLAGS) -o $@ -c $<

hsmport: hsmport.o $(OBJS_COMM)
	gcc hsmport.o $(LIBS) $(OBJS_COMM) -o ../priv/hsmport -l$(USB) -lerl_interface -lei -pthread

hsmlist: hsmlist.o $(OBJS_COMM)
	gcc hsmlist.o $(LIBS) $(OBJS_COMM) -g -o ../priv/hsmlist -l$(USB) -lerl_interface -lei -pthread

clean:
	rm *.o ../priv/hsmport ../priv/hsmlist

