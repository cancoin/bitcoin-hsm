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

#ifndef __DONGLECOMM_H__

#define __DONGLECOMM_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libusb.h>

struct dongleHandleInternal;
typedef struct dongleHandleInternal* dongleHandle;

int initDongle(libusb_context *ctx);
int exitDongle(libusb_context *ctx);
int sendApduDongle(dongleHandle handle, const unsigned char *apdu, size_t apduLength, unsigned char *out, size_t outLength, int *sw);
dongleHandle getDongle(libusb_context *ctx, int port, int bus);
void closeDongle(dongleHandle handle);

#endif

