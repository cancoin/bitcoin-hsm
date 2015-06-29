/*
 * Copyright (c) 2014 UBINITY SAS All rights reserved.
 * This source code is the property of UBINITY SAS. 
 * Redistribution and use in source (source code) or binary (object code)
 * forms with or without modification, for commercial, educational or
 * research purposes is not permitted without the prior written consent
 * of UBINITY SAS. 
 */
#include "hsmpin.h"

int main(int argc, char **argv) {
	libusb_device **usb_list;
	ssize_t usb_list_size;
	struct libusb_device_descriptor desc;
	uint8_t port;
	uint8_t bus;
	int devices = 0;
	int i;
	int err;
	unsigned char pin[8];
	unsigned char in[260];
	unsigned char out[260];
	int result;
	int sw;
	int apduSize;

	if (argc < 2) {
		fprintf(stderr, "Usage : %s [hex PIN]\n", argv[0]);
		return 0;
	}
	result = hexToBin(argv[1], pin, sizeof(pin));
	if (result == 0) {
		fprintf(stderr, "Invalid PIN\n");
		return 0;
	}

	libusb_init(NULL);
	usb_list_size = libusb_get_device_list(NULL, &usb_list);
	if (usb_list_size <= 0) {
		fprintf(stderr, "Failed to list usb devices\n");
		return 0;
	}

	for (i = 0; i < usb_list_size; i++) {
		err = libusb_get_device_descriptor(usb_list[i], &desc);
		if (err < 0) {
			fprintf(stderr, "Error getting device descriptor: %s\n", libusb_error_name(err));
			continue;
		};

		if (desc.idVendor == BTCHIP_VID &&
			(desc.idProduct == BTCHIP_HID_PID_LEDGER  ||
			 desc.idProduct == BTCHIP_HID_PID_LEDGER_PROTON ||
			 desc.idProduct == BTCHIP_HID_BOOTLOADER_PID)) {

			dongleHandle dongle;
			devices++;

			port = libusb_get_port_number(usb_list[i]);
			bus = libusb_get_bus_number(usb_list[i]);

			fprintf(stdout, "opening dongle at port: %i bus: %i\n", port, bus);

			dongle = getDongle(port, bus);

			if (dongle == NULL) {
			  fprintf(stderr, "Failed to open dongle at port %i bus %i", port, bus);
			  continue;
			}

			apduSize = 0;
			in[apduSize++] = BTCHIP_CLA;
			in[apduSize++] = BTCHIP_INS_VERIFY_PIN;
			in[apduSize++] = 0x00;
			in[apduSize++] = 0x00;
			in[apduSize++] = result;
			memcpy(in + apduSize, pin, result);
			apduSize += result;
			result = sendApduDongle(dongle, in, apduSize, out, sizeof(out), &sw);

			dongle = NULL;

			if (result < 0) {
				fprintf(stderr, "I/O error\n");
				closeDongle(dongle);
				continue;
			}
			if (sw != SW_OK) {
				fprintf(stderr, "Dongle application error : %.4x\n", sw);
				closeDongle(dongle);
				continue;
			}

			closeDongle(dongle);
			fprintf(stdout, "PIN verified dongle %i\n", devices);
		}

	}

	libusb_free_device_list(usb_list, 1);
	libusb_exit(NULL);

	if (devices == 0) {
	  fprintf(stdout, "No dongles found\n");
	  return 1;
	}

	return 0;
}
