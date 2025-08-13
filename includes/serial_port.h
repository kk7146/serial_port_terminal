#pragma once

#include <wx/string.h>
#include <memory>

class SerialPort {
public:
    SerialPort();
    ~SerialPort();

    SerialPort(const SerialPort&) = delete;
    SerialPort& operator=(const SerialPort&) = delete;

    bool open(const wxString& port, unsigned long baud);
    void close();
    bool isOpen() const;
    bool setBaud(const wxString& parity);
    bool setParity(const wxString& parity);
    bool setStop(const wxString& parity);
    bool setByte(const wxString& parity);

    bool write(const void* data, unsigned long size, unsigned long* written = nullptr);
    unsigned long read(void* buffer, unsigned long size);

private:
    struct Impl;                 // pImpl to hide Win32 details
    std::unique_ptr<Impl> d_;
    unsigned long baud;
    unsigned long parity;
	unsigned long stop;
	unsigned long byte;
};
