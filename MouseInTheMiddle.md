# Arduino Leonardo Serial Logging

This README documents the serial logging conventions used in the Arduino Leonardo project. The project implements structured

serial logging to facilitate debugging and monitoring system behavior. The types of logs are categorized by prefixes, making it easier to filter and understand the log outputs.

## Serial Logging Overview

The Arduino Leonardo utilizes different types of serial logging events, each prefixed with a unique identifier. This helps in quickly identifying the nature of the log message. The following are the types of serial logging events:

- **S: Startup Logs**
  - Prefix: `S`
  - Description: Used for logging system status and startup events. These logs provide insights into the system initialization processes and status.

- **E: Error Logs**
  - Prefix: `E`
  - Description: Dedicated to logging errors. Whenever the system encounters an error, it logs the event with this prefix to indicate a failure or issue that needs attention.

- **I: Information/Debug Logs**
  - Prefix: `I`
  - Description: Used for logging debug information. These logs are particularly useful for development and troubleshooting. The debug logs are only active when the debug flag is turned on.

- **M: Mouse HID Report Logs**
  - Prefix: `M`
  - Description: These logs are specific to Mouse HID Reports and are used when specific actions occur that are relevant to mouse movements or interactions.

## Configuration Settings
See the file Config.h
