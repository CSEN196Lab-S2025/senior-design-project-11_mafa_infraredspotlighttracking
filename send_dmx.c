void send_dmx(unsigned char *dmx_data) {
    FT_HANDLE dmx_handle;
    DWORD bytes_written;
    
    // Open the Enttec USB-DMX interface
    if (FT_Open(0, &dmx_handle) != FT_OK) {
        printf("Error: Could not open DMX device.\n");
        return;
    }

    // Set the USB parameters
    FT_SetBaudRate(dmx_handle, 250000);
    FT_SetDataCharacteristics(dmx_handle, FT_BITS_8, FT_STOP_BITS_2, FT_PARITY_NONE);
    FT_SetFlowControl(dmx_handle, FT_FLOW_NONE, 0, 0);

    // DMX requires a break signal before sending data
    FT_SetBreakOn(dmx_handle);
    usleep(100);  // Short break (100 microseconds)
    FT_SetBreakOff(dmx_handle);

    // Send DMX header + data
    unsigned char dmx_packet[DMX_CHANNELS + 1];
    dmx_packet[0] = 0;  // Start Code
    memcpy(&dmx_packet[1], dmx_data, DMX_CHANNELS);

    if (FT_Write(dmx_handle, dmx_packet, DMX_CHANNELS + 1, &bytes_written) != FT_OK) {
        printf("Error: Could not send DMX data.\n");
    }

    FT_Close(dmx_handle);
}
