#include "serial_port.h"
#include <wx/log.h>

#ifndef NOMINMAX
#define NOMINMAX
#endif

static BYTE MapStopBits(uint8_t sb) {
    if (sb == 2)
        return TWOSTOPBITS;
    // If the DCB is set to ONE5STOPBITS, WriteFile will return false.
    // To support 1.5 stop bits, uncomment the following line.
    //else if (sb == 15)
    //    return ONE5STOPBITS;
    return ONESTOPBIT;
}

static BYTE MapParity(uint8_t p) {
    if (p == 1)
        return ODDPARITY;
    else if (p == 2)
        return EVENPARITY;
    else if (p == 3)
        return MARKPARITY;
    else if (p == 4)
        return SPACEPARITY;
    else
        return NOPARITY;
}

SerialPort::SerialPort() {
    SerialPort::d_ = INVALID_HANDLE_VALUE;
}

SerialPort::~SerialPort() { close(); }

bool SerialPort::changeConfig(const SerialConfig& cfg) {
    DCB dcb;
    dcb.DCBlength = sizeof(dcb);
    if (!::GetCommState(d_, &dcb)) {
        ::CloseHandle(d_);
        return false;
    }

    dcb.BaudRate = cfg.baud;
    dcb.ByteSize = (cfg.byteSize < 4 || cfg.byteSize > 8) ? 8 : cfg.byteSize;
    dcb.StopBits = MapStopBits(cfg.stopBits);
    dcb.Parity = MapParity(cfg.parity);
    dcb.fParity = (dcb.Parity != NOPARITY);
    dcb.fBinary = TRUE;
    dcb.fDtrControl = cfg.dtr ? DTR_CONTROL_ENABLE : DTR_CONTROL_DISABLE;
    dcb.fRtsControl = cfg.rts ? RTS_CONTROL_ENABLE : RTS_CONTROL_DISABLE;
    if (!::SetCommState(d_, &dcb)) {
        ::CloseHandle(d_);
        return false;
    }
    return true;
}

bool SerialPort::open(const wxString& port, const SerialConfig& cfg) {
    close();

    wxString device = wxString::Format("\\\\.\\%s", port);
    d_ = ::CreateFileW(device.wc_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, nullptr);
    if (d_ == INVALID_HANDLE_VALUE) return false;

	if (!changeConfig(cfg))
        return false;

    COMMTIMEOUTS to{};
    to.ReadIntervalTimeout = 100;   // ms
    to.ReadTotalTimeoutConstant = 100;   // ms
    to.ReadTotalTimeoutMultiplier = 1;   // ms/byte
    to.WriteTotalTimeoutConstant = 100;   // ms
    to.WriteTotalTimeoutMultiplier = 1;   // ms/byte
    if (!::SetCommTimeouts(d_, &to)) { ::CloseHandle(d_); return false; }
    ::SetupComm(d_, 4096, 4096);
    ::PurgeComm(d_, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);
    ::SetCommMask(d_, EV_RXCHAR | EV_ERR);
    return true;
}

void SerialPort::close() {
    if (d_ != INVALID_HANDLE_VALUE) {
        ::CloseHandle(d_);
        d_ = INVALID_HANDLE_VALUE;
    }
}

bool SerialPort::isOpen() {
    return d_ != INVALID_HANDLE_VALUE;
}

bool SerialPort::write(const void* data, unsigned long size, unsigned long* written) {
    if (!isOpen() || data == nullptr || size == 0) {
        if (written) *written = 0;
        return false;
    }

    const char* p = static_cast<const char*>(data);
    unsigned long total = 0;

    while (total < size) {
        OVERLAPPED ov{};
        ov.hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
        if (!ov.hEvent) return false;

        DWORD w = 0;
        BOOL ok = ::WriteFile(d_, p + total, size - total, &w, &ov);
        if (!ok) {
            const DWORD err = GetLastError();
            if (err == ERROR_IO_PENDING) {
                const DWORD wait = WaitForSingleObject(ov.hEvent, INFINITE);
                if (wait != WAIT_OBJECT_0 || !::GetOverlappedResult(d_, &ov, &w, FALSE)) {
                    ::CancelIoEx(d_, &ov);
                    CloseHandle(ov.hEvent);
                    if (written) *written = total;
                    return false;
                }
            }
            else {
                CloseHandle(ov.hEvent);
                if (written)
                    *written = total;
                return false;
            }
        }
        CloseHandle(ov.hEvent);

        if (w == 0) break;
        total += w;
    }

    if (written) *written = total;
    return total == size;
}


unsigned long SerialPort::read(void* buffer, unsigned long size) {
    if (!isOpen()) return 0;
    DWORD r = 0;
    if (!::ReadFile(d_, buffer, static_cast<DWORD>(size), &r, nullptr)) return 0;
    return r;
}

HANDLE SerialPort::getHandle() {
    return d_;
}
