#include "serial_port.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

struct SerialPort::Impl {
    HANDLE handle = INVALID_HANDLE_VALUE;
};

SerialPort::SerialPort() : d_(std::make_unique<Impl>()) {}
SerialPort::~SerialPort() { close(); }

bool SerialPort::open(const wxString& port, unsigned long baud) {
    close();

    wxString device = wxString::Format("\\\\.\\%s", port);
    HANDLE h = ::CreateFileW(device.wc_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) return false;

    DCB dcb{};
    dcb.DCBlength = sizeof(dcb);
    if (!::GetCommState(h, &dcb)) { ::CloseHandle(h); return false; }

    dcb.BaudRate = this->baud;
    dcb.ByteSize = this->byte;
    dcb.StopBits = this->stop;
    dcb.Parity = NOPARITY; // ¹Ù²Ù±â
    dcb.fBinary = TRUE;
    dcb.fDtrControl = DTR_CONTROL_ENABLE;
    dcb.fRtsControl = RTS_CONTROL_ENABLE;

    if (!::SetCommState(h, &dcb)) { ::CloseHandle(h); return false; }

    COMMTIMEOUTS to{};
    to.ReadIntervalTimeout = 50;   // ms
    to.ReadTotalTimeoutConstant = 50;   // ms
    to.ReadTotalTimeoutMultiplier = 10;   // ms/byte
    to.WriteTotalTimeoutConstant = 50;   // ms
    to.WriteTotalTimeoutMultiplier = 10;   // ms/byte
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
