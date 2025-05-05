#ifndef DEVICE_ENUMERATOR_H
#define DEVICE_ENUMERATOR_H

// --- Standard Includes ---
#include <Arduino.h>
#include <USBHost_t36.h> // Core USB Host library
#include <math.h>        // Include for pow()
#include <stddef.h>      // Include for offsetof()
#include <string.h>      // For memset/memcpy
#include <new>           // For nothrow new

// --- Standard USB Definitions ---
#define USB_REQUEST_GET_STATUS        0
#define USB_REQUEST_CLEAR_FEATURE     1
#define USB_REQUEST_SET_FEATURE       3
#define USB_REQUEST_SET_ADDRESS       5
#define USB_REQUEST_GET_DESCRIPTOR    6
#define USB_REQUEST_SET_DESCRIPTOR    7
#define USB_REQUEST_GET_CONFIGURATION 8
#define USB_REQUEST_SET_CONFIGURATION 9
#define USB_REQUEST_GET_INTERFACE     10
#define USB_REQUEST_SET_INTERFACE     11
#define USB_REQUEST_SYNCH_FRAME       12
#define USB_DESCRIPTOR_DEVICE           1
#define USB_DESCRIPTOR_CONFIGURATION    2
#define USB_DESCRIPTOR_STRING           3
#define USB_DESCRIPTOR_INTERFACE        4
#define USB_DESCRIPTOR_ENDPOINT         5
#define USB_DESCRIPTOR_DEVICE_QUALIFIER 6
#define USB_DESCRIPTOR_OTHER_SPEED_CONFIGURATION 7
#define USB_DESCRIPTOR_INTERFACE_POWER  8
#define USB_DESCRIPTOR_OTG              9
#define USB_DESCRIPTOR_DEBUG            10
#define USB_DESCRIPTOR_INTERFACE_ASSOCIATION 11
#define USB_DESC_TYPE_HUB               0x29
#define USB_DESC_TYPE_SS_HUB            0x2A
#define USB_DESC_TYPE_ENDPOINT_COMPANION 0x30
#define USB_DESC_TYPE_HID               0x21
#define USB_DESC_TYPE_REPORT            0x22
#define USB_DESC_TYPE_PHYSICAL          0x23
#define CS_UNDEFINED            0x20
#define CS_INTERFACE            0x24
#define CS_ENDPOINT             0x25
#define USB_SPEED_FULL  0
#define USB_SPEED_LOW   1
#define USB_SPEED_HIGH  2

// --- Standard Descriptor Structures ---
// (Keep these struct definitions here as they are used by the data storage structs)
typedef struct {
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint16_t bcdUSB;
    uint8_t  bDeviceClass;
    uint8_t  bDeviceSubClass;
    uint8_t  bDeviceProtocol;
    uint8_t  bMaxPacketSize0;
    uint16_t idVendor;
    uint16_t idProduct;
    uint16_t bcdDevice;
    uint8_t  iManufacturer;
    uint8_t  iProduct;
    uint8_t  iSerialNumber;
    uint8_t  bNumConfigurations;
} __attribute__((packed)) usb_device_descriptor_t;

typedef struct {
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint16_t wTotalLength;
    uint8_t  bNumInterfaces;
    uint8_t  bConfigurationValue;
    uint8_t  iConfiguration;
    uint8_t  bmAttributes;
    uint8_t  bMaxPower;
} __attribute__((packed)) usb_configuration_descriptor_t;

typedef struct {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bInterfaceNumber;
    uint8_t bAlternateSetting;
    uint8_t bNumEndpoints;
    uint8_t bInterfaceClass;
    uint8_t bInterfaceSubClass;
    uint8_t bInterfaceProtocol;
    uint8_t iInterface;
} __attribute__((packed)) usb_interface_descriptor_t;

typedef struct {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bEndpointAddress;
    uint8_t bmAttributes;
    uint16_t wMaxPacketSize;
    uint8_t bInterval;
} __attribute__((packed)) usb_endpoint_descriptor_t;

typedef struct {
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bFirstInterface;
    uint8_t  bInterfaceCount;
    uint8_t  bFunctionClass;
    uint8_t  bFunctionSubClass;
    uint8_t  bFunctionProtocol;
    uint8_t  iFunction;
} __attribute__((packed)) usb_interface_assoc_descriptor_t;

typedef struct {
    uint8_t   bLength;
    uint8_t   bDescriptorType;
    uint16_t  bcdHID;
    uint8_t   bCountryCode;
    uint8_t   bNumDescriptors;
    struct {
        uint8_t type;
        uint8_t length_lo;
        uint8_t length_hi;
    } __attribute__((packed)) optional_descriptors[1];
} __attribute__((packed)) usb_hid_descriptor_t;

typedef struct {
    uint8_t bLength;
    uint8_t bDescriptorType;
} __attribute__((packed)) usb_descriptor_t;


// --- USB Data Storage Structures ---
#define MAX_CONFIGURATIONS_PER_DEVICE 1
#define MAX_INTERFACES_PER_CONFIG 8
#define MAX_ENDPOINTS_PER_INTERFACE 6
#define MAX_HID_REPORT_DESC_PER_INTERFACE 2
#define MAX_STRING_LENGTH 128

struct UsbEndpointData {
    uint8_t  bEndpointAddress = 0;
    uint8_t  bmAttributes = 0;
    uint16_t wMaxPacketSize = 0;
    uint8_t  bInterval = 0;
    uint8_t  bSynchAddress = 0;
};

struct UsbHidReportDescInfo {
    uint8_t   bDescriptorType = 0;
    uint16_t  wDescriptorLength = 0;
    uint8_t*  rawData = nullptr;
};

struct UsbInterfaceData {
    uint8_t bInterfaceNumber = 0xFF;
    uint8_t bAlternateSetting = 0;
    uint8_t bNumEndpoints = 0;
    uint8_t bInterfaceClass = 0;
    uint8_t bInterfaceSubClass = 0;
    uint8_t bInterfaceProtocol = 0;
    uint8_t iInterface = 0;
    char    interfaceString[MAX_STRING_LENGTH] = {0};
    UsbEndpointData endpoints[MAX_ENDPOINTS_PER_INTERFACE] = {};
    uint8_t         endpointCount = 0;
    bool     isHidInterface = false;
    uint16_t bcdHID = 0;
    uint8_t  bCountryCode = 0;
    uint8_t  bNumHidClassDescriptors = 0;
    UsbHidReportDescInfo reportDescriptors[MAX_HID_REPORT_DESC_PER_INTERFACE] = {};
    uint8_t              reportDescriptorCount = 0;
};

struct UsbConfigurationData {
    uint8_t  bConfigurationValue = 0;
    uint8_t  iConfiguration = 0;
    char     configurationString[MAX_STRING_LENGTH] = {0};
    uint8_t  bmAttributes = 0;
    uint8_t  bMaxPower = 0;
    uint16_t wTotalLength = 0;
    uint8_t* rawConfigData = nullptr;
    uint16_t rawConfigLen = 0;
    UsbInterfaceData interfaces[MAX_INTERFACES_PER_CONFIG] = {};
    uint8_t          interfaceCount = 0;
};

struct UsbDeviceData {
    uint16_t idVendor = 0;
    uint16_t idProduct = 0;
    uint8_t  speed = 0;
    uint8_t  hub_address = 0;
    uint8_t  hub_port = 0;
    uint8_t  address = 0;
    uint16_t bcdUSB = 0;
    uint8_t  bDeviceClass = 0;
    uint8_t  bDeviceSubClass = 0;
    uint8_t  bDeviceProtocol = 0;
    uint8_t  bMaxPacketSize0 = 0;
    uint16_t bcdDevice = 0;
    uint8_t  iManufacturer = 0;
    uint8_t  iProduct = 0;
    uint8_t  iSerialNumber = 0;
    uint8_t  bNumConfigurations = 0;
    char manufacturerString[MAX_STRING_LENGTH] = {0};
    char productString[MAX_STRING_LENGTH] = {0};
    char serialNumberString[MAX_STRING_LENGTH] = {0};
    UsbConfigurationData configurations[MAX_CONFIGURATIONS_PER_DEVICE] = {};
    uint8_t              configurationCount = 0;
};

// --- Configuration ---
// Moved to .ino or .cpp where needed, but define buffer size here
#define TEMP_BUFFER_SIZE 1024
// Make DEBUG_SERIAL extern if needed in .cpp, but better to pass it to print function
// extern HardwareSerial& DEBUG_SERIAL; // Example if needed, but avoid globals if possible

// --- Class Definition ---
class DeviceEnumerator : public USBDriver {
private:
    // --- Enumeration State Enum ---
    enum EnumerationState {
        STATE_IDLE, STATE_GETTING_DEVICE_DESC_8, STATE_GETTING_DEVICE_DESC_FULL,
        STATE_GETTING_STRING_MAN, STATE_GETTING_STRING_PROD, STATE_GETTING_STRING_SERIAL,
        STATE_GETTING_CONFIG_HEADER, STATE_GETTING_CONFIG_FULL,
        STATE_PARSING_CONFIG,
        STATE_GETTING_CONFIG_STRING,
        STATE_GETTING_INTERFACE_STRINGS,
        STATE_GETTING_REPORT_DESC,
        STATE_DONE, STATE_ERROR
    };

public:
    // --- Constructor ---
    DeviceEnumerator(USBHost& host);

    // --- USBDriver Virtual Methods ---
    virtual bool claim(Device_t* device, int type, const uint8_t* descriptors, uint32_t len) override;
    virtual void disconnect() override;
    virtual void control(const Transfer_t* transfer) override;
    virtual void Task() override;

    // --- Public Accessor Methods ---
    bool isEnumerationDone() const;
    bool isErrorState() const;
    const UsbDeviceData* getStoredDeviceData() const;
    Device_t* getCurrentDevice() const; // Helper to check device ptr

    // --- Public Action Methods ---
    void printStoredData(Print& printer) const; // Takes Print object (like Serial)

protected:
    // --- Internal Initialization ---
    void init();

    // --- Internal State Management & Parsing ---
    void startEnumeration(Device_t* device);
    void parseFullConfigurationBlock();
    void processStateMachine(const Transfer_t* transfer = nullptr); // Now private
    void queueGetDescriptor(uint8_t desc_type, uint8_t desc_index, uint16_t lang_id, uint16_t len);
    void queueGetHidReportDescriptor(uint8_t config_idx, uint8_t interface_idx, uint8_t report_desc_info_idx);
    void storeString(char* buffer, size_t buffer_len, const Transfer_t* transfer);
    void clearStoredDeviceData(); // Keep protected or make private


private:
    // --- Member Variables ---
    Device_t*         current_device_ = nullptr;
    EnumerationState  enum_state_ = STATE_IDLE;
    UsbDeviceData     stored_data_;

    // State variables for multi-step operations
    uint16_t          config_total_len_ = 0;
    uint8_t           current_config_index_ = 0;
    uint8_t           current_interface_index_ = 0;
    uint8_t           current_hid_report_index_ = 0;

    // Transfer buffer (needs alignment for DMA)
    alignas(4) uint8_t temp_buffer_[TEMP_BUFFER_SIZE];
    alignas(4) setup_t setup_packet_; // Setup packet for control transfers
};


// --- Helper Function Declarations (Implement in .cpp) ---
const char* descriptor_type_to_string(uint8_t type);
void print_endpoint_attributes(uint8_t attr); // Consider passing Print& object
void print_endpoint_interval(uint8_t interval, uint8_t speed, uint8_t attributes); // Consider passing Print& object


#endif // DEVICE_ENUMERATOR_H