#pragma once

#include <wx/string.h>
#include <memory>
#include <windows.h>

struct SerialConfig {
    unsigned long baud = 9600;
    uint8_t       byteSize = 8;
    uint8_t       stopBits = 1;
    uint8_t       parity = 0;
    bool          dtr = true;
    bool          rts = true;
};

class SerialPort {
public:
    SerialPort();
    ~SerialPort();

    SerialPort(const SerialPort&) = delete;
    SerialPort& operator=(const SerialPort&) = delete;

    bool open(const wxString& port, const SerialConfig& cfg);
    void close();
    bool isOpen() const;
    bool changeConfig(const SerialConfig& cfg);

    bool write(const void* data, unsigned long size, unsigned long* written = nullptr);
    unsigned long read(void* buffer, unsigned long size);

private:
    HANDLE d_;
};
