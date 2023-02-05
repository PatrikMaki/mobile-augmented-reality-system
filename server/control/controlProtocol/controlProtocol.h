/*
 * This is the MARS services ControlProtocol include file.
 */
#pragma once

/**
 * Construct and run the control protocol loop.
 *
 * @param callback is the callback function pointer for handling 
 *        the control messages which contains the null terminated 
 *        request and response messages.
 * @return non-0 value if the loop failed
 */
int ControlProtocol(int (*callback)(const char* request, char* response));
