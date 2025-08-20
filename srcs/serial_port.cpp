#include "serial_port.h"
#include <wx/log.h>

#ifndef NOMINMAX
#define NOMINMAX
#endif

// static wxString WinErrMsg(DWORD e) {
//     LPWSTR buf = nullptr;
//     DWORD n = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
//         nullptr, e, 0, (LPWSTR)&buf, 0, nullptr);
//     wxString s = (n && buf) ? wxString(buf) : wxString::Format("WinErr=%lu", e);
//     if (buf) LocalFree(buf);
//     return s.Trim(true).Trim(false);
// }

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
    d_ = ::CreateFileW(device.wc_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (d_ == INVALID_HANDLE_VALUE) return false;

	if (!changeConfig(cfg))
        return false;

    COMMTIMEOUTS to{};
    to.ReadIntervalTimeout = 50;   // ms
    to.ReadTotalTimeoutConstant = 50;   // ms
    to.ReadTotalTimeoutMultiplier = 1;   // ms/byte
    to.WriteTotalTimeoutConstant = 50;   // ms
    to.WriteTotalTimeoutMultiplier = 1;   // ms/byte
    if (!::SetCommTimeouts(d_, &to)) { ::CloseHandle(d_); return false; }
    ::SetupComm(d_, 4096, 4096);
    ::PurgeComm(d_, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);

    return true;
}

void SerialPort::close() {
    if (d_ != INVALID_HANDLE_VALUE) {
        ::CloseHandle(d_);
        d_ = INVALID_HANDLE_VALUE;
    }
}

bool SerialPort::isOpen() const {
    return d_ != INVALID_HANDLE_VALUE;
}

bool SerialPort::write(const void* data, unsigned long size, unsigned long* written) {
    if (!isOpen()) return false;
    DWORD w = 0;
    bool result = ::WriteFile(d_, data, static_cast<DWORD>(size), &w, nullptr);
    if (written) *written = w;
    return result == true;
}

unsigned long SerialPort::read(void* buffer, unsigned long size) {
    if (!isOpen()) return 0;
    DWORD r = 0;
    if (!::ReadFile(d_, buffer, static_cast<DWORD>(size), &r, nullptr)) return 0;
    return r;
}
