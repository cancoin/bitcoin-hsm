CC:=gcc
LIBUSB_CFLAGS = $(shell pkg-config --cflags libusb-1.0)
LIBUSB_LIBS = $(shell pkg-config --libs libusb-1.0)
USB = usb-1.0
EI:= $(shell erl -noshell -eval 'io:format("~s", [code:lib_dir(erl_interface)])' -s erlang halt)

#DEBUG = -DEXTRA_DEBUG -DDEBUG_COMM
WARN ?= -Wall -Werror
CFLAGS = $(WARN) -Os -DHAVE_LIBUSB $(DEBUG) $(LIBUSB_CFLAGS) -I$(EI)/include
LIBS = -L$(EI)/lib $(LIBUSB_LIBS)
COMMANDS = commands/import.o commands/derive.o commands/pubkey.o commands/pin.o commands/random.o commands/sign.o commands/verify.o
OBJS   = hexUtils.o btchipUtils.o $(COMMANDS)

OBJS_COMM = $(OBJS) dongleCommHid.o dongleComm.o ledgerLayer.o erl_comm.o commands.o

all: hsmport hsmlist hsmpin

%.o: %.c %.h
	$(CC) $(CFLAGS) -o $@ -c $<

commands/%.o: %.c %.h
	$(CC) $(CFLAGS) -o $@ -c $<

programs/%.o: %.c %.h
	$(CC) $(CFLAGS) -o $@ -c $<

hsmport: programs/hsmport.o $(OBJS_COMM)
	gcc programs/hsmport.o $(LIBS) $(OBJS_COMM)  -o ../priv/hsmport -l$(USB) -lerl_interface -lei -pthread

hsmlist: programs/hsmlist.o $(OBJS_COMM)
	gcc programs/hsmlist.o $(LIBS) $(OBJS_COMM) -g -o ../priv/hsmlist -l$(USB) -lerl_interface -lei -pthread

hsmpin: programs/hsmpin.o $(OBJS_COMM)
	gcc programs/hsmpin.o $(LIBS) $(OBJS_COMM) -g -o ../priv/hsmpin -l$(USB) -lerl_interface -lei -pthread

clean:
	rm -f *.o commands/*.o programs/*.o ../priv/hsmport ../priv/hsmlist ../priv/hsmpin

