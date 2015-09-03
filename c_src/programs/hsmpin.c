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
	uint8_t address;
	uint8_t bus;
	int devices = 0;
	int i;
	int err;
	int pin_size;
	unsigned char pin[8];

	if (argc < 2) {
		fprintf(stderr, "Usage : %s [hex PIN]\n", argv[0]);
		return 0;
	}
	pin_size = hexToBin(argv[1], pin, sizeof(pin));
	if (pin_size == 0) {
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

                        unsigned char in[260];
                        unsigned char out[260];
                        int sw;
                        int apduSize;
                        int result;
			dongleHandle dongle;
			devices++;

			address = libusb_get_device_address(usb_list[i]);
			bus = libusb_get_bus_number(usb_list[i]);

                        dongle = getDongle(NULL, address, bus);

			if (dongle == NULL) {
			  fprintf(stderr, "Failed to open dongle at address %i bus %i", address, bus);
			  continue;
			}

			apduSize = 0;
			in[apduSize++] = BTCHIP_CLA;
			in[apduSize++] = BTCHIP_INS_VERIFY_PIN;
			in[apduSize++] = 0x00;
			in[apduSize++] = 0x00;
			in[apduSize++] = pin_size;
			memcpy(in + apduSize, pin, pin_size);
			apduSize += pin_size;

			result = sendApduDongle(dongle, in, apduSize, out, sizeof(out), &sw);

			dongle = NULL;

			if (result < 0) {
				fprintf(stderr, "I/O error\n");
				closeDongle(dongle);
				exit(1);
			}
			if (sw != SW_OK) {
				fprintf(stderr, "Dongle application error : %.4x\n", sw);
				closeDongle(dongle);
				exit(1);
			}

			fprintf(stdout, "PIN verified (address %i bus %i)\n", address, bus);
                	closeDongle(dongle);
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
