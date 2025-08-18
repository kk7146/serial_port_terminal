#include "serial_port.h"
#include <wx/log.h>

#ifndef NOMINMAX
#define NOMINMAX
#endif

static wxString WinErrMsg(DWORD e) {
    LPWSTR buf = nullptr;
    DWORD n = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, e, 0, (LPWSTR)&buf, 0, nullptr);
    wxString s = (n && buf) ? wxString(buf) : wxString::Format("WinErr=%lu", e);
    if (buf) LocalFree(buf);
    return s.Trim(true).Trim(false);
}

static BYTE MapStopBits(uint8_t sb) {
    if (sb == 2) return TWOSTOPBITS;
    else if (sb == 15)ONE5STOPBITS;
    return ONESTOPBIT;
}
static BYTE MapParity(uint8_t p) { return (p <= 4) ? p : NOPARITY; }

struct SerialPort::Impl {
    HANDLE handle = INVALID_HANDLE_VALUE;
};

SerialPort::SerialPort() : d_(std::make_unique<Impl>()) {}
SerialPort::~SerialPort() { close(); }

bool SerialPort::open(const wxString& port, const SerialConfig& cfg) {
    close();

    wxString device = wxString::Format("\\\\.\\%s", port);
    HANDLE h = ::CreateFileW(device.wc_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) return false;

    DCB dcb{}; dcb.DCBlength = sizeof(dcb);
    if (!::GetCommState(h, &dcb)) { ::CloseHandle(h); return false; }

    dcb.BaudRate = cfg.baud;
    dcb.ByteSize = (cfg.byteSize < 4 || cfg.byteSize > 8) ? 8 : cfg.byteSize;
    dcb.StopBits = MapStopBits(cfg.stopBits);
    dcb.Parity = MapParity(cfg.parity);
    dcb.fParity = (dcb.Parity != NOPARITY);
    dcb.fBinary = TRUE;
    dcb.fDtrControl = cfg.dtr ? DTR_CONTROL_ENABLE : DTR_CONTROL_DISABLE;
    dcb.fRtsControl = cfg.rts ? RTS_CONTROL_ENABLE : RTS_CONTROL_DISABLE;

    if (!::GetCommState(h, &dcb)) { ::CloseHandle(h); return false; }
    COMMTIMEOUTS to{};
    to.ReadIntervalTimeout = 50;   // ms
    to.ReadTotalTimeoutConstant = 50;   // ms
    to.ReadTotalTimeoutMultiplier = 0;   // ms/byte
    to.WriteTotalTimeoutConstant = 50;   // ms
    to.WriteTotalTimeoutMultiplier = 0;   // ms/byte
    if (!::SetCommTimeouts(h, &to)) { ::CloseHandle(h); return false; }
    ::SetupComm(h, 4096, 4096);
    ::PurgeComm(h, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);

    d_->handle = h;
    return true;
}

void SerialPort::close() {
    if (d_->handle != INVALID_HANDLE_VALUE) {
        ::CloseHandle(d_->handle);
        d_->handle = INVALID_HANDLE_VALUE;
    }
}

bool SerialPort::isOpen() const {
    return d_->handle != INVALID_HANDLE_VALUE;
}

bool SerialPort::write(const void* data, unsigned long size, unsigned long* written) {
    if (!isOpen()) return false;
    DWORD w = 0;
    BOOL ok = ::WriteFile(d_->handle, data, static_cast<DWORD>(size), &w, nullptr);
    if (written) *written = w;
    return ok == TRUE;
}

unsigned long SerialPort::read(void* buffer, unsigned long size) {
    if (!isOpen()) return 0;
    DWORD r = 0;
    if (!::ReadFile(d_->handle, buffer, static_cast<DWORD>(size), &r, nullptr)) return 0;
    return r;
}
