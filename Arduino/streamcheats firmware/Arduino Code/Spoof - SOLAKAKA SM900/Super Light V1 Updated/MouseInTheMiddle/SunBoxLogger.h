#ifndef _SUNBOX_LOGGER_H_
#define _SUNBOX_LOGGER_H_

#include <Arduino.h>
#include <stdarg.h>

// Minimal port of the Teensy SunBoxLogger API. Routes to FTDI_DEVICE
// (defined in Config.h, currently Serial1). Prefixes match the Teensy
// build: S: startup, E: error, W: warning, I: info, D: debug, M: mouse.
// Debug output is gated on the compile-time DEBUG_MODE flag.
class SunBoxLogger {
public:
    void begin();

    void error(const String& message);
    void errorf(const char* format, ...);

    void warning(const String& message);
    void warningf(const char* format, ...);

    void startup(const String& message);
    void startupf(const char* format, ...);

    void info(const String& message);
    void infof(const char* format, ...);

    void debug(const String& message);
    void debugf(const char* format, ...);

    void mouse(const String& message);
    void mousef(const char* format, ...);

private:
    void printWithPrefix(const char* prefix, const String& message);
    void printfWithPrefix(const char* prefix, const char* format, va_list args);

    static const size_t FORMAT_BUFFER_SIZE = 160;
    char formatBuffer_[FORMAT_BUFFER_SIZE];
};

extern SunBoxLogger logger;

#endif // _SUNBOX_LOGGER_H_
