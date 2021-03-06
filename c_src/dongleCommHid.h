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

#ifndef __DONGLECOMM_HID_H__

#define __DONGLECOMM_HID_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libusb.h>

#define BTCHIP_VID 0x2581
#define BTCHIP_HID_PID 0x2b7c
#define BTCHIP_HID_PID_LEDGER 0x3b7c
#define BTCHIP_HID_PID_LEDGER_PROTON 0x4b7c
#define BTCHIP_HID_BOOTLOADER_PID 0x1807

int initHid(libusb_context *ctx);
int exitHid(libusb_context *ctx);
int sendApduHid(libusb_device_handle *handle, const unsigned char ledger, const unsigned char *apdu, size_t apduLength, unsigned char *out, size_t outLength, int *sw);
libusb_device_handle* getDongleHid(libusb_context *ctx, unsigned char *ledger, int port, int bus);
void closeDongleHid(libusb_device_handle *handle);

#endif

