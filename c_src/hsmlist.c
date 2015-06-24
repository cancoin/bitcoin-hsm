#include <getopt.h>
#include "hexUtils.h"
#include "dongleComm.h"
#include "btchipApdu.h"
#include "dongleCommHid.h"
#include "hsmport.h"
#include "hsmlist.h"

typedef unsigned char byte;

int main(int argc, char **argv) {
	int err;
	struct libusb_context *ctx = NULL;

	erl_init(NULL, 0);
	fprintf(stderr, "initusb");
	err = libusb_init(&ctx);
	fprintf(stderr, "initedusb");
	if (err < 0) {
		ERL_WRITE_ERROR(libusb_error_name(err));
		return err;
	}

	libusb_device **usb_list;
	ssize_t usb_list_size;
	int devices = 0;
	int i;
	uint8_t port_number;
	uint8_t bus;
	int reply_bytes;
	ETERM *locations;
	ETERM *location;

	usb_list_size = libusb_get_device_list(ctx, &usb_list);
	if(usb_list_size <= 0) {
		ERL_WRITE_ERROR("usb_error");
		return 1;
	}

	locations = erl_mk_empty_list();

	for (i = 0; i < usb_list_size; i++) {
		struct libusb_device_descriptor desc; err = libusb_get_device_descriptor(usb_list[i], &desc);
		if (err < 0) {
			ERL_WRITE_ERROR(libusb_error_name(err)); libusb_free_device_list(usb_list, 1);
			libusb_exit(ctx);
			return err;
		};
		port_number = libusb_get_port_number(usb_list[i]);
		bus = libusb_get_bus_number(usb_list[i]);
		if (desc.idVendor == BTCHIP_VID && desc.idProduct == BTCHIP_HID_PID) {
			devices++;
			location = erl_format("[{port, ~i}, {bus, ~i}]", port_number, bus);
			locations = erl_cons(location, locations);
		}
	}

	reply_bytes = erl_term_len(locations);
	byte reply_buffer[reply_bytes];
	erl_encode(locations, reply_buffer);
	write_cmd(reply_buffer, reply_bytes);
	erl_free_compound(locations);
	libusb_free_device_list(usb_list, 1);
	libusb_exit(ctx);
	return 0;
}
