#include "DeviceEnumerator.h"

// Define DEBUG_SERIAL here or pass it into functions that need it.
// Using a global define is simpler for this example.
#define DEBUG_SERIAL Serial

// ==========================================================================
// Helper Function Implementations
// ==========================================================================

const char* descriptor_type_to_string(uint8_t type) {
    switch (type) {
        // ... (Keep the content of this function as is) ...
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

void print_endpoint_attributes(Print& printer, uint8_t attr) {
    // ... (Keep the content of this function as is) ...
    uint8_t transfer_type = attr & 0x03;
    switch (transfer_type) {
        case 0: printer.print("Control"); break;
        case 1: printer.print("Isochronous"); break;
        case 2: printer.print("Bulk"); break;
        case 3: printer.print("Interrupt"); break;
    }
}

void print_endpoint_interval(Print& printer, uint8_t interval, uint8_t speed, uint8_t attributes) {
    // ... (Keep the content of this function as is) ...
    uint8_t transfer_type = attributes & 0x03;
    if (transfer_type != 1 && transfer_type != 3) { printer.printf("%u (N/A)", interval); return; }
    if (speed == USB_SPEED_HIGH) { if (interval == 0) interval = 1; if (interval > 16) interval = 16; float mf = pow(2.0f, (float)interval - 1.0f); float pus = mf * 125.0f; if (pus < 1.0f) { printer.printf("%u (%.3f us / %.1f MHz)", interval, pus, 1.0f / (pus / 1000000.0f) / 1000000.0f ); } else if (pus < 1000.0f) { printer.printf("%u (%.1f us / %.1f KHz)", interval, pus, 1000000.0f / pus / 1000.0f); } else { printer.printf("%u (%.1f ms / %.1f Hz)", interval, pus / 1000.0f, 1000000.0f / pus); } }
    else { if (transfer_type == 1) { printer.printf("%u (1 ms frame fixed)", interval); } else { if (interval == 0) interval = 1; printer.printf("%u (%u ms / %u Hz)", interval, interval, 1000 / interval); } }
}


// ==========================================================================
// DeviceEnumerator Class Method Implementations
// ==========================================================================

// *** FIXED CONSTRUCTOR ***
DeviceEnumerator::DeviceEnumerator(USBHost& host) : USBDriver() { // Call default base constructor
    init();
}

void DeviceEnumerator::init() {
    clearStoredDeviceData();
    driver_ready_for_device(this);
}

bool DeviceEnumerator::claim(Device_t* device, int type, const uint8_t* descriptors, uint32_t len) {
    // ... (Keep the content of this function as is - no changes needed here) ...
     if (type != 0 || device == nullptr) { DEBUG_SERIAL.printf("DeviceEnumerator::claim: Ignoring claim for type %d or null device.\n", type); return false; }
     if (enum_state_ != STATE_IDLE) { if (current_device_ == device) { DEBUG_SERIAL.println("DeviceEnumerator::claim: Already processing this device."); return false; } else if (current_device_ != nullptr) { DEBUG_SERIAL.println("!!! DeviceEnumerator::claim: Busy with another device, ignoring new claim."); return false; } }
     DEBUG_SERIAL.printf("DeviceEnumerator::claim: Claiming device VID:0x%04X PID:0x%04X (Type %d)\n", device->idVendor, device->idProduct, type);
     startEnumeration(device); return false;
}

void DeviceEnumerator::disconnect() {
    // ... (Keep the content of this function as is - no changes needed here) ...
    uint16_t vid = 0, pid = 0; if (current_device_) { vid = current_device_->idVendor; pid = current_device_->idProduct; DEBUG_SERIAL.printf("\n--- DeviceEnumerator: Device VID:0x%04X PID:0x%04X Disconnected ---\n", vid, pid); } else { vid = stored_data_.idVendor; pid = stored_data_.idProduct; if (vid != 0 || pid != 0) { DEBUG_SERIAL.printf("\n--- DeviceEnumerator: Device VID:0x%04X PID:0x%04X Disconnected (from stored) ---\n", vid, pid); } else { DEBUG_SERIAL.println("\n--- DeviceEnumerator: Unknown Device Disconnected ---"); } }
    init(); DEBUG_SERIAL.printf("DeviceEnumerator::disconnect: Cleared data for VID:0x%04X PID:0x%04X and reset state.\n", vid, pid);
}

void DeviceEnumerator::startEnumeration(Device_t* device) {
    // ... (Keep the content of this function as is - no changes needed here) ...
    DEBUG_SERIAL.println("DeviceEnumerator::startEnumeration"); clearStoredDeviceData(); current_device_ = device;
    if (!current_device_) { DEBUG_SERIAL.println("!!! ERROR: startEnumeration called with NULL device pointer!"); enum_state_ = STATE_ERROR; return; }
    stored_data_.idVendor = device->idVendor; stored_data_.idProduct = device->idProduct; stored_data_.speed = device->speed; stored_data_.hub_address = device->hub_address; stored_data_.hub_port = device->hub_port; stored_data_.address = device->address;
    DEBUG_SERIAL.printf("  Starting enumeration for: VID=0x%04X, PID=0x%04X, Addr=%u\n", stored_data_.idVendor, stored_data_.idProduct, stored_data_.address); enum_state_ = STATE_GETTING_DEVICE_DESC_8; processStateMachine();
}

void DeviceEnumerator::control(const Transfer_t* transfer) {
    // ... (Keep the content of this function as is - no changes needed here) ...
    DEBUG_SERIAL.printf("DeviceEnumerator::control - START - Current State: %d\n", enum_state_); if (!transfer) { DEBUG_SERIAL.println("  control() called with NULL transfer! Aborting."); return; } if (transfer->driver != this) { DEBUG_SERIAL.println("  control() transfer not for this driver instance. Ignoring."); return; } if (current_device_ == nullptr) { DEBUG_SERIAL.println("  control() called but current_device_ is NULL! (Device likely disconnected). Ignoring transfer."); return; }
    uint32_t token = transfer->qtd.token; DEBUG_SERIAL.printf("  Transfer details: Len=%u, Token=0x%X\n", transfer->length, token); if ((token & 0x68) != 0) { DEBUG_SERIAL.printf("!!! FDE::Control USB Error Token:0x%X (Babble/Buffer/Transaction)\n", token); enum_state_ = STATE_ERROR; processStateMachine(); return; } bool is_stall = (token & 0x40) != 0; if (is_stall) { DEBUG_SERIAL.printf("  STALL received. Current state: %d\n", enum_state_); if (enum_state_ != STATE_GETTING_REPORT_DESC) { DEBUG_SERIAL.println("!!! FDE::Control STALL in unexpected state. Setting ERROR state."); enum_state_ = STATE_ERROR; processStateMachine(); return; } }
    DEBUG_SERIAL.println("  Control transfer seems OK or STALL handled by state. Calling processStateMachine..."); processStateMachine(transfer); DEBUG_SERIAL.printf("DeviceEnumerator::control - END - New State: %d\n", enum_state_);
}

void DeviceEnumerator::Task() {
    // Check for disconnection first
    if (current_device_ == nullptr && enum_state_ != STATE_IDLE) {
        DEBUG_SERIAL.println("DeviceEnumerator::Task - Device missing but not IDLE. Resetting state.");
        enum_state_ = STATE_IDLE; // Reset state immediately if device ptr is null
        // Data cleanup will happen on next claim or disconnect event
        return;
    }

    // Advance state machine ONLY for states that progress without a direct USB callback
    // Examples: Parsing data that was previously stored, deciding to skip fetching a string.
    // *** CRITICAL: DO NOT call processStateMachine here for states waiting on control() ***
    // Specifically removed STATE_GETTING_REPORT_DESC from this check.
    if (enum_state_ == STATE_PARSING_CONFIG ||
       (enum_state_ == STATE_GETTING_CONFIG_STRING && current_device_ != nullptr) ||
       (enum_state_ == STATE_GETTING_INTERFACE_STRINGS && current_device_ != nullptr)
       )
    {
        DEBUG_SERIAL.printf("DeviceEnumerator::Task - Advancing state %d (non-USB callback trigger)\n", enum_state_);
        processStateMachine(); // Call state machine for non-USB driven transitions
    }
    // For all other states (like STATE_GETTING_REPORT_DESC, STATE_GETTING_DEVICE_DESC_8, etc.),
    // the state machine is advanced ONLY by the control() callback. Task() does nothing for them.
}

const UsbDeviceData* DeviceEnumerator::getStoredDeviceData() const {
    // ... (Keep the content of this function as is - no changes needed here) ...
    if (enum_state_ == STATE_DONE || enum_state_ == STATE_ERROR) { return &stored_data_; } return nullptr;
}

Device_t* DeviceEnumerator::getCurrentDevice() const {
    // ... (Keep the content of this function as is - no changes needed here) ...
    return current_device_;
}

bool DeviceEnumerator::isEnumerationDone() const {
    // ... (Keep the content of this function as is - no changes needed here) ...
    return enum_state_ == STATE_DONE;
}

bool DeviceEnumerator::isErrorState() const {
    // ... (Keep the content of this function as is - no changes needed here) ...
    return enum_state_ == STATE_ERROR;
}


void DeviceEnumerator::clearStoredDeviceData() {
    // ... (Keep the content of this function as is - no changes needed here) ...
     DEBUG_SERIAL.println("DeviceEnumerator::clearStoredDeviceData"); for (uint8_t c = 0; c < MAX_CONFIGURATIONS_PER_DEVICE; ++c) { UsbConfigurationData& cfg = stored_data_.configurations[c]; if (cfg.rawConfigData) { delete[] cfg.rawConfigData; cfg.rawConfigData = nullptr; } for (uint8_t i = 0; i < MAX_INTERFACES_PER_CONFIG; ++i) { UsbInterfaceData& iface = cfg.interfaces[i]; for (uint8_t r = 0; r < MAX_HID_REPORT_DESC_PER_INTERFACE; ++r) { UsbHidReportDescInfo& report = iface.reportDescriptors[r]; if (report.rawData) { delete[] report.rawData; report.rawData = nullptr; } } } }
     stored_data_ = UsbDeviceData(); enum_state_ = STATE_IDLE; current_device_ = nullptr; config_total_len_ = 0; current_config_index_ = 0; current_interface_index_ = 0; current_hid_report_index_ = 0; DEBUG_SERIAL.println("  Finished clearing data and resetting state.");
}

// --- CORRECTED processStateMachine Function (with scope fixes and min fix) ---
void DeviceEnumerator::processStateMachine(const Transfer_t* transfer) {
    bool repeat = false;
    int loop_guard = 0;

    DEBUG_SERIAL.printf(" >> processStateMachine ENTRY: State=%d (Transfer %s)\n", enum_state_, (transfer ? "provided" : "not provided"));

    do {
        repeat = false;
        if (loop_guard++ > 30) { // Increased guard slightly
            DEBUG_SERIAL.println("!!! State machine loop guard triggered! Forcing ERROR state.");
            enum_state_ = STATE_ERROR; break;
        }
        if (!current_device_ && enum_state_ != STATE_IDLE) {
            DEBUG_SERIAL.printf(" >> processStateMachine: Device disconnected during state %d processing. Aborting.\n", enum_state_); break;
        }

        DEBUG_SERIAL.printf(" >> processStateMachine LOOP (Guard %d): State=%d (Transfer %s)\n", loop_guard, enum_state_, (transfer ? "provided" : "not provided"));

        switch (enum_state_) {
            case STATE_IDLE: { // Added scope
                break; // Wait for claim
            }
            case STATE_GETTING_DEVICE_DESC_8: { // Added scope
                DEBUG_SERIAL.println(" State: Requesting DevDesc(8)...");
                queueGetDescriptor(USB_DESCRIPTOR_DEVICE, 0, 0, 8);
                enum_state_ = STATE_GETTING_DEVICE_DESC_FULL;
                break; // Wait
            }
            case STATE_GETTING_DEVICE_DESC_FULL: { // Added scope
                if (!transfer) { DEBUG_SERIAL.println(" State: GETTING_DEVICE_DESC_FULL waiting..."); break; }
                DEBUG_SERIAL.println(" State: Processing DevDesc(8), Requesting DevDesc(18)...");
                if (transfer->length < 8 || temp_buffer_[0] < 8 || temp_buffer_[1] != USB_DESCRIPTOR_DEVICE) { DEBUG_SERIAL.printf(" ERROR: Invalid partial DevDesc. Len=%u, bL=%u, bT=%u\n", transfer->length, temp_buffer_[0], temp_buffer_[1]); enum_state_ = STATE_ERROR; repeat = true; break; }
                stored_data_.bMaxPacketSize0 = temp_buffer_[7]; DEBUG_SERIAL.printf("   Got MaxPacketSize0 = %u\n", stored_data_.bMaxPacketSize0);
                queueGetDescriptor(USB_DESCRIPTOR_DEVICE, 0, 0, sizeof(usb_device_descriptor_t));
                enum_state_ = STATE_GETTING_STRING_MAN;
                break; // Wait
            }
            case STATE_GETTING_STRING_MAN: { // Added scope
                 if (!transfer) { DEBUG_SERIAL.println(" State: GETTING_STRING_MAN waiting..."); break; }
                 DEBUG_SERIAL.println(" State: Processing Full DevDesc...");
                 if (transfer->length < sizeof(usb_device_descriptor_t) || temp_buffer_[1] != USB_DESCRIPTOR_DEVICE) { DEBUG_SERIAL.printf(" ERROR: Short/Invalid Full DevDesc (Len=%u, Type=%u).\n", transfer->length, temp_buffer_[1]); enum_state_ = STATE_ERROR; repeat = true; break; }
                 { // Inner scope for 'd' is still good practice
                    usb_device_descriptor_t* d = (usb_device_descriptor_t*)temp_buffer_;
                    stored_data_.bcdUSB = d->bcdUSB; stored_data_.bDeviceClass = d->bDeviceClass; stored_data_.bDeviceSubClass = d->bDeviceSubClass;
                    stored_data_.bDeviceProtocol = d->bDeviceProtocol; stored_data_.bcdDevice = d->bcdDevice; stored_data_.iManufacturer = d->iManufacturer;
                    stored_data_.iProduct = d->iProduct; stored_data_.iSerialNumber = d->iSerialNumber; stored_data_.bNumConfigurations = d->bNumConfigurations;
                    if(current_device_) { stored_data_.address = current_device_->address; }
                 }
                 DEBUG_SERIAL.printf("   DevDesc OK: Addr=%u, Class=%X, #Cfg=%u, iM=%u, iP=%u, iS=%u\n", stored_data_.address, stored_data_.bDeviceClass, stored_data_.bNumConfigurations, stored_data_.iManufacturer, stored_data_.iProduct, stored_data_.iSerialNumber);
                 if (stored_data_.iManufacturer) { DEBUG_SERIAL.println(" State: Requesting Manuf String..."); queueGetDescriptor(USB_DESCRIPTOR_STRING, stored_data_.iManufacturer, 0x0409, MAX_STRING_LENGTH * 2 + 2); enum_state_ = STATE_GETTING_STRING_PROD; }
                 else { DEBUG_SERIAL.println(" State: Skipping Manuf String."); enum_state_ = STATE_GETTING_STRING_PROD; repeat = true; }
                 break; // Wait or repeat
            }
            case STATE_GETTING_STRING_PROD: { // Added scope
                 if (transfer) { storeString(stored_data_.manufacturerString, MAX_STRING_LENGTH, transfer); } else { DEBUG_SERIAL.println(" State: GETTING_STRING_PROD entered."); }
                 if (stored_data_.iProduct) { DEBUG_SERIAL.println(" State: Requesting Prod String..."); queueGetDescriptor(USB_DESCRIPTOR_STRING, stored_data_.iProduct, 0x0409, MAX_STRING_LENGTH * 2 + 2); enum_state_ = STATE_GETTING_STRING_SERIAL; }
                 else { DEBUG_SERIAL.println(" State: Skipping Prod String."); enum_state_ = STATE_GETTING_STRING_SERIAL; repeat = true; }
                 break; // Wait or repeat
            }
            case STATE_GETTING_STRING_SERIAL: { // Added scope
                 if (transfer) { storeString(stored_data_.productString, MAX_STRING_LENGTH, transfer); } else { DEBUG_SERIAL.println(" State: GETTING_STRING_SERIAL entered."); }
                 if (stored_data_.iSerialNumber) { DEBUG_SERIAL.println(" State: Requesting Serial String..."); queueGetDescriptor(USB_DESCRIPTOR_STRING, stored_data_.iSerialNumber, 0x0409, MAX_STRING_LENGTH * 2 + 2); enum_state_ = STATE_GETTING_CONFIG_HEADER; }
                 else { DEBUG_SERIAL.println(" State: Skipping Serial String."); enum_state_ = STATE_GETTING_CONFIG_HEADER; repeat = true; }
                 break; // Wait or repeat
            }
            case STATE_GETTING_CONFIG_HEADER: { // Added scope
                 if (transfer) { storeString(stored_data_.serialNumberString, MAX_STRING_LENGTH, transfer); } else { DEBUG_SERIAL.println(" State: GETTING_CONFIG_HEADER entered."); }
                 if (stored_data_.bNumConfigurations == 0) { DEBUG_SERIAL.println(" WARN: Device reports 0 configurations. DONE."); enum_state_ = STATE_DONE; repeat = true; break; }
                 DEBUG_SERIAL.println(" State: Requesting Config Header(9 bytes)..."); queueGetDescriptor(USB_DESCRIPTOR_CONFIGURATION, 0, 0, sizeof(usb_configuration_descriptor_t));
                 enum_state_ = STATE_GETTING_CONFIG_FULL;
                 break; // Wait
            }
            case STATE_GETTING_CONFIG_FULL: { // Added scope
                 if (!transfer) { DEBUG_SERIAL.println(" State: GETTING_CONFIG_FULL waiting..."); break; }
                 DEBUG_SERIAL.println(" State: Processing Config Header...");
                 if (transfer->length < sizeof(usb_configuration_descriptor_t) || temp_buffer_[1] != USB_DESCRIPTOR_CONFIGURATION) { DEBUG_SERIAL.printf(" ERROR: Invalid Config Header. Len=%u, Type=%u\n", transfer->length, temp_buffer_[1]); enum_state_ = STATE_ERROR; repeat = true; break; }
                 usb_configuration_descriptor_t* c = (usb_configuration_descriptor_t*)temp_buffer_;
                 config_total_len_ = c->wTotalLength; DEBUG_SERIAL.printf("   Config Header OK: wTotalLength=%u, #Intf=%u, CfgVal=%u\n", config_total_len_, c->bNumInterfaces, c->bConfigurationValue);
                 if (config_total_len_ < sizeof(usb_configuration_descriptor_t) || config_total_len_ == 0) { DEBUG_SERIAL.println(" ERROR: Invalid/Zero wTotalLength."); enum_state_ = STATE_ERROR; repeat = true; break; }
                 uint16_t request_len = min(config_total_len_, (uint16_t)TEMP_BUFFER_SIZE); // Use standard 2-arg min
                 DEBUG_SERIAL.printf(" State: Requesting Full Config (Req %u bytes, total %u)...\n", request_len, config_total_len_);
                 queueGetDescriptor(USB_DESCRIPTOR_CONFIGURATION, 0, 0, request_len);
                 enum_state_ = STATE_PARSING_CONFIG;
                 break; // Wait
            }
            case STATE_PARSING_CONFIG: { // Added scope
                 if (!transfer) { if (stored_data_.configurationCount > 0 && stored_data_.configurations[0].rawConfigData) { DEBUG_SERIAL.println(" State: PARSING_CONFIG (via Task) - Parsing stored data."); parseFullConfigurationBlock(); if (enum_state_ != STATE_ERROR) { current_config_index_ = 0; current_interface_index_ = 0; enum_state_ = STATE_GETTING_CONFIG_STRING; repeat = true; } } else { DEBUG_SERIAL.println(" State: PARSING_CONFIG waiting for transfer..."); } }
                 else { DEBUG_SERIAL.println(" State: PARSING_CONFIG (via control) - Storing Full Config Data..."); uint16_t actual_recv_len = (uint16_t)transfer->length; uint32_t tk = transfer->qtd.token;
                        if ((tk & 0x68) != 0 || (actual_recv_len == 0 && (tk & 0x40)) || (actual_recv_len == 0 && !(tk & 0x40))) { DEBUG_SERIAL.printf("!!! ERROR: USB Error/STALL/Zero Length (0x%X, %uB) on GetFullConfig.\n", tk, actual_recv_len); enum_state_ = STATE_ERROR; repeat = true; break; }
                        // *** FIXED min CALL ***
                        uint16_t len_to_store = min(config_total_len_, min(actual_recv_len, (uint16_t)TEMP_BUFFER_SIZE));
                        DEBUG_SERIAL.printf("   Received %u bytes. Storing %u bytes.\n", actual_recv_len, len_to_store);
                        if (len_to_store > 0) { if (stored_data_.configurationCount > 0 && stored_data_.configurations[0].rawConfigData) { delete[] stored_data_.configurations[0].rawConfigData; stored_data_.configurations[0].rawConfigData = nullptr; } stored_data_.configurationCount = 0; UsbConfigurationData& cfg = stored_data_.configurations[0]; cfg = UsbConfigurationData(); cfg.rawConfigData = new (std::nothrow) uint8_t[len_to_store];
                            if (cfg.rawConfigData) { memcpy(cfg.rawConfigData, temp_buffer_, len_to_store); cfg.rawConfigLen = len_to_store; stored_data_.configurationCount = 1; DEBUG_SERIAL.println("   Stored raw config successfully."); }
                            else { DEBUG_SERIAL.println("!!! ERROR: Malloc failed for raw config."); enum_state_ = STATE_ERROR; repeat = true; }
                        } else { DEBUG_SERIAL.println("!!! ERROR: Zero length to store for config."); enum_state_ = STATE_ERROR; repeat = true; }
                 }
                 break; // Wait for Task() or repeat on error
            }
            case STATE_GETTING_CONFIG_STRING: { // Added scope
                 if (transfer) { DEBUG_SERIAL.println(" WARN: STATE_GETTING_CONFIG_STRING unexpected transfer."); break; }
                 DEBUG_SERIAL.println(" State: Checking for Config String...");
                 if (stored_data_.configurationCount > 0 && stored_data_.configurations[0].iConfiguration > 0) {
                     uint8_t string_index = stored_data_.configurations[0].iConfiguration;
                     DEBUG_SERIAL.printf(" State: Requesting Config String (idx %u)...\n", string_index);
                     queueGetDescriptor(USB_DESCRIPTOR_STRING, string_index, 0x0409, MAX_STRING_LENGTH * 2 + 2);
                     enum_state_ = STATE_GETTING_INTERFACE_STRINGS; current_interface_index_ = 0;
                 } else {
                     DEBUG_SERIAL.println(" State: No Config String to fetch.");
                     enum_state_ = STATE_GETTING_INTERFACE_STRINGS; current_interface_index_ = 0; repeat = true;
                 }
                 break; // Wait or repeat
            }
            case STATE_GETTING_INTERFACE_STRINGS: { // Added scope
                 if (transfer) { /* ... process string ... */
                     DEBUG_SERIAL.println(" State: Processing received string...");
                     if (current_interface_index_ == 0 && stored_data_.configurationCount > 0 && stored_data_.configurations[0].iConfiguration > 0) { storeString(stored_data_.configurations[0].configurationString, MAX_STRING_LENGTH, transfer); }
                     else if (current_interface_index_ > 0 && stored_data_.configurationCount > 0 && (current_interface_index_ - 1) < stored_data_.configurations[0].interfaceCount) { if (stored_data_.configurations[0].interfaces[current_interface_index_ - 1].iInterface > 0) { storeString(stored_data_.configurations[0].interfaces[current_interface_index_ - 1].interfaceString, MAX_STRING_LENGTH, transfer); } else { DEBUG_SERIAL.println(" WARN: Received string, but previous interface index had iInterface=0."); } }
                     else { DEBUG_SERIAL.println(" WARN: Received string, context unclear."); }
                 } else { DEBUG_SERIAL.println(" State: GETTING_INTERFACE_STRINGS entered (no transfer)."); }
                 // Search for next string
                 bool request_queued = false;
                 if (stored_data_.configurationCount > 0) {
                     UsbConfigurationData& config = stored_data_.configurations[0];
                     while (current_interface_index_ < config.interfaceCount) {
                         UsbInterfaceData& iface = config.interfaces[current_interface_index_];
                         if (iface.iInterface > 0) {
                             DEBUG_SERIAL.printf(" State: Requesting Interface String (iface #%u, idx=%u)...\n", iface.bInterfaceNumber, iface.iInterface);
                             queueGetDescriptor(USB_DESCRIPTOR_STRING, iface.iInterface, 0x0409, MAX_STRING_LENGTH * 2 + 2);
                             current_interface_index_++; request_queued = true; break;
                         }
                         current_interface_index_++;
                     }
                 }
                 if (!request_queued) { DEBUG_SERIAL.println(" State: Done fetching Interface Strings."); enum_state_ = STATE_GETTING_REPORT_DESC; current_config_index_ = 0; current_interface_index_ = 0; current_hid_report_index_ = 0; repeat = true; }
                 break; // Wait or repeat
            }
            case STATE_GETTING_REPORT_DESC: { // Added scope
                bool processed_transfer = false;
                if (transfer) { /* ... process transfer ... */
                     DEBUG_SERIAL.println(" State: GETTING_REPORT_DESC processing received transfer..."); processed_transfer = true; bool found_pending = false; uint8_t completed_iface_idx = 0; uint8_t completed_report_idx = 0;
                     if (stored_data_.configurationCount > 0 && current_config_index_ < stored_data_.configurationCount) { UsbConfigurationData& c = stored_data_.configurations[current_config_index_]; for(uint8_t ii = 0; ii < c.interfaceCount && !found_pending; ++ii) { UsbInterfaceData& i = c.interfaces[ii]; if(i.isHidInterface) { for(uint8_t rr = 0; rr < i.reportDescriptorCount; ++rr) { UsbHidReportDescInfo& r = i.reportDescriptors[rr]; if (r.wDescriptorLength > 0 && r.rawData == nullptr) { completed_iface_idx = ii; completed_report_idx = rr; found_pending = true; break; } } } } }
                     if (found_pending) { DEBUG_SERIAL.printf("   Transfer likely corresponds to Iface %u, Report Idx %d.\n", stored_data_.configurations[current_config_index_].interfaces[completed_iface_idx].bInterfaceNumber, completed_report_idx); UsbInterfaceData& i = stored_data_.configurations[current_config_index_].interfaces[completed_iface_idx]; UsbHidReportDescInfo& r = i.reportDescriptors[completed_report_idx]; uint16_t actual_recv_len = (uint16_t)transfer->length; uint32_t tk = transfer->qtd.token; if (r.rawData) { delete[] r.rawData; r.rawData = nullptr; }
                         if (actual_recv_len == 0 && (tk & 0x40)) { DEBUG_SERIAL.printf("   STALL received. Marking Report Desc Iface %u, Idx %d as unavailable.\n", i.bInterfaceNumber, completed_report_idx); r.wDescriptorLength = 0; r.rawData = nullptr; }
                         else if (actual_recv_len > 0) { uint16_t len_to_store = min(r.wDescriptorLength, actual_recv_len); r.rawData = new (std::nothrow) uint8_t[len_to_store]; if (r.rawData) { memcpy(r.rawData, temp_buffer_, len_to_store); r.wDescriptorLength = len_to_store; DEBUG_SERIAL.printf("   Stored Report Desc Iface %u Idx %d (%u bytes)\n", i.bInterfaceNumber, completed_report_idx, len_to_store); } else { DEBUG_SERIAL.println("!!! ERROR: Malloc failed for report desc!"); r.wDescriptorLength = 0; r.rawData = nullptr; } }
                         else { DEBUG_SERIAL.printf(" WARN: Received 0 bytes (not STALL) for Report Desc Iface %u Idx %d.\n", i.bInterfaceNumber, completed_report_idx); r.wDescriptorLength = 0; r.rawData = nullptr; }
                     } else { DEBUG_SERIAL.println(" WARN: Received transfer in GETTING_REPORT_DESC, but no pending report found."); processed_transfer = false; }
                }
                // Search for next request
                DEBUG_SERIAL.println(" State: Searching for next Report Desc to fetch..."); bool request_queued = false;
                if (stored_data_.configurationCount > 0 && current_config_index_ < stored_data_.configurationCount) {
                    UsbConfigurationData& c = stored_data_.configurations[current_config_index_];
                    for (; current_interface_index_ < c.interfaceCount; ++current_interface_index_) {
                        UsbInterfaceData& i = c.interfaces[current_interface_index_];
                        if (i.isHidInterface) {
                            for (; current_hid_report_index_ < i.reportDescriptorCount; ++current_hid_report_index_) {
                                UsbHidReportDescInfo& r = i.reportDescriptors[current_hid_report_index_];
                                if (r.wDescriptorLength > 0 && r.rawData == nullptr) {
                                    DEBUG_SERIAL.printf("   Found pending: Iface #%u (Idx %u), Report Idx %d. Queueing request...\n", i.bInterfaceNumber, current_interface_index_, current_hid_report_index_);
                                    queueGetHidReportDescriptor(current_config_index_, current_interface_index_, current_hid_report_index_);
                                    request_queued = true; goto report_search_end; // Use goto to break nested loops
                                }
                            }
                        }
                        current_hid_report_index_ = 0; // Reset report index for next interface
                    }
                }
            report_search_end:; // Label for goto
                if (!request_queued) { DEBUG_SERIAL.println(" State: No more Report Descriptors to fetch. Enumeration DONE."); enum_state_ = STATE_DONE; repeat = true; }
                break; // Wait or repeat if DONE
            }
            case STATE_DONE: { // Added scope
                DEBUG_SERIAL.println(" State: Processing DONE state.");
                repeat = false; // Stay DONE
                break;
            }
            case STATE_ERROR: { // Added scope
                DEBUG_SERIAL.println("!!! State: Processing ERROR state.");
                repeat = false; // Stay ERROR
                break;
            }
            default: { // Added scope
                DEBUG_SERIAL.printf("!!! State: UNKNOWN state (%d)! Halting.\n", enum_state_);
                enum_state_ = STATE_ERROR;
                repeat = false;
                break;
            }
        } // end switch
    } while (repeat);

    DEBUG_SERIAL.printf(" << processStateMachine EXIT: State=%d\n", enum_state_);
}


void DeviceEnumerator::queueGetDescriptor(uint8_t desc_type, uint8_t desc_index, uint16_t lang_id, uint16_t len) {
    // ... (Keep the content of this function as is - no changes needed here) ...
    if (!current_device_) { DEBUG_SERIAL.println("!!! queueGetDescriptor: No current device!"); return; } if (len == 0) { DEBUG_SERIAL.println(" WARN: queueGetDescriptor called with length 0. Aborting queue."); return; } uint16_t request_len = len; if (request_len > TEMP_BUFFER_SIZE) { request_len = TEMP_BUFFER_SIZE; } DEBUG_SERIAL.printf("  Queueing GetDescriptor: Type=%u (%s), Idx=%u, Lang=0x%X, ReqLen=%u (OrigLen=%u)\n", desc_type, descriptor_type_to_string(desc_type), desc_index, lang_id, request_len, len);
    mk_setup(setup_packet_, 0x80, USB_REQUEST_GET_DESCRIPTOR, (desc_type << 8) | desc_index, lang_id, request_len); if (!queue_Control_Transfer(current_device_, &setup_packet_, temp_buffer_, this)) { DEBUG_SERIAL.println("!!! ERROR: queue_Control_Transfer failed! Setting ERROR state."); enum_state_ = STATE_ERROR; }
}

void DeviceEnumerator::queueGetHidReportDescriptor(uint8_t config_idx, uint8_t iface_idx, uint8_t report_idx) {
    // ... (Keep the content of this function as is - no changes needed here) ...
    if (!current_device_) { DEBUG_SERIAL.println("!!! queueGetHidReportDescriptor: No current device!"); return; } if (config_idx >= stored_data_.configurationCount || config_idx >= MAX_CONFIGURATIONS_PER_DEVICE) { DEBUG_SERIAL.printf("!!! queueGetHidReportDescriptor: Invalid config index %u\n", config_idx); return; } UsbConfigurationData& c = stored_data_.configurations[config_idx]; if (iface_idx >= c.interfaceCount || iface_idx >= MAX_INTERFACES_PER_CONFIG) { DEBUG_SERIAL.printf("!!! queueGetHidReportDescriptor: Invalid interface index %u\n", iface_idx); return; } UsbInterfaceData& i = c.interfaces[iface_idx]; if (report_idx >= i.reportDescriptorCount || report_idx >= MAX_HID_REPORT_DESC_PER_INTERFACE) { DEBUG_SERIAL.printf("!!! queueGetHidReportDescriptor: Invalid report index %u\n", report_idx); return; } UsbHidReportDescInfo& r = i.reportDescriptors[report_idx]; if (r.wDescriptorLength == 0) { DEBUG_SERIAL.printf(" WARN: Skipping GetHidReportDescriptor for Iface %u Idx %d - stored length is 0.\n", i.bInterfaceNumber, report_idx); r.rawData = nullptr; Task(); return; } uint16_t len_to_request = min(r.wDescriptorLength, (uint16_t)TEMP_BUFFER_SIZE); DEBUG_SERIAL.printf("  Queueing GetHidReportDescriptor: Iface=%u, ReportIdx=%d, ReqLen=%u (TotalLen=%u)\n", i.bInterfaceNumber, report_idx, len_to_request, r.wDescriptorLength); mk_setup(setup_packet_, 0x81, USB_REQUEST_GET_DESCRIPTOR, (USB_DESC_TYPE_REPORT << 8) | 0, i.bInterfaceNumber, len_to_request); if (!queue_Control_Transfer(current_device_, &setup_packet_, temp_buffer_, this)) { DEBUG_SERIAL.println("!!! ERROR: queue_Control_Transfer failed for HID Report Desc! Setting ERROR state."); enum_state_ = STATE_ERROR; }
}

void DeviceEnumerator::parseFullConfigurationBlock() {
    // ... (Keep the content of this function as is - no changes needed here) ...
    DEBUG_SERIAL.println("\n--- Starting parseFullConfigurationBlock ---"); if (stored_data_.configurationCount == 0 || !stored_data_.configurations[0].rawConfigData) { DEBUG_SERIAL.println(" ERROR: No raw config data available to parse."); enum_state_ = STATE_ERROR; return; } UsbConfigurationData& config = stored_data_.configurations[0]; const uint8_t* buffer = config.rawConfigData; uint16_t len = config.rawConfigLen; const uint8_t* ptr = buffer; const uint8_t* end_ptr = buffer + len; DEBUG_SERIAL.printf(" Parsing %u bytes of raw config data.\n", len); if (ptr + sizeof(usb_configuration_descriptor_t) > end_ptr || ptr[1] != USB_DESCRIPTOR_CONFIGURATION) { DEBUG_SERIAL.println(" ERROR: Raw data does not start with a valid Config Desc header."); enum_state_ = STATE_ERROR; return; } usb_configuration_descriptor_t* cfg_desc = (usb_configuration_descriptor_t*)ptr; config.bConfigurationValue = cfg_desc->bConfigurationValue; config.iConfiguration = cfg_desc->iConfiguration; config.bmAttributes = cfg_desc->bmAttributes; config.bMaxPower = cfg_desc->bMaxPower; config.wTotalLength = cfg_desc->wTotalLength; DEBUG_SERIAL.printf(" Config Desc: CfgVal=%u, iCfg=%u, Attr=0x%X, Pwr=%umA, #Intf=%u, RepLen=%u\n", config.bConfigurationValue, config.iConfiguration, config.bmAttributes, config.bMaxPower * 2, cfg_desc->bNumInterfaces, config.wTotalLength); ptr += cfg_desc->bLength; UsbInterfaceData* currentInterface = nullptr; config.interfaceCount = 0;
    while (ptr < end_ptr) { if (ptr + 2 > end_ptr) { DEBUG_SERIAL.println(" ERROR: Malformed block (EOF before descriptor header). Parsing stopped."); break; } uint8_t dLen = ptr[0]; uint8_t dType = ptr[1]; if (dLen < 2) { DEBUG_SERIAL.printf(" ERROR: Invalid descriptor length %u at offset %ld. Parsing stopped.\n", dLen, (long)(ptr - buffer)); break; } if (ptr + dLen > end_ptr) { DEBUG_SERIAL.printf(" ERROR: Descriptor length %u exceeds remaining buffer size at offset %ld. Parsing stopped.\n", dLen, (long)(ptr - buffer)); break; } DEBUG_SERIAL.printf("\n Found Desc: Type=0x%02X (%s), Length=%u at offset %ld\n", dType, descriptor_type_to_string(dType), dLen, (long)(ptr - buffer));
    switch (dType) { case USB_DESCRIPTOR_INTERFACE: if (config.interfaceCount < MAX_INTERFACES_PER_CONFIG) { if (dLen < sizeof(usb_interface_descriptor_t)) { DEBUG_SERIAL.println(" ERROR: Interface descriptor too short."); } else { currentInterface = &config.interfaces[config.interfaceCount]; *currentInterface = UsbInterfaceData(); usb_interface_descriptor_t* ifd = (usb_interface_descriptor_t*)ptr; currentInterface->bInterfaceNumber = ifd->bInterfaceNumber; currentInterface->bAlternateSetting = ifd->bAlternateSetting; currentInterface->bNumEndpoints = ifd->bNumEndpoints; currentInterface->bInterfaceClass = ifd->bInterfaceClass; currentInterface->bInterfaceSubClass = ifd->bInterfaceSubClass; currentInterface->bInterfaceProtocol = ifd->bInterfaceProtocol; currentInterface->iInterface = ifd->iInterface; currentInterface->isHidInterface = (ifd->bInterfaceClass == 3); currentInterface->endpointCount = 0; currentInterface->reportDescriptorCount = 0; DEBUG_SERIAL.printf(" -> Interface Stored (Index %u): Num=%u (Alt %u), Class=%X, Sub=%X, Proto=%X, #EPs=%u, iInterface=%u %s\n", config.interfaceCount, currentInterface->bInterfaceNumber, currentInterface->bAlternateSetting, currentInterface->bInterfaceClass, currentInterface->bInterfaceSubClass, currentInterface->bInterfaceProtocol, currentInterface->bNumEndpoints, currentInterface->iInterface, (currentInterface->isHidInterface ? "[HID]" : "")); config.interfaceCount++; } } else { DEBUG_SERIAL.printf(" WARN: Exceeded MAX_INTERFACES_PER_CONFIG (%u). Skipping interface.\n", MAX_INTERFACES_PER_CONFIG); currentInterface = nullptr; } break;
    case USB_DESCRIPTOR_ENDPOINT: if (currentInterface == nullptr) { DEBUG_SERIAL.println(" WARN: Found Endpoint descriptor without a current Interface context. Skipping."); } else if (currentInterface->endpointCount >= MAX_ENDPOINTS_PER_INTERFACE) { DEBUG_SERIAL.printf(" WARN: Exceeded MAX_ENDPOINTS_PER_INTERFACE (%u) for Iface %u. Skipping endpoint.\n", MAX_ENDPOINTS_PER_INTERFACE, currentInterface->bInterfaceNumber); } else if (dLen < sizeof(usb_endpoint_descriptor_t)) { DEBUG_SERIAL.println(" ERROR: Endpoint descriptor too short. Skipping."); } else { UsbEndpointData& epd = currentInterface->endpoints[currentInterface->endpointCount]; epd = UsbEndpointData(); usb_endpoint_descriptor_t* epd_raw = (usb_endpoint_descriptor_t*)ptr; epd.bEndpointAddress = epd_raw->bEndpointAddress; epd.bmAttributes = epd_raw->bmAttributes; epd.wMaxPacketSize = epd_raw->wMaxPacketSize; epd.bInterval = epd_raw->bInterval; epd.bSynchAddress = 0; if (((epd.bmAttributes & 0x03) == 1) && dLen >= 9) { epd.bSynchAddress = ptr[8]; } DEBUG_SERIAL.printf("    -> Endpoint Stored (Index %u): Addr=0x%X (%s), Attr=0x%X (%s), MaxPkt=%u, Interval=%u\n", currentInterface->endpointCount, epd.bEndpointAddress, ((epd.bEndpointAddress & 0x80) ? "IN" : "OUT"), epd.bmAttributes, (((epd.bmAttributes & 0x03) == 0) ? "Ctrl" : (((epd.bmAttributes & 0x03) == 1) ? "Iso" : (((epd.bmAttributes & 0x03) == 2) ? "Bulk" : "Int"))), epd.wMaxPacketSize & 0x7FF, epd.bInterval); currentInterface->endpointCount++; } break;
    case USB_DESC_TYPE_HID: if (currentInterface == nullptr) { DEBUG_SERIAL.println(" WARN: Found HID descriptor without a current Interface context. Skipping."); } else if (!currentInterface->isHidInterface) { DEBUG_SERIAL.printf(" WARN: Found HID descriptor associated with non-HID Interface %u. Skipping.\n", currentInterface->bInterfaceNumber); } else if (dLen < offsetof(usb_hid_descriptor_t, optional_descriptors)) { DEBUG_SERIAL.println(" ERROR: HID descriptor header too short. Skipping."); } else { usb_hid_descriptor_t* hd = (usb_hid_descriptor_t*)ptr; currentInterface->bcdHID = hd->bcdHID; currentInterface->bCountryCode = hd->bCountryCode; currentInterface->bNumHidClassDescriptors = hd->bNumDescriptors; currentInterface->reportDescriptorCount = 0; DEBUG_SERIAL.printf("    -> HID Class Desc Stored: bcd=0x%X, Country=%u, #OptionalDesc=%u\n", currentInterface->bcdHID, currentInterface->bCountryCode, currentInterface->bNumHidClassDescriptors); const uint8_t* optional_desc_ptr = ((const uint8_t*)hd) + offsetof(usb_hid_descriptor_t, optional_descriptors); for (int i = 0; i < currentInterface->bNumHidClassDescriptors; ++i) { if (optional_desc_ptr + 3 > ptr + dLen) { DEBUG_SERIAL.printf(" ERROR: HID optional descriptor entry %d goes out of bounds. Stopping optional parse.\n", i); break; } uint8_t optional_type = optional_desc_ptr[0]; uint16_t optional_len = optional_desc_ptr[1] | (optional_desc_ptr[2] << 8); DEBUG_SERIAL.printf("       Optional HID Desc %d: Type=0x%X (%s), Len=%u\n", i, optional_type, descriptor_type_to_string(optional_type), optional_len); if (optional_type == USB_DESC_TYPE_REPORT) { if (currentInterface->reportDescriptorCount < MAX_HID_REPORT_DESC_PER_INTERFACE) { UsbHidReportDescInfo& r = currentInterface->reportDescriptors[currentInterface->reportDescriptorCount]; r = UsbHidReportDescInfo(); r.bDescriptorType = optional_type; r.wDescriptorLength = optional_len; r.rawData = nullptr; DEBUG_SERIAL.printf("         -> Stored Report Desc Info (Index %u)\n", currentInterface->reportDescriptorCount); currentInterface->reportDescriptorCount++; } else { DEBUG_SERIAL.printf(" WARN: Exceeded MAX_HID_REPORT_DESC_PER_INTERFACE (%u). Skipping optional report desc.\n", MAX_HID_REPORT_DESC_PER_INTERFACE); } } optional_desc_ptr += 3; } } break;
    case USB_DESCRIPTOR_INTERFACE_ASSOCIATION: if (dLen < sizeof(usb_interface_assoc_descriptor_t)) { DEBUG_SERIAL.println(" ERROR: IAD too short."); } else { usb_interface_assoc_descriptor_t* iad = (usb_interface_assoc_descriptor_t*)ptr; DEBUG_SERIAL.printf(" -> IAD: FirstIface=%u, Count=%u, Class=%X, Sub=%X, Proto=%X, iFunc=%u\n", iad->bFirstInterface, iad->bInterfaceCount, iad->bFunctionClass, iad->bFunctionSubClass, iad->bFunctionProtocol, iad->iFunction); } break;
    default: DEBUG_SERIAL.print("    -> Unknown/Unhandled Descriptor. Data (hex): "); for(int i=2; i<dLen; ++i) { if (i >= 18) { DEBUG_SERIAL.print("..."); break; } DEBUG_SERIAL.printf("%02X ", ptr[i]); } DEBUG_SERIAL.println(); break; } ptr += dLen; }
    if (ptr > end_ptr) { DEBUG_SERIAL.println(" WARN: Parser overran end of buffer."); } else if (ptr < end_ptr) { DEBUG_SERIAL.printf(" WARN: Parser finished with %ld bytes remaining.\n", (long)(end_ptr - ptr)); } DEBUG_SERIAL.println("--- Finished parseFullConfigurationBlock ---");
}

void DeviceEnumerator::storeString(char* buffer, size_t buffer_len, const Transfer_t* transfer) {
    // ... (Keep the content of this function as is - no changes needed here) ...
    if (!buffer || buffer_len == 0) return; buffer[0] = '\0'; if (!transfer || transfer->length < 2) { DEBUG_SERIAL.println("   storeString: Transfer null or too short."); return; } const uint8_t* data = (const uint8_t*)temp_buffer_; uint8_t total_len = data[0]; uint8_t desc_type = data[1]; if (desc_type != USB_DESCRIPTOR_STRING) { DEBUG_SERIAL.printf("   storeString: Incorrect descriptor type (%u).\n", desc_type); return; } if (total_len < 2) { DEBUG_SERIAL.println("   storeString: Descriptor length < 2."); return; } if (total_len > transfer->length) { total_len = transfer->length; } size_t char_count = 0; for (uint8_t i = 2; i < total_len && char_count < (buffer_len - 1); i += 2) { if (i + 1 < total_len && data[i+1] == 0) { char ascii_char = (char)data[i]; buffer[char_count++] = isprint(ascii_char) ? ascii_char : '?'; } else { buffer[char_count++] = '?'; } } buffer[char_count] = '\0'; DEBUG_SERIAL.printf("   storeString: Stored '%s'\n", buffer);
}

void DeviceEnumerator::printStoredData(Print& printer) const {
    // ... (Keep the content of this function as is - no changes needed here) ...
     printer.println("\n================================================="); printer.println("======= Stored USB Device Enumeration Data ======="); printer.println("================================================="); if (enum_state_ == STATE_ERROR) { printer.println("!!! WARNING: Enumeration ended in ERROR state. Data below might be incomplete or incorrect. !!!"); } else if (enum_state_ != STATE_DONE) { printer.println("!!! WARNING: Enumeration not fully DONE. Data below might be incomplete. !!!"); } if (stored_data_.idVendor == 0 && stored_data_.idProduct == 0) { printer.println("--- No Valid Device Data Stored ---"); printer.println("================================================="); return; }
     printer.println("--- Device Info ---"); printer.printf("  VID: 0x%04X, PID: 0x%04X, Address: %u\n", stored_data_.idVendor, stored_data_.idProduct, stored_data_.address); printer.print("  Speed: "); switch(stored_data_.speed) { case USB_SPEED_LOW: printer.print("Low (1.5Mbps)"); break; case USB_SPEED_FULL: printer.print("Full (12Mbps)"); break; case USB_SPEED_HIGH: printer.print("High (480Mbps)"); break; default: printer.printf("Unknown (%u)", stored_data_.speed); break; } printer.println(); printer.printf("  Location: Hub %u, Port %u\n", stored_data_.hub_address, stored_data_.hub_port); printer.printf("  Class: 0x%02X, SubClass: 0x%02X, Protocol: 0x%02X (%s)\n", stored_data_.bDeviceClass, stored_data_.bDeviceSubClass, stored_data_.bDeviceProtocol, (stored_data_.bDeviceClass == 0 ? "(Defined at Interface level)" : "Device Specific")); printer.printf("  MaxPacketSize0: %u bytes\n", stored_data_.bMaxPacketSize0); printer.printf("  bcdUSB: 0x%04X (%u.%u%u), ", stored_data_.bcdUSB, stored_data_.bcdUSB >> 8, (stored_data_.bcdUSB >> 4) & 0xF, stored_data_.bcdUSB & 0xF); printer.printf("bcdDevice: 0x%04X (%u.%u%u)\n", stored_data_.bcdDevice, stored_data_.bcdDevice >> 8, (stored_data_.bcdDevice >> 4) & 0xF, stored_data_.bcdDevice & 0xF); printer.printf("  NumConfigurations Reported: %u (Stored: %u)\n", stored_data_.bNumConfigurations, stored_data_.configurationCount); printer.printf("  String Index - Manufacturer: %u\n", stored_data_.iManufacturer); printer.printf("  String Index - Product:      %u\n", stored_data_.iProduct); printer.printf("  String Index - Serial #:     %u\n", stored_data_.iSerialNumber); printer.printf("  String Value - Manufacturer: '%s'\n", stored_data_.manufacturerString); printer.printf("  String Value - Product:      '%s'\n", stored_data_.productString); printer.printf("  String Value - Serial #:     '%s'\n", stored_data_.serialNumberString);
     for (uint8_t c = 0; c < stored_data_.configurationCount; ++c) { const UsbConfigurationData& config = stored_data_.configurations[c]; printer.printf("\n--- Configuration %u Details ---\n", c); printer.printf("  bConfigurationValue: %u\n", config.bConfigurationValue); printer.printf("  iConfiguration (String Index): %u\n", config.iConfiguration); printer.printf("  Configuration String:        '%s'\n", config.configurationString); printer.printf("  bmAttributes:        0x%02X (", config.bmAttributes); if(config.bmAttributes & 0x40) printer.print("SelfPowered"); else printer.print("BusPowered"); if(config.bmAttributes & 0x20) printer.print(", RemoteWakeup"); printer.println(")"); printer.printf("  bMaxPower:           %u (%u mA)\n", config.bMaxPower, config.bMaxPower * 2); printer.printf("  wTotalLength (Reported by Desc): %u\n", config.wTotalLength); printer.printf("  Raw Config Data Length Stored: %u bytes\n", config.rawConfigLen);
     if (config.rawConfigData && config.rawConfigLen > 0) { printer.println("  Raw Config Data Dump (hex):"); printer.print("    "); for(uint16_t k=0; k < config.rawConfigLen; ++k) { if (k > 0 && k % 16 == 0) { printer.print("\n    "); } else if (k > 0 && k % 8 == 0) { printer.print(" "); } printer.printf("%02X ", config.rawConfigData[k]); } printer.println(); } else { printer.println("  Raw Config Data Dump: <Not Available or Empty>"); } printer.printf("  Number of Interfaces Parsed: %u\n", config.interfaceCount);
     for (uint8_t i = 0; i < config.interfaceCount; ++i) { const UsbInterfaceData& iface = config.interfaces[i]; printer.printf("\n  --- Interface %u (Array Index %u) ---\n", iface.bInterfaceNumber, i); printer.printf("    bAlternateSetting: %u\n", iface.bAlternateSetting); printer.printf("    bInterfaceClass:   0x%02X\n", iface.bInterfaceClass); printer.printf("    bInterfaceSubClass:0x%02X\n", iface.bInterfaceSubClass); printer.printf("    bInterfaceProtocol:0x%02X\n", iface.bInterfaceProtocol); printer.printf("    iInterface (String Index): %u\n", iface.iInterface); printer.printf("    Interface String:        '%s'\n", iface.interfaceString); printer.printf("    bNumEndpoints Reported:    %u (Stored: %u)\n", iface.bNumEndpoints, iface.endpointCount); printer.printf("    Is HID Interface?: %s\n", (iface.isHidInterface ? "Yes" : "No"));
     if (iface.isHidInterface) { printer.println("    --- HID Class Descriptor Info ---"); printer.printf("      bcdHID:          0x%04X (%u.%u%u)\n", iface.bcdHID, iface.bcdHID >> 8, (iface.bcdHID >> 4) & 0xF, iface.bcdHID & 0xF); printer.printf("      bCountryCode:    %u\n", iface.bCountryCode); printer.printf("      bNumDescriptors (Optional in HID Desc): %u\n", iface.bNumHidClassDescriptors); printer.printf("      Report Descriptors Found/Stored: %u\n", iface.reportDescriptorCount);
     for (uint8_t r = 0; r < iface.reportDescriptorCount; ++r) { const UsbHidReportDescInfo& report = iface.reportDescriptors[r]; printer.printf("      --- Stored Report Descriptor Info %u ---\n", r); printer.printf("        bDescriptorType:   0x%02X (%s)\n", report.bDescriptorType, descriptor_type_to_string(report.bDescriptorType)); printer.printf("        wDescriptorLength: %u bytes\n", report.wDescriptorLength); if (report.rawData && report.wDescriptorLength > 0) { printer.println("        Raw Report Descriptor Dump (hex):"); printer.print("          "); for (uint16_t k = 0; k < report.wDescriptorLength; ++k) { if (k > 0 && k % 16 == 0) { printer.print("\n          "); } else if (k > 0 && k % 8 == 0) { printer.print(" "); } printer.printf("%02X ", report.rawData[k]); } printer.println(); } else { printer.println("        Raw Report Descriptor Dump: <Not Fetched, Empty, or Error>"); } } }
     printer.println("    --- Endpoints Stored ---"); if (iface.endpointCount == 0) { printer.println("      (None Found/Stored)"); } for (uint8_t e = 0; e < iface.endpointCount; ++e) { const UsbEndpointData& ep = iface.endpoints[e]; uint16_t maxPkt = ep.wMaxPacketSize & 0x7FF; uint8_t multi = (ep.wMaxPacketSize >> 11) & 0x03; printer.printf("      Endpoint %u: Addr=0x%02X(%s), Attr=0x%02X(%s), MaxPkt=%u%s, Interval=%u\n", e, ep.bEndpointAddress, ((ep.bEndpointAddress & 0x80) ? "IN" : "OUT"), ep.bmAttributes, (((ep.bmAttributes & 0x03) == 0) ? "Ctrl" : (((ep.bmAttributes & 0x03) == 1) ? "Iso" : (((ep.bmAttributes & 0x03) == 2) ? "Bulk" : "Int"))), maxPkt, (stored_data_.speed == USB_SPEED_HIGH && maxPkt > 0 && ((ep.bmAttributes & 0x03) == 1 || (ep.bmAttributes & 0x03) == 3) && multi > 0) ? (" x" + String(multi + 1)) : "", ep.bInterval); printer.print("        Interval Interpretation: "); print_endpoint_interval(printer, ep.bInterval, stored_data_.speed, ep.bmAttributes); printer.println(); } } }
     printer.println("\n================================================="); printer.println("======= End of Stored Data Printout ======="); printer.println("=================================================");
}