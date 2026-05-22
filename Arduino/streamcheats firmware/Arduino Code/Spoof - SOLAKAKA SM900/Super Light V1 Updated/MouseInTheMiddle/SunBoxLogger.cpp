#include "SunBoxLogger.h"
#include "Config.h"

SunBoxLogger logger;

void SunBoxLogger::begin() {
    // FTDI_DEVICE.begin(BAUD) is already called in MouseInTheMiddle.ino setup().
    // Nothing further to do for now.
}

void SunBoxLogger::error(const String& message)   { printWithPrefix("E: ", message); }
void SunBoxLogger::warning(const String& message) { printWithPrefix("W: ", message); }
void SunBoxLogger::startup(const String& message) { printWithPrefix("S: ", message); }
void SunBoxLogger::info(const String& message)    { printWithPrefix("I: ", message); }
void SunBoxLogger::mouse(const String& message)   { printWithPrefix("M: ", message); }

void SunBoxLogger::debug(const String& message) {
    if (DEBUG_MODE) printWithPrefix("D: ", message);
}

void SunBoxLogger::errorf(const char* format, ...) {
    va_list args; va_start(args, format);
    printfWithPrefix("E: ", format, args);
    va_end(args);
}

void SunBoxLogger::warningf(const char* format, ...) {
    va_list args; va_start(args, format);
    printfWithPrefix("W: ", format, args);
    va_end(args);
}

void SunBoxLogger::startupf(const char* format, ...) {
    va_list args; va_start(args, format);
    printfWithPrefix("S: ", format, args);
    va_end(args);
}

void SunBoxLogger::infof(const char* format, ...) {
    va_list args; va_start(args, format);
    printfWithPrefix("I: ", format, args);
    va_end(args);
}

void SunBoxLogger::mousef(const char* format, ...) {
    va_list args; va_start(args, format);
    printfWithPrefix("M: ", format, args);
    va_end(args);
}

void SunBoxLogger::debugf(const char* format, ...) {
    if (!DEBUG_MODE) return;
    va_list args; va_start(args, format);
    printfWithPrefix("D: ", format, args);
    va_end(args);
}

void SunBoxLogger::printWithPrefix(const char* prefix, const String& message) {
    FTDI_DEVICE.print(prefix);
    FTDI_DEVICE.println(message);
}

void SunBoxLogger::printfWithPrefix(const char* prefix, const char* format, va_list args) {
    vsnprintf(formatBuffer_, FORMAT_BUFFER_SIZE, format, args);
    FTDI_DEVICE.print(prefix);
    FTDI_DEVICE.println(formatBuffer_);
}
