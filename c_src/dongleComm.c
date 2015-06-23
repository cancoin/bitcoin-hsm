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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dongleComm.h"
#include "dongleCommHid.h"
#ifdef DEBUG_COMM
#include "hexUtils.h"
#endif

typedef enum {
	TRANSPORT_NONE,
	TRANSPORT_HID,
	TRANSPORT_WINUSB
} dongleTransport;

typedef struct dongleHandleInternal {
	dongleTransport transport;
	void* handle;
} dongleHandleInternal;

int initDongle() {
	int result = initHid();
		return result;
}

int exitDongle(void) {
	int result = exitHid();
		return result;
}

int sendApduDongle(dongleHandle handle, const unsigned char *apdu, size_t apduLength, unsigned char *out, size_t outLength, int *sw) 
{
	int result;
  result = sendApduHid((libusb_device_handle*)handle->handle, apdu, apduLength, out, outLength, sw);
	return result;
}

dongleHandle getDongle(libusb_context *ctx, int port, int bus) {
	dongleHandle result = (dongleHandle)malloc(sizeof(dongleHandleInternal));
	if (result == NULL) {
		return result;
	}
	result->transport = TRANSPORT_HID;
	result->handle = getDongleHid(ctx, port, bus);
	if (result->handle != NULL) {
		return result;
	}
	free(result);
	return NULL;
}


void closeDongle(dongleHandle handle) {
	closeDongleHid((libusb_device_handle*)handle->handle);
	handle->transport = TRANSPORT_NONE;
	free(handle);
}

