/*
*******************************************************************************    
*   BTChip Bitcoin Hardware Wallet C test interface
*   (c) 2014 BTChip - 1BTChip7VfTnrPra5jqci7ejnMguuHogTn
*   
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
*   Unless required by applicable law or agreed to in writing, software
*   distributed under the License is distributed on an "AS IS" BASIS,
*   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*   limitations under the License.
********************************************************************************/

#include "dongleCommHid.h"
#include "ledgerLayer.h"

#define TIMEOUT 10000
#define SW1_DATA 0x61
#define MAX_BLOCK 64

#ifdef HAVE_LIBUSB

int initHid(libusb_context *ctx) {
	return libusb_init(&ctx);
}

int exitHid(libusb_context *ctx) {
	libusb_exit(ctx);
	return 0;
}

int sendApduHid(libusb_device_handle *handle, const unsigned char ledger, const unsigned char *apdu, size_t apduLength, unsigned char *out, size_t outLength, int *sw) {
	unsigned char buffer[400];
	unsigned char paddingBuffer[MAX_BLOCK];
	int result;
	int length;
	int swOffset;
	int remaining = apduLength;
	int offset = 0;

	if (ledger) {
		result = wrapCommandAPDU(DEFAULT_LEDGER_CHANNEL, apdu, apduLength, LEDGER_HID_PACKET_SIZE, buffer, sizeof(buffer));
		if (result < 0) {
			return result;
		}
		remaining = result;
	}
	else {
		memcpy(buffer, apdu, apduLength);
		remaining = apduLength;
	}
	while (remaining > 0) {
		int blockSize = (remaining > MAX_BLOCK ? MAX_BLOCK : remaining);
		memset(paddingBuffer, 0, MAX_BLOCK);
		memcpy(paddingBuffer, buffer + offset, blockSize);
		result = libusb_interrupt_transfer(handle, 0x02, paddingBuffer, blockSize, &length, TIMEOUT);
		if (result < 0) {
			return result;
		}
		offset += blockSize;
		remaining -= blockSize;
	}
	result = libusb_interrupt_transfer(handle, 0x82, buffer, MAX_BLOCK, &length, TIMEOUT);
	if (result < 0) {
		return result;
	}
	offset = MAX_BLOCK;
	if (!ledger) {
		if (buffer[0] == SW1_DATA) {
			int dummy;
			length = buffer[1];
			length += 2;
			if (length > (MAX_BLOCK - 2)) {	
				remaining = length - (MAX_BLOCK - 2);
				while (remaining != 0) {
					int blockSize;
					if (remaining > MAX_BLOCK) {
						blockSize = MAX_BLOCK;
					}
					else {
						blockSize = remaining;
					}
					result = libusb_interrupt_transfer(handle, 0x82, buffer + offset, MAX_BLOCK, &dummy, TIMEOUT);
					if (result < 0) {
						return result;
					}
					offset += blockSize;
					remaining -= blockSize;
				}
			}
			length -= 2;
			memcpy(out, buffer + 2, length);
			swOffset = 2 + length;
		}
		else {
			length = 0;
			swOffset = 0;
		}
		if (sw != NULL) {
			*sw = (buffer[swOffset] << 8) | buffer[swOffset + 1];
		}
	}
	else {
		for (;;) {
			int dummy;
			result = unwrapReponseAPDU(DEFAULT_LEDGER_CHANNEL, buffer, offset, LEDGER_HID_PACKET_SIZE, out, outLength);			
			if (result < 0) {
				return result;
			}
			if (result != 0) {
				length = result - 2;
				swOffset = result - 2;
				break;
			}
			result = libusb_interrupt_transfer(handle, 0x82, buffer + offset, MAX_BLOCK, &dummy, TIMEOUT);
			if (result < 0) {
				return result;
			}
			offset += MAX_BLOCK;
		}
		if (sw != NULL) {
			*sw = (out[swOffset] << 8) | out[swOffset + 1];
		}
	}
	return length;
}

libusb_device_handle* getDongleHid(libusb_context *ctx, unsigned char *ledger, int address, int bus) {
	struct libusb_device **devs;
	struct libusb_device *found = NULL;
	struct libusb_device *dev = NULL;
	struct libusb_device_handle *handle = NULL;
	size_t i = 0;
	int r;
	int s;
	int devices = 0;
	int usb_address;
	int usb_bus;

	if (libusb_get_device_list(ctx, &devs) < 0)
		return NULL;

	while ((dev = devs[i++]) != NULL) {
		struct libusb_device_descriptor desc;
		r = libusb_get_device_descriptor(dev, &desc);
		if (r < 0) {
			goto out;
		}

		if (desc.idVendor == BTCHIP_VID) {
			if(desc.idProduct == BTCHIP_HID_PID || desc.idProduct == BTCHIP_HID_BOOTLOADER_PID) {
				*ledger = 0;
			}
			else
			if (desc.idProduct == BTCHIP_HID_PID_LEDGER  || desc.idProduct == BTCHIP_HID_PID_LEDGER_PROTON) {
				*ledger = 1;
			}
			else {
				continue;
			}

			devices++;
			usb_address = libusb_get_device_address(dev);
			usb_bus = libusb_get_bus_number(dev);
			if(address == usb_address && bus == usb_bus) {
				found = dev;
				break;
			}
		}
	}

	if (found) {
		s = libusb_open(found, &handle);
		if (s < 0) {
			handle = NULL;
			goto out;
		}
		if (handle) {
			libusb_detach_kernel_driver(handle, 0);
			libusb_claim_interface(handle, 0);
			libusb_free_device_list(devs, 1);
			return handle;
		}
	}

out:
	libusb_free_device_list(devs, 1);
	return handle;
}

void closeDongleHid(libusb_device_handle *handle) {	
	libusb_release_interface(handle, 0);
	libusb_attach_kernel_driver(handle, 0);
	libusb_close(handle);
}

#endif

