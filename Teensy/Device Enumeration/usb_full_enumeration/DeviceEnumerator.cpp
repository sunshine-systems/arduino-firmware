#include "DeviceEnumerator.h"
#include "config.h" // Include configuration for logging flag and Serial port
#include <Print.h>  // Ensure Print class is known

// ==========================================================================
// Helper Function Implementations
// ==========================================================================

const char* descriptor_type_to_string(uint8_t type) {
    // This function just returns static strings, no logging needed here.
    switch (type) {
        case USB_DESCRIPTOR_DEVICE:           return "DEVICE";
        case USB_DESCRIPTOR_CONFIGURATION:    return "CONFIGURATION";
        case USB_DESCRIPTOR_STRING:           return "STRING";
        case USB_DESCRIPTOR_INTERFACE:        return "INTERFACE";
        case USB_DESCRIPTOR_ENDPOINT:         return "ENDPOINT";
        case USB_DESCRIPTOR_DEVICE_QUALIFIER: return "DEVICE_QUALIFIER";
        case USB_DESCRIPTOR_OTHER_SPEED_CONFIGURATION: return "OTHER_SPEED_CONFIG";
        case USB_DESCRIPTOR_INTERFACE_POWER:  return "INTERFACE_POWER";
        case USB_DESCRIPTOR_OTG:              return "OTG";
        case USB_DESCRIPTOR_DEBUG:            return "DEBUG";
        case USB_DESCRIPTOR_INTERFACE_ASSOCIATION: return "INTERFACE_ASSOCIATION";
        case USB_DESC_TYPE_HUB:               return "HUB";
        case USB_DESC_TYPE_SS_HUB:            return "SS_HUB";
        case USB_DESC_TYPE_ENDPOINT_COMPANION:return "SS_ENDPOINT_COMPANION";
        case USB_DESC_TYPE_HID:               return "HID";
        case USB_DESC_TYPE_REPORT:            return "REPORT";
        case USB_DESC_TYPE_PHYSICAL:          return "PHYSICAL";
        case CS_UNDEFINED:                    return "CS_UNDEFINED";
        case CS_INTERFACE:                    return "CS_INTERFACE";
        case CS_ENDPOINT:                     return "CS_ENDPOINT";
        default:                              return "UNKNOWN";
    }
}

// Modified print functions to use runtime checks
void print_endpoint_attributes(Print& printer, uint8_t attr) {
    if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
        uint8_t transfer_type = attr & 0x03;
        switch (transfer_type) {
            case 0:
                printer.print("Control");
                break;
            case 1:
                printer.print("Isochronous");
                break;
            case 2:
                printer.print("Bulk");
                break;
            case 3:
                printer.print("Interrupt");
                break;
        }
    }
}

void print_endpoint_interval(Print& printer, uint8_t interval, uint8_t speed, uint8_t attributes) {
    if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
        uint8_t transfer_type = attributes & 0x03;
        if (transfer_type != 1 && transfer_type != 3) {
            printer.printf("%u (N/A)", interval);
            return;
        }
        if (speed == USB_SPEED_HIGH) {
            if (interval == 0) interval = 1; // As per USB spec
            if (interval > 16) interval = 16; // Max interval for HS
            float mf = pow(2.0f, (float)interval - 1.0f);
            float pus = mf * 125.0f; // Period in microseconds
            if (pus < 1.0f) {
                printer.printf("%u (%.3f us / %.1f MHz)", interval, pus, 1.0f / (pus / 1000000.0f) / 1000000.0f );
            } else if (pus < 1000.0f) {
                printer.printf("%u (%.1f us / %.1f KHz)", interval, pus, 1000000.0f / pus / 1000.0f);
            } else {
                printer.printf("%u (%.1f ms / %.1f Hz)", interval, pus / 1000.0f, 1000000.0f / pus);
            }
        } else { // Full or Low speed
            if (transfer_type == 1) { // Isochronous
                 printer.printf("%u (1 ms frame fixed)", interval); // Interval is always 1ms for FS/LS Iso
            } else { // Interrupt
                if (interval == 0) interval = 1; // Min interval for FS/LS Interrupt
                printer.printf("%u (%u ms / %u Hz)", interval, interval, 1000 / interval);
            }
        }
    }
}


// ==========================================================================
// DeviceEnumerator Class Method Implementations
// ==========================================================================

DeviceEnumerator::DeviceEnumerator(USBHost& host) : USBDriver() {
    init();
}

void DeviceEnumerator::init() {
    // clearStoredDeviceData has its own internal logging control
    clearStoredDeviceData();
    // This internal USBHost function registers the driver; no logging needed.
    driver_ready_for_device(this);
}

bool DeviceEnumerator::claim(Device_t* device, int type, const uint8_t* descriptors, uint32_t len) {
    // Only claim root device connections (type 0)
    if (type != 0 || device == nullptr) {
        if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
            DEBUG_SERIAL_ENUM.printf("DeviceEnumerator::claim: Ignoring claim for type %d or null device.\n", type);
        }
        return false;
    }

    // Check if already busy
    if (enum_state_ != STATE_IDLE) {
        if (current_device_ == device) {
            if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                DEBUG_SERIAL_ENUM.println("DeviceEnumerator::claim: Already processing this device.");
            }
            return false; // Already handling it
        } else if (current_device_ != nullptr) {
            if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                DEBUG_SERIAL_ENUM.println("!!! DeviceEnumerator::claim: Busy with another device, ignoring new claim.");
            }
            return false; // Busy with different device
        }
    }

    if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
        DEBUG_SERIAL_ENUM.printf("DeviceEnumerator::claim: Claiming device VID:0x%04X PID:0x%04X (Type %d)\n", device->idVendor, device->idProduct, type);
    }
    startEnumeration(device);
    // Return false: We are just *analyzing* the device. We don't prevent
    // other functional drivers (like HID) from claiming it later.
    return false;
}

void DeviceEnumerator::disconnect() {
    uint16_t vid = 0;
    uint16_t pid = 0;

    if (current_device_) {
        vid = current_device_->idVendor;
        pid = current_device_->idProduct;
        if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
            DEBUG_SERIAL_ENUM.printf("\n--- DeviceEnumerator: Device VID:0x%04X PID:0x%04X Disconnected ---\n", vid, pid);
        }
    } else {
        // Log based on stored data if current_device is already null
        vid = stored_data_.idVendor;
        pid = stored_data_.idProduct;
        if (vid != 0 || pid != 0) {
            if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                DEBUG_SERIAL_ENUM.printf("\n--- DeviceEnumerator: Device VID:0x%04X PID:0x%04X Disconnected (from stored) ---\n", vid, pid);
            }
        } else {
            if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                DEBUG_SERIAL_ENUM.println("\n--- DeviceEnumerator: Unknown Device Disconnected ---");
            }
        }
    }

    // Reset state and clear data (which has its own logging)
    init();

    if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
        DEBUG_SERIAL_ENUM.printf("DeviceEnumerator::disconnect: Cleared data for VID:0x%04X PID:0x%04X and reset state.\n", vid, pid);
    }
}

void DeviceEnumerator::startEnumeration(Device_t* device) {
    if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
        DEBUG_SERIAL_ENUM.println("DeviceEnumerator::startEnumeration");
    }
    clearStoredDeviceData(); // clearStoredDeviceData has internal logging control
    current_device_ = device;

    if (!current_device_) {
        if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
            DEBUG_SERIAL_ENUM.println("!!! ERROR: startEnumeration called with NULL device pointer!");
        }
        enum_state_ = STATE_ERROR;
        return;
    }

    // Store basic device info immediately
    stored_data_.idVendor = device->idVendor;
    stored_data_.idProduct = device->idProduct;
    stored_data_.speed = device->speed;
    stored_data_.hub_address = device->hub_address;
    stored_data_.hub_port = device->hub_port;
    stored_data_.address = device->address; // Get assigned address

    if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
        DEBUG_SERIAL_ENUM.printf("  Starting enumeration for: VID=0x%04X, PID=0x%04X, Addr=%u\n", stored_data_.idVendor, stored_data_.idProduct, stored_data_.address);
    }
    enum_state_ = STATE_GETTING_DEVICE_DESC_8;
    processStateMachine(); // Start the state machine
}

void DeviceEnumerator::control(const Transfer_t* transfer) {
    if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
        DEBUG_SERIAL_ENUM.printf("DeviceEnumerator::control - START - Current State: %d\n", enum_state_);
    }

    if (!transfer) {
        if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
            DEBUG_SERIAL_ENUM.println("  control() called with NULL transfer! Aborting.");
        }
        return;
    }

    // Ensure transfer is for this driver instance
    if (transfer->driver != this) {
        if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
            DEBUG_SERIAL_ENUM.println("  control() transfer not for this driver instance. Ignoring.");
        }
        return;
    }

    // Ensure we have a device context (it might have disconnected between queueing and callback)
    if (current_device_ == nullptr) {
        if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
            DEBUG_SERIAL_ENUM.println("  control() called but current_device_ is NULL! (Device likely disconnected). Ignoring transfer.");
        }
         // State should be reset by disconnect() or Task() detecting null device_
         return;
    }

    uint32_t token = transfer->qtd.token;
    if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
        DEBUG_SERIAL_ENUM.printf("  Transfer details: Len=%u, Token=0x%X\n", transfer->length, token);
    }

    // Check for USB errors (Babble, Buffer, Transaction) - Adjust masks as needed for Teensy core
    // Using example masks - VERIFY THESE ARE CORRECT for your specific USBHost_t36 version/core
    #ifndef QTD_TOKEN_STATUS_XACT_ERR
    #define QTD_TOKEN_STATUS_XACT_ERR    0x08 // Transaction Error
    #endif
    #ifndef QTD_TOKEN_STATUS_BUFFER_ERR
    #define QTD_TOKEN_STATUS_BUFFER_ERR  0x20 // Buffer Error
    #endif
    #ifndef QTD_TOKEN_STATUS_HALTED
    #define QTD_TOKEN_STATUS_HALTED      0x40 // Halted (STALL, NAK, etc.)
    #endif
    // #define QTD_TOKEN_STATUS_BABBLE      0x10 // Babble detected (Teensy specific?) - Check exact definition if needed

    if ((token & (QTD_TOKEN_STATUS_XACT_ERR | QTD_TOKEN_STATUS_BUFFER_ERR /* | QTD_TOKEN_STATUS_BABBLE */)) ) { // Check Babble if defined
        if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
            DEBUG_SERIAL_ENUM.printf("!!! FDE::Control USB Error Token:0x%X (Babble/Buffer/Transaction)\n", token);
        }
        enum_state_ = STATE_ERROR;
        processStateMachine(); // Process the error state
        return;
    }

    // Check for STALL
    bool is_stall = (token & QTD_TOKEN_STATUS_HALTED);
    if (is_stall) {
        if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
            DEBUG_SERIAL_ENUM.printf("  STALL received. Current state: %d\n", enum_state_);
        }
        // Allow STALL only during report descriptor fetch for now
        if (enum_state_ != STATE_GETTING_REPORT_DESC) {
            if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                DEBUG_SERIAL_ENUM.println("!!! FDE::Control STALL in unexpected state. Setting ERROR state.");
            }
            enum_state_ = STATE_ERROR;
            processStateMachine();
            return;
        }
        // If getting report desc, the state machine itself handles the stall.
    }

    if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
        DEBUG_SERIAL_ENUM.println("  Control transfer seems OK or STALL handled by state. Calling processStateMachine...");
    }
    processStateMachine(transfer); // Process the successful transfer or allowed stall

    if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
        DEBUG_SERIAL_ENUM.printf("DeviceEnumerator::control - END - New State: %d\n", enum_state_);
    }
}

void DeviceEnumerator::Task() {
    // Check for disconnection first
    // If device pointer is null but state is not idle/done/error, it means disconnect happened.
    if (current_device_ == nullptr &&
        enum_state_ != STATE_IDLE &&
        enum_state_ != STATE_DONE &&
        enum_state_ != STATE_ERROR)
    {
        if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
            DEBUG_SERIAL_ENUM.println("DeviceEnumerator::Task - Device missing but not IDLE/DONE/ERROR. Resetting state.");
        }
        init(); // Reset state and clear data fully
        return;
    }

    // Only run state machine from Task() for non-callback-driven states
    // These are states that need to proceed after data is stored or a non-USB decision is made.
    if ( (enum_state_ == STATE_PARSING_CONFIG) || // Need Task to trigger parsing after config data is stored
         (enum_state_ == STATE_GETTING_CONFIG_STRING && current_device_ != nullptr) || // Need Task if skipping config string
         (enum_state_ == STATE_GETTING_INTERFACE_STRINGS && current_device_ != nullptr) || // Need Task to find next interface string or finish
         (enum_state_ == STATE_GETTING_REPORT_DESC && current_device_ != nullptr) // Need Task to find next report desc or finish
       )
    {
        if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
            DEBUG_SERIAL_ENUM.printf("DeviceEnumerator::Task - Advancing state %d (non-USB callback trigger)\n", enum_state_);
        }
        processStateMachine(); // Call state machine for non-USB driven transitions
    }
    // Note: The -Wswitch warnings reported previously relate to this 'if' block,
    // because it only handles a subset of states. This is intentional and acceptable.
}

// Accessor methods - no logging needed
const UsbDeviceData* DeviceEnumerator::getStoredDeviceData() const {
    if (enum_state_ == STATE_DONE || enum_state_ == STATE_ERROR) {
        return &stored_data_;
    }
    return nullptr;
}

Device_t* DeviceEnumerator::getCurrentDevice() const {
    return current_device_;
}

bool DeviceEnumerator::isEnumerationDone() const {
    return enum_state_ == STATE_DONE;
}

bool DeviceEnumerator::isErrorState() const {
    return enum_state_ == STATE_ERROR;
}


void DeviceEnumerator::clearStoredDeviceData() {
    if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
        DEBUG_SERIAL_ENUM.println("DeviceEnumerator::clearStoredDeviceData");
    }
    // Deallocate memory carefully
    for (uint8_t c = 0; c < MAX_CONFIGURATIONS_PER_DEVICE; ++c) {
        UsbConfigurationData& cfg = stored_data_.configurations[c];
        if (cfg.rawConfigData) {
            delete[] cfg.rawConfigData;
            cfg.rawConfigData = nullptr;
        }
        for (uint8_t i = 0; i < MAX_INTERFACES_PER_CONFIG; ++i) {
            UsbInterfaceData& iface = cfg.interfaces[i];
            for (uint8_t r = 0; r < MAX_HID_REPORT_DESC_PER_INTERFACE; ++r) {
                UsbHidReportDescInfo& report = iface.reportDescriptors[r];
                if (report.rawData) {
                    delete[] report.rawData;
                    report.rawData = nullptr;
                }
            }
        }
    }
    // Reset the main struct and state variables
    stored_data_ = UsbDeviceData(); // Use default constructor to reset
    enum_state_ = STATE_IDLE;
    current_device_ = nullptr; // Clear device pointer
    config_total_len_ = 0;
    current_config_index_ = 0;
    current_interface_index_ = 0;
    current_hid_report_index_ = 0;
    if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
        DEBUG_SERIAL_ENUM.println("  Finished clearing data and resetting state.");
    }
}


// --- processStateMachine Function (with runtime logging checks and scope fixes) ---
// *** REMOVED default argument '= nullptr' from definition ***
void DeviceEnumerator::processStateMachine(const Transfer_t* transfer) {
    bool repeat = false;
    int loop_guard = 0;

    if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
        DEBUG_SERIAL_ENUM.printf(" >> processStateMachine ENTRY: State=%d (Transfer %s)\n", enum_state_, (transfer ? "provided" : "not provided"));
    }

    do {
        repeat = false;
        if (loop_guard++ > 30) {
            if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                DEBUG_SERIAL_ENUM.println("!!! State machine loop guard triggered! Forcing ERROR state.");
            }
            enum_state_ = STATE_ERROR;
            break;
        }
        // Check for disconnection *during* state processing
        if (!current_device_ &&
             enum_state_ != STATE_IDLE &&
             enum_state_ != STATE_DONE &&
             enum_state_ != STATE_ERROR)
        {
            if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                DEBUG_SERIAL_ENUM.printf(" >> processStateMachine: Device disconnected during state %d processing. Aborting state machine.\n", enum_state_);
            }
            // Don't set ERROR here, disconnect() or Task() should handle cleanup
            return; // Exit state machine immediately
        }

        if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
            DEBUG_SERIAL_ENUM.printf(" >> processStateMachine LOOP (Guard %d): State=%d (Transfer %s)\n", loop_guard, enum_state_, (transfer ? "provided" : "not provided"));
        }

        switch (enum_state_) {
            case STATE_IDLE:
            { // Added scope
                // Do nothing, wait for claim
                break;
            } // End scope

            case STATE_GETTING_DEVICE_DESC_8:
            { // Added scope
                if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                    DEBUG_SERIAL_ENUM.println(" State: Requesting DevDesc(8)...");
                }
                queueGetDescriptor(USB_DESCRIPTOR_DEVICE, 0, 0, 8);
                enum_state_ = STATE_GETTING_DEVICE_DESC_FULL;
                // No repeat, wait for control() callback
                break;
            } // End scope

            case STATE_GETTING_DEVICE_DESC_FULL:
            { // Added scope
                if (!transfer) {
                     if (ENABLE_LOGGING_DEVICE_ENUMERATOR) { DEBUG_SERIAL_ENUM.println(" State: GETTING_DEVICE_DESC_FULL waiting..."); }
                     break; // Wait
                }
                if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                    DEBUG_SERIAL_ENUM.println(" State: Processing DevDesc(8), Requesting DevDesc(18)...");
                }
                if (transfer->length < 8 || temp_buffer_[0] < 8 || temp_buffer_[1] != USB_DESCRIPTOR_DEVICE) {
                    if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                        DEBUG_SERIAL_ENUM.printf(" ERROR: Invalid partial DevDesc. Len=%u, bL=%u, bT=%u\n", transfer->length, temp_buffer_[0], temp_buffer_[1]);
                    }
                    enum_state_ = STATE_ERROR;
                    repeat = true;
                    break;
                }
                stored_data_.bMaxPacketSize0 = temp_buffer_[7];
                if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                    DEBUG_SERIAL_ENUM.printf("   Got MaxPacketSize0 = %u\n", stored_data_.bMaxPacketSize0);
                }
                queueGetDescriptor(USB_DESCRIPTOR_DEVICE, 0, 0, sizeof(usb_device_descriptor_t));
                enum_state_ = STATE_GETTING_STRING_MAN;
                break; // Wait
            } // End scope

            case STATE_GETTING_STRING_MAN:
            { // Added scope
                 if (!transfer) {
                     if (ENABLE_LOGGING_DEVICE_ENUMERATOR) { DEBUG_SERIAL_ENUM.println(" State: GETTING_STRING_MAN waiting..."); }
                     break;
                 }
                 if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                    DEBUG_SERIAL_ENUM.println(" State: Processing Full DevDesc...");
                 }
                 if (transfer->length < sizeof(usb_device_descriptor_t) || temp_buffer_[1] != USB_DESCRIPTOR_DEVICE) {
                    if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                        DEBUG_SERIAL_ENUM.printf(" ERROR: Short/Invalid Full DevDesc (Len=%u, Type=%u).\n", transfer->length, temp_buffer_[1]);
                    }
                    enum_state_ = STATE_ERROR;
                    repeat = true;
                    break;
                 }
                 { // Inner scope for descriptor pointer 'd'
                    usb_device_descriptor_t* d = (usb_device_descriptor_t*)temp_buffer_;
                    stored_data_.bcdUSB = d->bcdUSB;
                    stored_data_.bDeviceClass = d->bDeviceClass;
                    stored_data_.bDeviceSubClass = d->bDeviceSubClass;
                    stored_data_.bDeviceProtocol = d->bDeviceProtocol;
                    stored_data_.bcdDevice = d->bcdDevice;
                    stored_data_.iManufacturer = d->iManufacturer;
                    stored_data_.iProduct = d->iProduct;
                    stored_data_.iSerialNumber = d->iSerialNumber;
                    stored_data_.bNumConfigurations = d->bNumConfigurations;
                    // Ensure address is up-to-date if it changed during setup
                    if(current_device_) {
                        stored_data_.address = current_device_->address;
                    }
                 } // End inner scope for 'd'
                 if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                    DEBUG_SERIAL_ENUM.printf("   DevDesc OK: Addr=%u, Class=%X, #Cfg=%u, iM=%u, iP=%u, iS=%u\n", stored_data_.address, stored_data_.bDeviceClass, stored_data_.bNumConfigurations, stored_data_.iManufacturer, stored_data_.iProduct, stored_data_.iSerialNumber);
                 }

                 if (stored_data_.iManufacturer) {
                    if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                        DEBUG_SERIAL_ENUM.println(" State: Requesting Manuf String...");
                    }
                    queueGetDescriptor(USB_DESCRIPTOR_STRING, stored_data_.iManufacturer, 0x0409, MAX_STRING_LENGTH * 2 + 2);
                    enum_state_ = STATE_GETTING_STRING_PROD; // Move to next state
                 } else {
                    if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                        DEBUG_SERIAL_ENUM.println(" State: Skipping Manuf String.");
                    }
                    enum_state_ = STATE_GETTING_STRING_PROD; // Move to next state
                    repeat = true; // Immediately evaluate next state
                 }
                 break; // Wait or repeat
            } // End scope

            case STATE_GETTING_STRING_PROD:
            { // Added scope
                 if (transfer) {
                     // storeString has its own internal logging control
                     storeString(stored_data_.manufacturerString, MAX_STRING_LENGTH, transfer);
                 } else {
                     if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                         DEBUG_SERIAL_ENUM.println(" State: GETTING_STRING_PROD entered.");
                     }
                 }

                 if (stored_data_.iProduct) {
                      if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                          DEBUG_SERIAL_ENUM.println(" State: Requesting Prod String...");
                      }
                      queueGetDescriptor(USB_DESCRIPTOR_STRING, stored_data_.iProduct, 0x0409, MAX_STRING_LENGTH * 2 + 2);
                      enum_state_ = STATE_GETTING_STRING_SERIAL;
                 } else {
                      if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                          DEBUG_SERIAL_ENUM.println(" State: Skipping Prod String.");
                      }
                      enum_state_ = STATE_GETTING_STRING_SERIAL;
                      repeat = true;
                 }
                 break; // Wait or repeat
            } // End scope

             case STATE_GETTING_STRING_SERIAL:
             { // Added scope
                 if (transfer) {
                     storeString(stored_data_.productString, MAX_STRING_LENGTH, transfer);
                 } else {
                     if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                         DEBUG_SERIAL_ENUM.println(" State: GETTING_STRING_SERIAL entered.");
                     }
                 }

                 if (stored_data_.iSerialNumber) {
                     if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                         DEBUG_SERIAL_ENUM.println(" State: Requesting Serial String...");
                     }
                     queueGetDescriptor(USB_DESCRIPTOR_STRING, stored_data_.iSerialNumber, 0x0409, MAX_STRING_LENGTH * 2 + 2);
                     enum_state_ = STATE_GETTING_CONFIG_HEADER;
                 } else {
                     if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                         DEBUG_SERIAL_ENUM.println(" State: Skipping Serial String.");
                     }
                     enum_state_ = STATE_GETTING_CONFIG_HEADER;
                     repeat = true;
                 }
                 break; // Wait or repeat
            } // End scope

            case STATE_GETTING_CONFIG_HEADER:
            { // Added scope
                 if (transfer) {
                     // Process potential serial string from previous state
                     storeString(stored_data_.serialNumberString, MAX_STRING_LENGTH, transfer);
                 } else {
                     if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                         DEBUG_SERIAL_ENUM.println(" State: GETTING_CONFIG_HEADER entered.");
                     }
                 }

                 if (stored_data_.bNumConfigurations == 0) {
                    if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                        DEBUG_SERIAL_ENUM.println(" WARN: Device reports 0 configurations. DONE.");
                    }
                    enum_state_ = STATE_DONE;
                    repeat = true;
                    break;
                 }
                 if (stored_data_.bNumConfigurations > MAX_CONFIGURATIONS_PER_DEVICE) {
                    if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                        DEBUG_SERIAL_ENUM.printf(" WARN: Device reports %u configs, only storing %u.\n", stored_data_.bNumConfigurations, MAX_CONFIGURATIONS_PER_DEVICE);
                    }
                    // Continue with the first config
                 }
                 if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                    DEBUG_SERIAL_ENUM.println(" State: Requesting Config Header(9 bytes)...");
                 }
                 // Get first config header (index 0)
                 queueGetDescriptor(USB_DESCRIPTOR_CONFIGURATION, 0, 0, sizeof(usb_configuration_descriptor_t));
                 enum_state_ = STATE_GETTING_CONFIG_FULL;
                 break; // Wait
            } // End scope

            case STATE_GETTING_CONFIG_FULL:
            { // Added scope
                 if (!transfer) {
                      if (ENABLE_LOGGING_DEVICE_ENUMERATOR) { DEBUG_SERIAL_ENUM.println(" State: GETTING_CONFIG_FULL waiting..."); }
                      break;
                 }
                 if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                    DEBUG_SERIAL_ENUM.println(" State: Processing Config Header...");
                 }
                 if (transfer->length < sizeof(usb_configuration_descriptor_t) || temp_buffer_[1] != USB_DESCRIPTOR_CONFIGURATION) {
                    if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                        DEBUG_SERIAL_ENUM.printf(" ERROR: Invalid Config Header. Len=%u, Type=%u\n", transfer->length, temp_buffer_[1]);
                    }
                    enum_state_ = STATE_ERROR;
                    repeat = true;
                    break;
                 }
                 { // Inner scope for descriptor pointer 'c'
                    usb_configuration_descriptor_t* c = (usb_configuration_descriptor_t*)temp_buffer_;
                    config_total_len_ = c->wTotalLength;
                    if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                        DEBUG_SERIAL_ENUM.printf("   Config Header OK: wTotalLength=%u, #Intf=%u, CfgVal=%u\n", config_total_len_, c->bNumInterfaces, c->bConfigurationValue);
                    }
                    if (config_total_len_ < sizeof(usb_configuration_descriptor_t) || config_total_len_ == 0) {
                        if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                            DEBUG_SERIAL_ENUM.println(" ERROR: Invalid/Zero wTotalLength.");
                        }
                        enum_state_ = STATE_ERROR;
                        repeat = true;
                        break;
                    }
                    // Store header info now, before full request
                    if (stored_data_.configurationCount < MAX_CONFIGURATIONS_PER_DEVICE) {
                       UsbConfigurationData& cfg_data = stored_data_.configurations[stored_data_.configurationCount];
                       // Clear previous if any - Use default constructor assignment
                       cfg_data = UsbConfigurationData();
                       cfg_data.bConfigurationValue = c->bConfigurationValue;
                       cfg_data.iConfiguration = c->iConfiguration;
                       cfg_data.bmAttributes = c->bmAttributes;
                       cfg_data.bMaxPower = c->bMaxPower;
                       cfg_data.wTotalLength = c->wTotalLength; // Store reported length
                       // rawConfigData allocated later
                    } // else: Ignore config if exceeds max (already warned)
                 } // End inner scope for 'c'

                 // *** Variable declared within its case scope ***
                 uint16_t request_len = min(config_total_len_, (uint16_t)TEMP_BUFFER_SIZE);
                 if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                    DEBUG_SERIAL_ENUM.printf(" State: Requesting Full Config (Req %u bytes, total %u)...\n", request_len, config_total_len_);
                 }
                 // Get full first config
                 queueGetDescriptor(USB_DESCRIPTOR_CONFIGURATION, 0, 0, request_len);
                 enum_state_ = STATE_PARSING_CONFIG;
                 break; // Wait
            } // End scope

            case STATE_PARSING_CONFIG:
            { // Added scope
                 // Entered EITHER by control() with the full config data, OR by Task() to trigger parsing
                 if (transfer) {
                     // --- Received Full Config Data ---
                     if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                        DEBUG_SERIAL_ENUM.println(" State: PARSING_CONFIG (via control) - Storing Full Config Data...");
                     }
                     uint16_t actual_recv_len = (uint16_t)transfer->length;
                     uint32_t tk = transfer->qtd.token;
                     // Check token for errors again
                     if ((tk & (QTD_TOKEN_STATUS_HALTED | QTD_TOKEN_STATUS_BUFFER_ERR | QTD_TOKEN_STATUS_XACT_ERR /* | QTD_TOKEN_STATUS_BABBLE */))) {
                        if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                            DEBUG_SERIAL_ENUM.printf("!!! ERROR: USB Error/STALL (0x%X) on GetFullConfig.\n", tk);
                        }
                        enum_state_ = STATE_ERROR;
                        repeat = true;
                        break;
                     }
                     if (actual_recv_len == 0 && config_total_len_ > 0) {
                        if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                            DEBUG_SERIAL_ENUM.println("!!! ERROR: Received 0 bytes for non-zero expected config length.");
                        }
                        enum_state_ = STATE_ERROR;
                        repeat = true;
                        break;
                     }

                    // Determine length to store: minimum of expected, received, and buffer size
                    uint16_t len_to_store = min((uint16_t)config_total_len_, min(actual_recv_len, (uint16_t)TEMP_BUFFER_SIZE));

                    if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                        DEBUG_SERIAL_ENUM.printf("   Received %u bytes. Storing %u bytes.\n", actual_recv_len, len_to_store);
                    }

                    if (stored_data_.configurationCount < MAX_CONFIGURATIONS_PER_DEVICE) {
                         UsbConfigurationData& cfg = stored_data_.configurations[stored_data_.configurationCount];
                         // Allocate or reallocate buffer
                         if (cfg.rawConfigData) {
                             delete[] cfg.rawConfigData;
                             cfg.rawConfigData = nullptr;
                         }
                         if (len_to_store > 0) {
                             cfg.rawConfigData = new (std::nothrow) uint8_t[len_to_store];
                             if (cfg.rawConfigData) {
                                 memcpy(cfg.rawConfigData, temp_buffer_, len_to_store);
                                 cfg.rawConfigLen = len_to_store;
                                 // Increment count *after* successful storage
                                 stored_data_.configurationCount++;
                                 if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                                    DEBUG_SERIAL_ENUM.println("   Stored raw config successfully.");
                                 }
                                 // CRITICAL: Do NOT set repeat=true here. Break and wait for Task() to trigger parsing.
                             } else {
                                 if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                                    DEBUG_SERIAL_ENUM.println("!!! ERROR: Malloc failed for raw config.");
                                 }
                                 cfg.rawConfigLen = 0; // Ensure length is 0
                                 enum_state_ = STATE_ERROR;
                                 repeat = true; // Error, try to repeat/exit
                             }
                         } else {
                              if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                                 DEBUG_SERIAL_ENUM.println("!!! ERROR: Zero length determined for config storage.");
                              }
                              cfg.rawConfigLen = 0;
                              enum_state_ = STATE_ERROR;
                              repeat = true; // Error
                         }
                    } else {
                         if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                            DEBUG_SERIAL_ENUM.println(" WARN: Received config data but no more storage slots available.");
                         }
                         // Ignore data, proceed as if parsed (but empty) or error? Go to next state maybe.
                         // For simplicity, let's treat this as an error for now.
                         enum_state_ = STATE_ERROR;
                         repeat = true;
                    }
                 } else {
                     // --- Triggered by Task() ---
                     if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                        DEBUG_SERIAL_ENUM.println(" State: PARSING_CONFIG (via Task) - Checking stored data...");
                     }
                     // We assume only one config for now (index 0)
                     if (stored_data_.configurationCount > 0 &&
                         stored_data_.configurations[0].rawConfigData != nullptr &&
                         stored_data_.configurations[0].rawConfigLen > 0)
                     {
                         if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                            DEBUG_SERIAL_ENUM.println("   Parsing stored data...");
                         }
                         parseFullConfigurationBlock(); // parse function has own logging control
                         if (enum_state_ != STATE_ERROR) { // Check if parsing failed
                            if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                                DEBUG_SERIAL_ENUM.println("   Parsing seems OK. Moving to get strings.");
                            }
                             current_config_index_ = 0; // Reset indices for string/report fetching
                             current_interface_index_ = 0;
                             current_hid_report_index_ = 0;
                             enum_state_ = STATE_GETTING_CONFIG_STRING;
                             repeat = true; // Immediately evaluate next state
                         } else {
                             if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                                 DEBUG_SERIAL_ENUM.println("   Parsing failed. State set to ERROR.");
                             }
                              // No repeat, stay in ERROR state.
                         }
                     } else {
                         if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                            DEBUG_SERIAL_ENUM.println("   No valid raw config data found to parse. Moving on/Error?");
                            DEBUG_SERIAL_ENUM.println("   WARN: Skipping string/report fetching due to missing config data.");
                         }
                         // Consider this DONE if config couldn't be parsed/stored
                         enum_state_ = STATE_DONE;
                         repeat = true;
                     }
                 }
                 break; // Break is essential for Task() trigger and repeat logic
            } // End scope

            case STATE_GETTING_CONFIG_STRING:
            { // Added scope
                 // Entered either via repeat=true or Task()
                 if (transfer) {
                      if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                         DEBUG_SERIAL_ENUM.println(" WARN: STATE_GETTING_CONFIG_STRING unexpected transfer.");
                      }
                      break; // Should not happen
                 }

                 if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                    DEBUG_SERIAL_ENUM.println(" State: Checking for Config String...");
                 }
                 // Check config 0 only
                 if (stored_data_.configurationCount > 0 && stored_data_.configurations[0].iConfiguration > 0) {
                     uint8_t string_index = stored_data_.configurations[0].iConfiguration;
                     if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                        DEBUG_SERIAL_ENUM.printf(" State: Requesting Config String (idx %u)...\n", string_index);
                     }
                     queueGetDescriptor(USB_DESCRIPTOR_STRING, string_index, 0x0409, MAX_STRING_LENGTH * 2 + 2);
                     enum_state_ = STATE_GETTING_INTERFACE_STRINGS;
                     current_interface_index_ = 0; // Reset for interface strings
                     // No repeat, wait for control()
                 } else {
                     if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                        DEBUG_SERIAL_ENUM.println(" State: No Config String to fetch.");
                     }
                     enum_state_ = STATE_GETTING_INTERFACE_STRINGS;
                     current_interface_index_ = 0; // Reset for interface strings
                     repeat = true; // Immediately check interface strings
                 }
                 break;
            } // End scope

            case STATE_GETTING_INTERFACE_STRINGS:
            { // Added scope
                 // Entered either via control() with a string, Task(), or repeat=true
                 if (transfer) {
                    if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                        DEBUG_SERIAL_ENUM.println(" State: Processing received string...");
                    }
                    // Figure out which string this was based on state BEFORE the request was queued
                    bool is_config_string = (current_interface_index_ == 0 && stored_data_.configurationCount > 0 && stored_data_.configurations[0].iConfiguration > 0);
                    uint8_t relevant_interface_idx = current_interface_index_ - 1;

                    if (is_config_string) {
                         storeString(stored_data_.configurations[0].configurationString, MAX_STRING_LENGTH, transfer);
                    } else if (stored_data_.configurationCount > 0 &&
                               relevant_interface_idx < stored_data_.configurations[0].interfaceCount)
                    {
                        UsbInterfaceData& iface = stored_data_.configurations[0].interfaces[relevant_interface_idx];
                        if (iface.iInterface > 0) {
                           storeString(iface.interfaceString, MAX_STRING_LENGTH, transfer);
                        } else {
                           if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                               DEBUG_SERIAL_ENUM.println(" WARN: Received string, but previous interface index had iInterface=0.");
                           }
                        }
                    } else {
                        if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                            DEBUG_SERIAL_ENUM.println(" WARN: Received string, context unclear or index out of bounds.");
                        }
                    }
                 } else {
                     if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                         DEBUG_SERIAL_ENUM.println(" State: GETTING_INTERFACE_STRINGS entered (Task/repeat).");
                     }
                 }

                 // *** Variable declared within its case scope ***
                 bool request_queued = false;
                 if (stored_data_.configurationCount > 0) {
                     UsbConfigurationData& config = stored_data_.configurations[0];
                     while (current_interface_index_ < config.interfaceCount) {
                         UsbInterfaceData& iface = config.interfaces[current_interface_index_];
                         if (iface.iInterface > 0) {
                             if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                                DEBUG_SERIAL_ENUM.printf(" State: Requesting Interface String (iface #%u, Struct Idx %u, String Idx=%u)...\n", iface.bInterfaceNumber, current_interface_index_, iface.iInterface);
                             }
                             queueGetDescriptor(USB_DESCRIPTOR_STRING, iface.iInterface, 0x0409, MAX_STRING_LENGTH * 2 + 2);
                             current_interface_index_++;
                             request_queued = true;
                             enum_state_ = STATE_GETTING_INTERFACE_STRINGS;
                             break;
                         }
                         current_interface_index_++;
                     }
                 }

                 if (!request_queued) {
                     if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                        DEBUG_SERIAL_ENUM.println(" State: Done fetching Interface Strings.");
                     }
                     enum_state_ = STATE_GETTING_REPORT_DESC;
                     current_config_index_ = 0;
                     current_interface_index_ = 0;
                     current_hid_report_index_ = 0;
                     repeat = true;
                 }
                 break;
            } // End scope

            case STATE_GETTING_REPORT_DESC:
            { // Added scope
                // *** REMOVED unused variable processed_transfer ***

                // Entered via control() with report data, Task(), or repeat=true
                if (transfer) {
                    if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                        DEBUG_SERIAL_ENUM.println(" State: GETTING_REPORT_DESC processing received transfer...");
                    }
                    // --- Process received data (could be report descriptor or STALL) ---
                    bool found_pending = false;
                    uint8_t completed_iface_idx = 0;
                    uint8_t completed_report_idx = 0;

                    // Find which report descriptor this transfer corresponds to.
                    if (stored_data_.configurationCount > current_config_index_) {
                        UsbConfigurationData& c = stored_data_.configurations[current_config_index_];
                        for(uint8_t ii = 0; ii < c.interfaceCount && !found_pending; ++ii) {
                            UsbInterfaceData& i = c.interfaces[ii];
                            if(i.isHidInterface) {
                                for(uint8_t rr = 0; rr < i.reportDescriptorCount; ++rr) {
                                    UsbHidReportDescInfo& r = i.reportDescriptors[rr];
                                    if (r.wDescriptorLength > 0 && r.rawData == nullptr) {
                                        completed_iface_idx = ii;
                                        completed_report_idx = rr;
                                        found_pending = true;
                                        break;
                                    }
                                }
                            }
                        }
                    } // End search

                    if (found_pending) {
                        if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                           DEBUG_SERIAL_ENUM.printf("   Transfer likely corresponds to Iface %u (Struct Idx %u), Report Idx %d.\n",
                                   stored_data_.configurations[current_config_index_].interfaces[completed_iface_idx].bInterfaceNumber,
                                   completed_iface_idx, completed_report_idx);
                        }
                        UsbInterfaceData& i = stored_data_.configurations[current_config_index_].interfaces[completed_iface_idx];
                        UsbHidReportDescInfo& r = i.reportDescriptors[completed_report_idx];
                        uint16_t actual_recv_len = (uint16_t)transfer->length;
                        uint32_t tk = transfer->qtd.token;

                        if (tk & QTD_TOKEN_STATUS_HALTED) {
                             if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                                DEBUG_SERIAL_ENUM.printf("   STALL received. Marking Report Desc Iface %u, Idx %d as unavailable.\n", i.bInterfaceNumber, completed_report_idx);
                             }
                             r.wDescriptorLength = 0;
                             r.rawData = nullptr;
                        }
                        else if (actual_recv_len > 0) {
                            uint16_t len_to_store = min(r.wDescriptorLength, actual_recv_len);
                            // Ensure previous data is deleted if retrying
                            if (r.rawData) delete[] r.rawData;
                            r.rawData = new (std::nothrow) uint8_t[len_to_store];
                            if (r.rawData) {
                                memcpy(r.rawData, temp_buffer_, len_to_store);
                                r.wDescriptorLength = len_to_store;
                                if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                                   DEBUG_SERIAL_ENUM.printf("   Stored Report Desc Iface %u Idx %d (%u bytes)\n", i.bInterfaceNumber, completed_report_idx, len_to_store);
                                }
                            } else {
                                if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                                   DEBUG_SERIAL_ENUM.println("!!! ERROR: Malloc failed for report desc!");
                                }
                                r.wDescriptorLength = 0;
                                r.rawData = nullptr;
                            }
                        } else {
                             if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                                DEBUG_SERIAL_ENUM.printf(" WARN: Received 0 bytes (not STALL) for Report Desc Iface %u Idx %d.\n", i.bInterfaceNumber, completed_report_idx);
                             }
                             r.wDescriptorLength = 0;
                             r.rawData = nullptr;
                        }
                    } else {
                        if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                            DEBUG_SERIAL_ENUM.println(" WARN: Received transfer in GETTING_REPORT_DESC, but no pending report found.");
                        }
                    }
                    repeat = true; // Immediately search for the next report descriptor request
                } // End if(transfer) block


                // --- Search for the *next* Report Descriptor to request ---
                if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                    DEBUG_SERIAL_ENUM.println(" State: Searching for next Report Desc to fetch...");
                }
                // *** Variable declared within its case scope ***
                bool request_queued_report = false;
                if (stored_data_.configurationCount > current_config_index_) {
                    UsbConfigurationData& c = stored_data_.configurations[current_config_index_];
                    for (; current_interface_index_ < c.interfaceCount; ++current_interface_index_) {
                        UsbInterfaceData& i = c.interfaces[current_interface_index_];
                        if (i.isHidInterface) {
                            for (; current_hid_report_index_ < i.reportDescriptorCount; ++current_hid_report_index_) {
                                UsbHidReportDescInfo& r = i.reportDescriptors[current_hid_report_index_];
                                if (r.wDescriptorLength > 0 && r.rawData == nullptr) {
                                    if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                                        DEBUG_SERIAL_ENUM.printf("   Found pending: Iface #%u (Struct Idx %u), Report Idx %d. Queueing request...\n", i.bInterfaceNumber, current_interface_index_, current_hid_report_index_);
                                    }
                                    queueGetHidReportDescriptor(current_config_index_, current_interface_index_, current_hid_report_index_);
                                    current_hid_report_index_++;
                                    request_queued_report = true;
                                    goto report_search_end; // Use goto to break nested loops and wait
                                }
                            } // End report loop
                        } // End if HID
                        current_hid_report_index_ = 0; // Reset report index for the *next* interface
                    } // End interface loop
                } // End config check
            report_search_end:; // Label for goto jump

                if (!request_queued_report) {
                     if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                        DEBUG_SERIAL_ENUM.println(" State: No more Report Descriptors to fetch. Enumeration DONE.");
                     }
                     enum_state_ = STATE_DONE;
                     repeat = true; // Go evaluate DONE state
                }
                break; // Wait for queued request or repeat if DONE/Error
            } // End scope

            case STATE_DONE:
            { // Added scope
                if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                    DEBUG_SERIAL_ENUM.println(" State: Processing DONE state.");
                }
                repeat = false; // Stay DONE
                break;
            } // End scope

            case STATE_ERROR:
            { // Added scope
                if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                    DEBUG_SERIAL_ENUM.println("!!! State: Processing ERROR state.");
                }
                repeat = false; // Stay ERROR
                break;
            } // End scope

            default:
            { // Added scope
                if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                    DEBUG_SERIAL_ENUM.printf("!!! State: UNKNOWN state (%d)! Halting.\n", enum_state_);
                }
                enum_state_ = STATE_ERROR;
                repeat = false;
                break;
            } // End scope
        } // end switch
    } while (repeat);

    if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
        DEBUG_SERIAL_ENUM.printf(" << processStateMachine EXIT: State=%d\n", enum_state_);
    }
}


void DeviceEnumerator::queueGetDescriptor(uint8_t desc_type, uint8_t desc_index, uint16_t lang_id, uint16_t len) {
    if (!current_device_) {
        if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
           DEBUG_SERIAL_ENUM.println("!!! queueGetDescriptor: No current device!");
        }
        return;
    }
    if (len == 0) {
        if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
           DEBUG_SERIAL_ENUM.println(" WARN: queueGetDescriptor called with length 0. Aborting queue.");
        }
        return;
    }
    // Limit request length to buffer size
    uint16_t request_len = len;
    if (request_len > TEMP_BUFFER_SIZE) {
        request_len = TEMP_BUFFER_SIZE;
    }

    if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
       DEBUG_SERIAL_ENUM.printf("  Queueing GetDescriptor: Type=%u (%s), Idx=%u, Lang=0x%X, ReqLen=%u (OrigLen=%u)\n",
            desc_type, descriptor_type_to_string(desc_type), desc_index, lang_id, request_len, len);
    }

    // Create setup packet
    mk_setup(setup_packet_, 0x80, USB_REQUEST_GET_DESCRIPTOR, (desc_type << 8) | desc_index, lang_id, request_len);

    // Queue the control transfer
    bool success = queue_Control_Transfer(current_device_, &setup_packet_, temp_buffer_, this);
    if (!success) {
        if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
           DEBUG_SERIAL_ENUM.println("!!! ERROR: queue_Control_Transfer failed! Setting ERROR state.");
        }
        enum_state_ = STATE_ERROR;
    }
}

void DeviceEnumerator::queueGetHidReportDescriptor(uint8_t config_idx, uint8_t iface_idx, uint8_t report_idx) {
    if (!current_device_) {
        if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
           DEBUG_SERIAL_ENUM.println("!!! queueGetHidReportDescriptor: No current device!");
        }
        return;
    }
    // Bounds checks
    if (config_idx >= stored_data_.configurationCount || config_idx >= MAX_CONFIGURATIONS_PER_DEVICE) {
        if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
           DEBUG_SERIAL_ENUM.printf("!!! queueGetHidReportDescriptor: Invalid config index %u\n", config_idx);
        }
        return;
    }
    UsbConfigurationData& c = stored_data_.configurations[config_idx];
    if (iface_idx >= c.interfaceCount || iface_idx >= MAX_INTERFACES_PER_CONFIG) {
         if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
            DEBUG_SERIAL_ENUM.printf("!!! queueGetHidReportDescriptor: Invalid interface index %u\n", iface_idx);
         }
         return;
    }
    UsbInterfaceData& i = c.interfaces[iface_idx];
    if (report_idx >= i.reportDescriptorCount || report_idx >= MAX_HID_REPORT_DESC_PER_INTERFACE) {
         if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
             DEBUG_SERIAL_ENUM.printf("!!! queueGetHidReportDescriptor: Invalid report index %u\n", report_idx);
         }
         return;
    }
    UsbHidReportDescInfo& r = i.reportDescriptors[report_idx];

    // Check if the descriptor info indicates a valid length was parsed
    if (r.wDescriptorLength == 0) {
         if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
             DEBUG_SERIAL_ENUM.printf(" WARN: Skipping GetHidReportDescriptor for Iface %u Idx %d - stored length is 0.\n", i.bInterfaceNumber, report_idx);
         }
         r.rawData = nullptr; // Ensure data pointer is null
         // Need to advance state machine? Task() should handle finding the next one.
         Task(); // Call Task to potentially advance state if this was the only one left
         return;
    }

    // Limit request length to buffer size
    uint16_t len_to_request = min(r.wDescriptorLength, (uint16_t)TEMP_BUFFER_SIZE);

    if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
        DEBUG_SERIAL_ENUM.printf("  Queueing GetHidReportDescriptor: Iface=%u, ReportIdx=%d, ReqLen=%u (TotalLen=%u)\n",
             i.bInterfaceNumber, report_idx, len_to_request, r.wDescriptorLength);
    }

    // Create setup packet for GET_DESCRIPTOR (HID Report type)
    mk_setup(setup_packet_, 0x81, USB_REQUEST_GET_DESCRIPTOR, (USB_DESC_TYPE_REPORT << 8) | 0, i.bInterfaceNumber, len_to_request);

    // Queue the control transfer
    bool success = queue_Control_Transfer(current_device_, &setup_packet_, temp_buffer_, this);
    if (!success) {
        if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
            DEBUG_SERIAL_ENUM.println("!!! ERROR: queue_Control_Transfer failed for HID Report Desc! Setting ERROR state.");
        }
        enum_state_ = STATE_ERROR;
    }
}

void DeviceEnumerator::parseFullConfigurationBlock() {
    if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
        DEBUG_SERIAL_ENUM.println("\n--- Starting parseFullConfigurationBlock ---");
    }

    // We only handle config 0
    if (stored_data_.configurationCount == 0 || !stored_data_.configurations[0].rawConfigData) {
        if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
           DEBUG_SERIAL_ENUM.println(" ERROR: No raw config data available to parse.");
        }
        enum_state_ = STATE_ERROR;
        return;
    }

    UsbConfigurationData& config = stored_data_.configurations[0];
    const uint8_t* buffer = config.rawConfigData;
    uint16_t len = config.rawConfigLen;
    const uint8_t* ptr = buffer;
    const uint8_t* end_ptr = buffer + len;

    if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
        DEBUG_SERIAL_ENUM.printf(" Parsing %u bytes of raw config data.\n", len);
    }

    // Basic validation of the start of the block
    if (ptr + sizeof(usb_configuration_descriptor_t) > end_ptr || ptr[1] != USB_DESCRIPTOR_CONFIGURATION) {
        if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
           DEBUG_SERIAL_ENUM.println(" ERROR: Raw data does not start with a valid Config Desc header.");
        }
        enum_state_ = STATE_ERROR;
        return;
    }

    // Process the main configuration descriptor (already stored basic info)
    usb_configuration_descriptor_t* cfg_desc = (usb_configuration_descriptor_t*)ptr;
    // We previously stored: bConfigurationValue, iConfiguration, bmAttributes, bMaxPower, wTotalLength
    if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
        DEBUG_SERIAL_ENUM.printf(" Config Desc: CfgVal=%u, iCfg=%u, Attr=0x%X, Pwr=%umA, #Intf=%u, RepLen=%u\n",
            config.bConfigurationValue, config.iConfiguration, config.bmAttributes, config.bMaxPower * 2, cfg_desc->bNumInterfaces, config.wTotalLength);
    }
    ptr += cfg_desc->bLength;

    // Prepare for parsing interfaces and endpoints
    UsbInterfaceData* currentInterface = nullptr;
    config.interfaceCount = 0; // Reset count before parsing

    // Loop through the rest of the configuration block
    while (ptr < end_ptr) {
        // Check for minimum descriptor header size
        if (ptr + 2 > end_ptr) {
            if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
               DEBUG_SERIAL_ENUM.println(" ERROR: Malformed block (EOF before descriptor header). Parsing stopped.");
            }
            break; // Stop parsing
        }

        uint8_t dLen = ptr[0];
        uint8_t dType = ptr[1];

        // Validate descriptor length
        if (dLen < 2) {
            if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
               DEBUG_SERIAL_ENUM.printf(" ERROR: Invalid descriptor length %u at offset %ld. Parsing stopped.\n", dLen, (long)(ptr - buffer));
            }
            break; // Stop parsing
        }
        if (ptr + dLen > end_ptr) {
             if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                DEBUG_SERIAL_ENUM.printf(" ERROR: Descriptor length %u exceeds remaining buffer size at offset %ld. Parsing stopped.\n", dLen, (long)(ptr - buffer));
             }
             break; // Stop parsing
        }

        if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
            DEBUG_SERIAL_ENUM.printf("\n Found Desc: Type=0x%02X (%s), Length=%u at offset %ld\n",
                dType, descriptor_type_to_string(dType), dLen, (long)(ptr - buffer));
        }

        // Process known descriptor types
        switch (dType) {
            case USB_DESCRIPTOR_INTERFACE:
            { // Added scope
                if (config.interfaceCount < MAX_INTERFACES_PER_CONFIG) {
                    if (dLen < sizeof(usb_interface_descriptor_t)) {
                         if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                            DEBUG_SERIAL_ENUM.println(" ERROR: Interface descriptor too short.");
                         }
                         enum_state_ = STATE_ERROR;
                         goto parse_end;
                    } else {
                        currentInterface = &config.interfaces[config.interfaceCount];
                        *currentInterface = UsbInterfaceData();
                        usb_interface_descriptor_t* ifd = (usb_interface_descriptor_t*)ptr;
                        currentInterface->bInterfaceNumber = ifd->bInterfaceNumber;
                        currentInterface->bAlternateSetting = ifd->bAlternateSetting;
                        currentInterface->bNumEndpoints = ifd->bNumEndpoints;
                        currentInterface->bInterfaceClass = ifd->bInterfaceClass;
                        currentInterface->bInterfaceSubClass = ifd->bInterfaceSubClass;
                        currentInterface->bInterfaceProtocol = ifd->bInterfaceProtocol;
                        currentInterface->iInterface = ifd->iInterface;
                        currentInterface->isHidInterface = (ifd->bInterfaceClass == 3);
                        currentInterface->endpointCount = 0;
                        currentInterface->reportDescriptorCount = 0;

                        if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                            DEBUG_SERIAL_ENUM.printf(" -> Interface Stored (Index %u): Num=%u (Alt %u), Class=%X, Sub=%X, Proto=%X, #EPs=%u, iInterface=%u %s\n",
                                config.interfaceCount, currentInterface->bInterfaceNumber, currentInterface->bAlternateSetting,
                                currentInterface->bInterfaceClass, currentInterface->bInterfaceSubClass, currentInterface->bInterfaceProtocol,
                                currentInterface->bNumEndpoints, currentInterface->iInterface, (currentInterface->isHidInterface ? "[HID]" : ""));
                        }
                        config.interfaceCount++;
                    }
                } else {
                     if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                        DEBUG_SERIAL_ENUM.printf(" WARN: Exceeded MAX_INTERFACES_PER_CONFIG (%u). Skipping interface.\n", MAX_INTERFACES_PER_CONFIG);
                     }
                     currentInterface = nullptr;
                }
                break;
            } // End scope

            case USB_DESCRIPTOR_ENDPOINT:
            { // Added scope
                if (currentInterface == nullptr) {
                    if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                       DEBUG_SERIAL_ENUM.println(" WARN: Found Endpoint descriptor without a current Interface context. Skipping.");
                    }
                } else if (currentInterface->endpointCount >= MAX_ENDPOINTS_PER_INTERFACE) {
                    if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                       DEBUG_SERIAL_ENUM.printf(" WARN: Exceeded MAX_ENDPOINTS_PER_INTERFACE (%u) for Iface %u. Skipping endpoint.\n", MAX_ENDPOINTS_PER_INTERFACE, currentInterface->bInterfaceNumber);
                    }
                } else if (dLen < sizeof(usb_endpoint_descriptor_t)) {
                     if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                        DEBUG_SERIAL_ENUM.println(" ERROR: Endpoint descriptor too short. Skipping.");
                     }
                } else {
                    UsbEndpointData& epd = currentInterface->endpoints[currentInterface->endpointCount];
                    epd = UsbEndpointData();
                    usb_endpoint_descriptor_t* epd_raw = (usb_endpoint_descriptor_t*)ptr;
                    epd.bEndpointAddress = epd_raw->bEndpointAddress;
                    epd.bmAttributes = epd_raw->bmAttributes;
                    epd.wMaxPacketSize = epd_raw->wMaxPacketSize;
                    epd.bInterval = epd_raw->bInterval;
                    epd.bSynchAddress = 0;
                    if (((epd.bmAttributes & 0x03) == 1) && dLen >= 9) {
                        epd.bSynchAddress = ptr[8];
                    }
                    if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                        uint16_t maxPktSize = epd.wMaxPacketSize & 0x7FF;
                        DEBUG_SERIAL_ENUM.printf("    -> Endpoint Stored (Index %u): Addr=0x%X (%s), Attr=0x%X (",
                             currentInterface->endpointCount, epd.bEndpointAddress,
                             ((epd.bEndpointAddress & 0x80) ? "IN" : "OUT"),
                             epd.bmAttributes);
                        print_endpoint_attributes(DEBUG_SERIAL_ENUM, epd.bmAttributes);
                        DEBUG_SERIAL_ENUM.printf("), MaxPkt=%u, Interval=%u", maxPktSize, epd.bInterval);
                        if (epd.bSynchAddress != 0) {
                           DEBUG_SERIAL_ENUM.printf(", SyncAddr=%u", epd.bSynchAddress);
                        }
                        DEBUG_SERIAL_ENUM.println();
                    }
                    currentInterface->endpointCount++;
                }
                break;
            } // End scope

            case USB_DESC_TYPE_HID:
            { // Added scope
                if (currentInterface == nullptr) {
                    if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                       DEBUG_SERIAL_ENUM.println(" WARN: Found HID descriptor without a current Interface context. Skipping.");
                    }
                } else if (!currentInterface->isHidInterface) {
                    if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                       DEBUG_SERIAL_ENUM.printf(" WARN: Found HID descriptor associated with non-HID Interface %u. Skipping.\n", currentInterface->bInterfaceNumber);
                    }
                } else if (dLen < offsetof(usb_hid_descriptor_t, optional_descriptors)) {
                     if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                        DEBUG_SERIAL_ENUM.println(" ERROR: HID descriptor header too short. Skipping.");
                     }
                } else {
                    usb_hid_descriptor_t* hd = (usb_hid_descriptor_t*)ptr;
                    currentInterface->bcdHID = hd->bcdHID;
                    currentInterface->bCountryCode = hd->bCountryCode;
                    currentInterface->bNumHidClassDescriptors = hd->bNumDescriptors;
                    currentInterface->reportDescriptorCount = 0;

                    if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                       DEBUG_SERIAL_ENUM.printf("    -> HID Class Desc Stored: bcd=0x%X, Country=%u, #OptionalDesc=%u\n",
                            currentInterface->bcdHID, currentInterface->bCountryCode, currentInterface->bNumHidClassDescriptors);
                    }
                    const uint8_t* optional_desc_ptr = ((const uint8_t*)hd) + offsetof(usb_hid_descriptor_t, optional_descriptors);
                    const uint8_t* hid_desc_end = ptr + dLen;

                    for (int i = 0; i < currentInterface->bNumHidClassDescriptors; ++i) {
                        if (optional_desc_ptr + 3 > hid_desc_end) {
                            if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                               DEBUG_SERIAL_ENUM.printf(" ERROR: HID optional descriptor entry %d header goes out of bounds. Stopping optional parse.\n", i);
                            }
                            break;
                        }
                        uint8_t optional_type = optional_desc_ptr[0];
                        uint16_t optional_len = optional_desc_ptr[1] | (optional_desc_ptr[2] << 8);

                        if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                           DEBUG_SERIAL_ENUM.printf("       Optional HID Desc %d: Type=0x%X (%s), Len=%u\n",
                                i, optional_type, descriptor_type_to_string(optional_type), optional_len);
                        }

                        if (optional_type == USB_DESC_TYPE_REPORT) {
                            if (currentInterface->reportDescriptorCount < MAX_HID_REPORT_DESC_PER_INTERFACE) {
                                UsbHidReportDescInfo& r = currentInterface->reportDescriptors[currentInterface->reportDescriptorCount];
                                r = UsbHidReportDescInfo();
                                r.bDescriptorType = optional_type;
                                r.wDescriptorLength = optional_len;
                                r.rawData = nullptr;

                                if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                                   DEBUG_SERIAL_ENUM.printf("         -> Stored Report Desc Info (Index %u)\n", currentInterface->reportDescriptorCount);
                                }
                                currentInterface->reportDescriptorCount++;
                            } else {
                                 if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                                    DEBUG_SERIAL_ENUM.printf(" WARN: Exceeded MAX_HID_REPORT_DESC_PER_INTERFACE (%u). Skipping optional report desc.\n", MAX_HID_REPORT_DESC_PER_INTERFACE);
                                 }
                            }
                        }
                        optional_desc_ptr += 3;
                    }
                }
                break;
            } // End scope

            case USB_DESCRIPTOR_INTERFACE_ASSOCIATION:
            { // Added scope
                if (dLen < sizeof(usb_interface_assoc_descriptor_t)) {
                     if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                        DEBUG_SERIAL_ENUM.println(" ERROR: IAD too short.");
                     }
                } else {
                    usb_interface_assoc_descriptor_t* iad = (usb_interface_assoc_descriptor_t*)ptr;
                    if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                       DEBUG_SERIAL_ENUM.printf(" -> IAD: FirstIface=%u, Count=%u, Class=%X, Sub=%X, Proto=%X, iFunc=%u\n",
                            iad->bFirstInterface, iad->bInterfaceCount, iad->bFunctionClass,
                            iad->bFunctionSubClass, iad->bFunctionProtocol, iad->iFunction);
                    }
                }
                break;
            } // End scope

            default:
            { // Added scope
                 if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                    DEBUG_SERIAL_ENUM.print("    -> Unknown/Unhandled Descriptor. Data (hex): ");
                    for(int i = 2; i < dLen; ++i) {
                        if (i >= 18) {
                             DEBUG_SERIAL_ENUM.print("...");
                             break;
                        }
                        DEBUG_SERIAL_ENUM.printf("%02X ", ptr[i]);
                    }
                    DEBUG_SERIAL_ENUM.println();
                 }
                 break;
            } // End scope
        } // End switch(dType)

        // Advance pointer to the next descriptor
        ptr += dLen;

    } // End while(ptr < end_ptr)

// Label for goto on fatal parse error
parse_end:;

    if (ptr > end_ptr) {
        if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
           DEBUG_SERIAL_ENUM.println(" WARN: Parser overran end of buffer.");
        }
    } else if (ptr < end_ptr) {
         if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
            DEBUG_SERIAL_ENUM.printf(" WARN: Parser finished with %ld bytes remaining.\n", (long)(end_ptr - ptr));
         }
    }

    if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
       DEBUG_SERIAL_ENUM.println("--- Finished parseFullConfigurationBlock ---");
    }
}

void DeviceEnumerator::storeString(char* buffer, size_t buffer_len, const Transfer_t* transfer) {
    // Basic validation
    if (!buffer || buffer_len == 0) {
        return;
    }
    buffer[0] = '\0'; // Ensure null termination always

    if (!transfer || transfer->length < 2) {
        if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
           DEBUG_SERIAL_ENUM.println("   storeString: Transfer null or too short.");
        }
        return;
    }

    const uint8_t* data = (const uint8_t*)temp_buffer_; // Assumes data is in temp_buffer_
    uint8_t total_len = data[0]; // bLength
    uint8_t desc_type = data[1]; // bDescriptorType

    if (desc_type != USB_DESCRIPTOR_STRING) {
        if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
           DEBUG_SERIAL_ENUM.printf("   storeString: Incorrect descriptor type (%u).\n", desc_type);
        }
        return;
    }

    if (total_len < 2) {
         if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
            DEBUG_SERIAL_ENUM.println("   storeString: Descriptor length < 2.");
         }
         return;
    }

    // Ensure total_len doesn't exceed actual received length
    if (total_len > transfer->length) {
        total_len = transfer->length;
    }

    size_t char_count = 0;
    // Start from index 2 (after bLength and bDescriptorType)
    // Iterate by 2 bytes (UTF-16 LE)
    for (uint8_t i = 2; i < total_len && char_count < (buffer_len - 1); i += 2) {
        // Check for simple ASCII (high byte is 0)
        if (i + 1 < total_len && data[i+1] == 0) {
            char ascii_char = (char)data[i];
            // Store printable chars, replace others with '?'
            buffer[char_count++] = isprint(ascii_char) ? ascii_char : '?';
        } else {
            // Non-ASCII or malformed, store '?'
            buffer[char_count++] = '?';
        }
    }
    // Null terminate the result string
    buffer[char_count] = '\0';

    if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
       DEBUG_SERIAL_ENUM.printf("   storeString: Stored '%s'\n", buffer);
    }
}

void DeviceEnumerator::printStoredData(Print& printer) const {
    // Check flag at the beginning of the function
    if (!ENABLE_LOGGING_DEVICE_ENUMERATOR) {
        return; // Don't print anything if logging is disabled
    }

    printer.println("\n=================================================");
    printer.println("======= Stored USB Device Enumeration Data =======");
    printer.println("=================================================");

    if (enum_state_ == STATE_ERROR) {
        printer.println("!!! WARNING: Enumeration ended in ERROR state. Data below might be incomplete or incorrect. !!!");
    } else if (enum_state_ != STATE_DONE) {
        printer.println("!!! WARNING: Enumeration not fully DONE. Data below might be incomplete. !!!");
    }

    // Check if basic VID/PID were stored
    if (stored_data_.idVendor == 0 && stored_data_.idProduct == 0) {
        printer.println("--- No Valid Device Data Stored ---");
        printer.println("=================================================");
        return;
    }

    printer.println("--- Device Info ---");
    printer.printf("  VID: 0x%04X, PID: 0x%04X, Address: %u\n", stored_data_.idVendor, stored_data_.idProduct, stored_data_.address);
    printer.print("  Speed: ");
    switch(stored_data_.speed) {
        case USB_SPEED_LOW:  printer.print("Low (1.5Mbps)"); break;
        case USB_SPEED_FULL: printer.print("Full (12Mbps)"); break;
        case USB_SPEED_HIGH: printer.print("High (480Mbps)"); break;
        default: printer.printf("Unknown (%u)", stored_data_.speed); break;
    }
    printer.println(); // End line for speed
    printer.printf("  Location: Hub %u, Port %u\n", stored_data_.hub_address, stored_data_.hub_port);
    printer.printf("  Class: 0x%02X, SubClass: 0x%02X, Protocol: 0x%02X (%s)\n",
        stored_data_.bDeviceClass, stored_data_.bDeviceSubClass, stored_data_.bDeviceProtocol,
        (stored_data_.bDeviceClass == 0 ? "(Defined at Interface level)" : "Device Specific"));
    printer.printf("  MaxPacketSize0: %u bytes\n", stored_data_.bMaxPacketSize0);
    // Print BCD values
    printer.printf("  bcdUSB: 0x%04X (%u.%u%u), ",
        stored_data_.bcdUSB, stored_data_.bcdUSB >> 8, (stored_data_.bcdUSB >> 4) & 0xF, stored_data_.bcdUSB & 0xF);
    printer.printf("bcdDevice: 0x%04X (%u.%u%u)\n",
        stored_data_.bcdDevice, stored_data_.bcdDevice >> 8, (stored_data_.bcdDevice >> 4) & 0xF, stored_data_.bcdDevice & 0xF);
    printer.printf("  NumConfigurations Reported: %u (Stored: %u)\n",
        stored_data_.bNumConfigurations, stored_data_.configurationCount);
    printer.printf("  String Index - Manufacturer: %u\n", stored_data_.iManufacturer);
    printer.printf("  String Index - Product:      %u\n", stored_data_.iProduct);
    printer.printf("  String Index - Serial #:     %u\n", stored_data_.iSerialNumber);
    printer.printf("  String Value - Manufacturer: '%s'\n", stored_data_.manufacturerString);
    printer.printf("  String Value - Product:      '%s'\n", stored_data_.productString);
    printer.printf("  String Value - Serial #:     '%s'\n", stored_data_.serialNumberString);

    // Loop through stored configurations (usually just one)
    for (uint8_t c = 0; c < stored_data_.configurationCount; ++c) {
        const UsbConfigurationData& config = stored_data_.configurations[c];
        printer.printf("\n--- Configuration %u Details ---\n", c);
        printer.printf("  bConfigurationValue: %u\n", config.bConfigurationValue);
        printer.printf("  iConfiguration (String Index): %u\n", config.iConfiguration);
        printer.printf("  Configuration String:        '%s'\n", config.configurationString);
        printer.printf("  bmAttributes:        0x%02X (", config.bmAttributes);
        if(config.bmAttributes & 0x40) { // Bit 6: Self-powered
            printer.print("SelfPowered");
        } else {
            printer.print("BusPowered");
        }
        if(config.bmAttributes & 0x20) { // Bit 5: Remote Wakeup
             printer.print(", RemoteWakeup");
        }
        printer.println(")"); // End attributes line
        printer.printf("  bMaxPower:           %u (%u mA)\n", config.bMaxPower, config.bMaxPower * 2);
        printer.printf("  wTotalLength (Reported by Desc): %u\n", config.wTotalLength);
        printer.printf("  Raw Config Data Length Stored: %u bytes\n", config.rawConfigLen);

        // Optional: Print raw config data dump
        if (config.rawConfigData && config.rawConfigLen > 0) {
            printer.println("  Raw Config Data Dump (hex):");
            printer.print("    ");
            for(uint16_t k=0; k < config.rawConfigLen; ++k) {
                if (k > 0 && k % 16 == 0) {
                    printer.print("\n    ");
                } else if (k > 0 && k % 8 == 0) {
                    printer.print(" "); // Extra space after 8 bytes
                }
                printer.printf("%02X ", config.rawConfigData[k]);
            }
            printer.println(); // End dump
        } else {
            printer.println("  Raw Config Data Dump: <Not Available or Empty>");
        }
        printer.printf("  Number of Interfaces Parsed: %u\n", config.interfaceCount);

        // Loop through parsed interfaces
        for (uint8_t i = 0; i < config.interfaceCount; ++i) {
            const UsbInterfaceData& iface = config.interfaces[i];
            printer.printf("\n  --- Interface %u (Array Index %u) ---\n", iface.bInterfaceNumber, i);
            printer.printf("    bAlternateSetting: %u\n", iface.bAlternateSetting);
            printer.printf("    bInterfaceClass:   0x%02X\n", iface.bInterfaceClass);
            printer.printf("    bInterfaceSubClass:0x%02X\n", iface.bInterfaceSubClass);
            printer.printf("    bInterfaceProtocol:0x%02X\n", iface.bInterfaceProtocol);
            printer.printf("    iInterface (String Index): %u\n", iface.iInterface);
            printer.printf("    Interface String:        '%s'\n", iface.interfaceString);
            printer.printf("    bNumEndpoints Reported:    %u (Stored: %u)\n", iface.bNumEndpoints, iface.endpointCount);
            printer.printf("    Is HID Interface?: %s\n", (iface.isHidInterface ? "Yes" : "No"));

            if (iface.isHidInterface) {
                printer.println("    --- HID Class Descriptor Info ---");
                printer.printf("      bcdHID:          0x%04X (%u.%u%u)\n",
                    iface.bcdHID, iface.bcdHID >> 8, (iface.bcdHID >> 4) & 0xF, iface.bcdHID & 0xF);
                printer.printf("      bCountryCode:    %u\n", iface.bCountryCode);
                printer.printf("      bNumDescriptors (Optional in HID Desc): %u\n", iface.bNumHidClassDescriptors);
                printer.printf("      Report Descriptors Found/Stored: %u\n", iface.reportDescriptorCount);

                // Loop through stored HID report descriptor info
                for (uint8_t r = 0; r < iface.reportDescriptorCount; ++r) {
                    const UsbHidReportDescInfo& report = iface.reportDescriptors[r];
                    printer.printf("      --- Stored Report Descriptor Info %u ---\n", r);
                    printer.printf("        bDescriptorType:   0x%02X (%s)\n", report.bDescriptorType, descriptor_type_to_string(report.bDescriptorType));
                    printer.printf("        wDescriptorLength: %u bytes\n", report.wDescriptorLength);
                    // Optional: Print raw report descriptor dump
                    if (report.rawData && report.wDescriptorLength > 0) {
                        printer.println("        Raw Report Descriptor Dump (hex):");
                        printer.print("          ");
                        for (uint16_t k = 0; k < report.wDescriptorLength; ++k) {
                            if (k > 0 && k % 16 == 0) {
                                printer.print("\n          ");
                            } else if (k > 0 && k % 8 == 0) {
                                printer.print(" ");
                            }
                            printer.printf("%02X ", report.rawData[k]);
                        }
                        printer.println(); // End dump
                    } else {
                        printer.println("        Raw Report Descriptor Dump: <Not Fetched, Empty, or Error>");
                    }
                } // End loop reports
            } // End if HID

            printer.println("    --- Endpoints Stored ---");
            if (iface.endpointCount == 0) {
                printer.println("      (None Found/Stored)");
            }
            // Loop through stored endpoints for this interface
            for (uint8_t e = 0; e < iface.endpointCount; ++e) {
                const UsbEndpointData& ep = iface.endpoints[e];
                uint16_t maxPkt = ep.wMaxPacketSize & 0x7FF; // Mask out transaction/mult bits
                // High-speed specific transaction multiplier bits
                uint8_t multi = (stored_data_.speed == USB_SPEED_HIGH) ? ((ep.wMaxPacketSize >> 11) & 0x03) : 0;

                printer.printf("      Endpoint %u: Addr=0x%02X(%s), Attr=0x%02X(",
                    e, ep.bEndpointAddress, ((ep.bEndpointAddress & 0x80) ? "IN" : "OUT"), ep.bmAttributes);
                print_endpoint_attributes(printer, ep.bmAttributes); // Use helper
                printer.printf("), MaxPkt=%u", maxPkt);
                // Print transaction multiplier if relevant (High Speed, Int/Iso)
                if (multi > 0 && (ep.bmAttributes & 0x03) != 0 && (ep.bmAttributes & 0x03) != 2) { // Check type not Bulk/Control
                     printer.printf(" x%u", multi + 1);
                }
                printer.printf(", Interval=%u", ep.bInterval);
                 if (ep.bSynchAddress != 0) { // Print Sync Address if Iso and non-zero
                    printer.printf(", SyncAddr=%u", ep.bSynchAddress);
                 }
                 printer.println(); // End endpoint line

                // Print interval interpretation
                printer.print("        Interval Interpretation: ");
                print_endpoint_interval(printer, ep.bInterval, stored_data_.speed, ep.bmAttributes); // Use helper
                printer.println(); // End interval interpretation line
            } // End loop endpoints
        } // End loop interfaces
    } // End loop configurations

    printer.println("\n=================================================");
    printer.println("======= End of Stored Data Printout =======");
    printer.println("=================================================");
}