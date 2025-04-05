#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ftd2xx.h"

#define DMX_CHANNELS 512
#define DMX_START_CODE 0
#define VL2202_PAN_CHANNEL 1  // Adjust according to fixture profile
#define VL2202_TILT_CHANNEL 2 // Adjust according to fixture profile

FT_HANDLE dmx_handle;

// Function to send DMX data
void send_dmx(unsigned char *dmx_data) {
    DWORD bytes_written;
    unsigned char packet[DMX_CHANNELS + 1];
    packet[0] = DMX_START_CODE;  // DMX start code
    memcpy(&packet[1], dmx_data, DMX_CHANNELS);

    FT_Write(dmx_handle, packet, DMX_CHANNELS + 1, &bytes_written);
}

// Function to initialize DMX communication
int init_dmx() {
    if (FT_Open(0, &dmx_handle) != FT_OK) {
        printf("Failed to open DMX interface.\n");
        return 0;
    }
    FT_ResetDevice(dmx_handle);
    FT_SetBaudRate(dmx_handle, 250000);
    FT_SetDataCharacteristics(dmx_handle, FT_BITS_8, FT_STOP_BITS_2, FT_PARITY_NONE);
    FT_SetFlowControl(dmx_handle, FT_FLOW_NONE, 0, 0);
    FT_ClrRts(dmx_handle);
    return 1;
}

// Function to set pan/tilt values for VL2202
void move_light(int pan, int tilt) {
    unsigned char dmx_data[DMX_CHANNELS] = {0};
    dmx_data[VL2202_PAN_CHANNEL - 1] = pan;
    dmx_data[VL2202_TILT_CHANNEL - 1] = tilt;
    send_dmx(dmx_data);
}

int main() {
    if (!init_dmx()) {
        return 1;
    }
    printf("DMX Initialized. Moving light...\n");

    move_light(128, 128); // Move to center position
    sleep(2);
    move_light(255, 0); // Move to a different position
    sleep(2);

    FT_Close(dmx_handle);
    return 0;
}
