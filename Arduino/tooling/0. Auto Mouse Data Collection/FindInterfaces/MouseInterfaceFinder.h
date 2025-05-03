#ifndef MOUSE_INTERFACE_FINDER_H
#define MOUSE_INTERFACE_FINDER_H

#include <usbhub.h>

// Maximum number of mouse interfaces to store
#define MAX_MOUSE_INTERFACES 10

// Struct to store mouse interface information
struct MouseInterfaceInfo {
    uint8_t interfaceNumber;
    uint8_t protocol;
    uint8_t endpointAddress;
};

// Global variables
extern USB Usb;
extern MouseInterfaceInfo mouseInterfaces[MAX_MOUSE_INTERFACES];
extern uint8_t mouseInterfaceCount;
extern uint8_t currentInterface;

// Function declarations
void PrintDescriptors(uint8_t addr);
uint8_t getdevdescr(uint8_t addr, uint8_t &num_conf);
uint8_t getconfdescr(uint8_t addr, uint8_t conf);
void PrintMouseInterfaces();

#endif
