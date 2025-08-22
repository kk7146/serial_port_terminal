#include "main_frame.h"
#include <wx/msgdlg.h>
#include <map>
#include <vector>

static const std::map<wxString, BYTE> parityMap = {
        { "none",  0 },
        { "odd",   1 },
        { "even",  2 },
        { "mark",  3 },
        { "space", 4 }
};

// Uncomment the following code if you want to use SetupAPI for enumerating serial ports.

// #ifndef GUID_DEVINTERFACE_COMPORT
// DEFINE_GUID(GUID_DEVINTERFACE_COMPORT,
//     0x86E0D1E0L, 0x8089, 0x11D0, 0x9C, 0xE4, 0x08, 0x00, 0x3E, 0x30, 0x1F, 0x73);
// #endif
// 
// std::vector<wxString> EnumerateSerialPorts_SetupAPI() {
//     std::vector<wxString> ports;
// 
//     HDEVINFO hDevInfo = SetupDiGetClassDevsW(&GUID_DEVINTERFACE_COMPORT,
//         nullptr, nullptr,
//         DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);
//     if (hDevInfo == INVALID_HANDLE_VALUE) return ports;
// 
//     SP_DEVICE_INTERFACE_DATA ifData{};
//     ifData.cbSize = sizeof(ifData);
// 
//     for (DWORD i = 0; SetupDiEnumDeviceInterfaces(hDevInfo, nullptr,
//         &GUID_DEVINTERFACE_COMPORT, i, &ifData); ++i) {
//         DWORD required = 0;
//         SetupDiGetDeviceInterfaceDetailW(hDevInfo, &ifData, nullptr, 0, &required, nullptr);
//         if (required == 0) continue;
// 
//         std::vector<BYTE> buffer(required);
//         auto* detail = reinterpret_cast<SP_DEVICE_INTERFACE_DETAIL_DATA_W*>(buffer.data());
//         detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W);
// 
//         SP_DEVINFO_DATA devInfo{};
//         devInfo.cbSize = sizeof(devInfo);
// 
//         if (!SetupDiGetDeviceInterfaceDetailW(hDevInfo, &ifData, detail, required, nullptr, &devInfo))
//             continue;
// 
//         WCHAR friendly[256];
//         if (SetupDiGetDeviceRegistryPropertyW(hDevInfo, &devInfo, SPDRP_FRIENDLYNAME,
//             nullptr, reinterpret_cast<PBYTE>(friendly),
//             sizeof(friendly), nullptr)) {
// 
//             wxString fn = friendly;
//             int l = fn.Find('('), r = fn.Find(')');
//             if (l != wxNOT_FOUND && r != wxNOT_FOUND && r > l) {
//                 wxString com = fn.Mid(l + 1, r - l - 1);
//                 ports.push_back(com);
//                 continue;
//             }
//         }
//     }
// 
//     SetupDiDestroyDeviceInfoList(hDevInfo);
//     return ports;
// }


std::vector<wxString> EnumerateSerialPorts_Registry() {
    std::vector<wxString> ports;
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
        L"HARDWARE\\DEVICEMAP\\SERIALCOMM",
        0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        return ports;
    }

    DWORD index = 0;
    WCHAR valueName[256];
    BYTE  data[256];
    DWORD valueLen, dataLen, type;

    while (1) {
        valueLen = static_cast<DWORD>(std::size(valueName));
        dataLen = static_cast<DWORD>(std::size(data));
        LONG ret = RegEnumValueW(hKey, index++, valueName, &valueLen,
            nullptr, &type, data, &dataLen);
        if (ret == ERROR_NO_MORE_ITEMS) break;
        if (ret == ERROR_SUCCESS && type == REG_SZ) {
            ports.emplace_back(reinterpret_cast<wchar_t*>(data));
        }
    }

    RegCloseKey(hKey);
    return ports;
}

mainFrame::mainFrame(wxWindow* parent, wxWindowID id)
    : wxFrame(parent, id, "Serial Terminal", wxDefaultPosition, wxSize(700, 600)) {
    SetMinSize(wxSize(700, 600));
    SetMaxSize(wxSize(700, 600));

    auto* panel = new wxPanel(this);

    //Transmit Macro
    new wxStaticBox(panel, wxID_ANY, "Send Macro", wxPoint(16, 16), wxSize(120, 192));
    macroTx_ = new wxTextCtrl(panel, wxID_ANY, "", wxPoint(26, 36), wxSize(102, 80), wxTE_MULTILINE);
    macroCrlf_ = new wxCheckBox(panel, wxID_ANY, "CRLF", wxPoint(41, 120));
    macroCrlf_->SetValue(true);
    new wxStaticText(panel, wxID_ANY, "Timing (ms)", wxPoint(41, 138));
    wxTextValidator validator(wxFILTER_NUMERIC);
    timingTx_ = new wxTextCtrl(panel, wxID_ANY, "1000", wxPoint(39, 155), wxSize(76, 20), wxTE_PROCESS_ENTER, validator);
    btnRepeat_ = new wxButton(panel, wxID_ANY, "Repeat", wxPoint(38, 180), wxSize(77, 23));

    // Port Setup
    new wxStaticBox(panel, wxID_ANY, "Port Setup", wxPoint(544, 16), wxSize(120, 192));
    lstPorts_ = new wxListBox(panel, wxID_ANY, wxPoint(552, 37), wxSize(104, 105));
    btnRefresh_ = new wxButton(panel, wxID_ANY, "Refresh", wxPoint(560, 150), wxSize(88, 23));
    btnConnect_ = new wxButton(panel, wxID_ANY, "Connect", wxPoint(560, 176), wxSize(88, 23));

    // Transmit Data
    new wxStaticBox(panel, wxID_ANY, "Send Data", wxPoint(140, 76), wxSize(401, 132));
    txtTx_ = new wxTextCtrl(panel, wxID_ANY, "", wxPoint(148, 90), wxSize(384, 80), wxTE_MULTILINE | wxTE_PROCESS_ENTER);
    cbCrlf_ = new wxCheckBox(panel, wxID_ANY, "CRLF", wxPoint(150, 176));
    cbCrlf_->SetValue(true);
    btnSend_ = new wxButton(panel, wxID_ANY, "Send", wxPoint(450, 176), wxSize(72, 23));
    btnClearTx_ = new wxButton(panel, wxID_ANY, "Clear", wxPoint(370, 176), wxSize(72, 23));

	//Serial Port Settings
	new wxStaticBox(panel, wxID_ANY, "Port Settings", wxPoint(140, 16), wxSize(401, 60));
    new wxStaticText(panel, wxID_ANY, "Parity Bit", wxPoint(260, 25));
    cbParity_ = new wxComboBox(panel, wxID_ANY, "", wxPoint(260, 45), wxSize(60, 20));
    cbParity_->Append("none");
    cbParity_->Append("even");
    cbParity_->Append("odd");
    cbParity_->Append("mark");
    cbParity_->Append("space");
    cbParity_->SetValue("none");
    new wxStaticText(panel, wxID_ANY, "Stop Bit", wxPoint(330, 25));
    cbStopbit_ = new wxComboBox(panel, wxID_ANY, "", wxPoint(330, 45), wxSize(50, 20));
    cbStopbit_->Append("1");
    cbStopbit_->Append("1.5");
    cbStopbit_->Append("2");
    cbStopbit_->SetValue("1");
    new wxStaticText(panel, wxID_ANY, "Byte Size", wxPoint(390, 25));
    cbBytesize_ = new wxComboBox(panel, wxID_ANY, "", wxPoint(390, 45), wxSize(50, 20));
    cbBytesize_->Append("5");
    cbBytesize_->Append("6");
    cbBytesize_->Append("7");
    cbBytesize_->Append("8");
    cbBytesize_->SetValue("8");
    new wxStaticText(panel, wxID_ANY, "Baud Rate", wxPoint(450, 25));
    cbBaud_ = new wxComboBox(panel, wxID_ANY, "", wxPoint(450, 45), wxSize(70, 20));
    cbBaud_->Append("600");
    cbBaud_->Append("1200");
    cbBaud_->Append("4800");
    cbBaud_->Append("9600");
    cbBaud_->Append("19200");
    cbBaud_->Append("38400");
    cbBaud_->Append("57600");
    cbBaud_->Append("115200");
    cbBaud_->Append("128000");
    cbBaud_->Append("256000");
    cbBaud_->SetValue("115200");

    // Received Data
    new wxStaticBox(panel, wxID_ANY, "Received Data", wxPoint(16, 224), wxSize(648, 302));
    txtRx_ = new wxTextCtrl(panel, wxID_ANY, "", wxPoint(32, 280), wxSize(616, 230), wxTE_MULTILINE | wxTE_READONLY);
    btnClearRx_ = new wxButton(panel, wxID_ANY, "Clear", wxPoint(160, 244), wxSize(72, 23));
    rbChar_ = new wxRadioButton(panel, wxID_ANY, "Char", wxPoint(32, 248), wxDefaultSize, wxRB_GROUP);
    rbHex_ = new wxRadioButton(panel, wxID_ANY, "Hex", wxPoint(104, 248));
    rbChar_->SetValue(true);
    cbListen_ = new wxCheckBox(panel, wxID_ANY, "Listening", wxPoint(240, 248));
    cbListen_->SetValue(true);

    CreateStatusBar(1);
    
    // Repeat Timer
    repeatTimer_.SetOwner(this);

    // Bindings
    btnSend_->Bind(wxEVT_BUTTON, &mainFrame::OnSend, this);
    btnClearTx_->Bind(wxEVT_BUTTON, &mainFrame::OnClearTx, this);
    btnConnect_->Bind(wxEVT_BUTTON, &mainFrame::OnConnectToggle, this);
    btnRepeat_->Bind(wxEVT_BUTTON, &mainFrame::OnRepeatToggle, this);
    btnRefresh_->Bind(wxEVT_BUTTON, &mainFrame::OnRefresh, this);
    btnClearRx_->Bind(wxEVT_BUTTON, &mainFrame::OnClearRx, this);
    rbChar_->Bind(wxEVT_RADIOBUTTON, &mainFrame::OnModeChar, this);
    rbHex_->Bind(wxEVT_RADIOBUTTON, &mainFrame::OnModeHex, this);
    Bind(wxEVT_COMBOBOX, &mainFrame::OnChange, this); // When all combo boxes are changed, this event will be called.
    Bind(wxEVT_TIMER, &mainFrame::OnRepeatTimer, this, repeatTimer_.GetId());
    Bind(wxEVT_CLOSE_WINDOW, &mainFrame::OnClose, this);
    Bind(EVT_RX_DATA, [this](wxThreadEvent& e) {
        rxAccum_ += std::string(e.GetString().ToUTF8());
        if (!cbListen_->IsChecked())
			rxAccum_.clear();
        if (uiFlushClock_.Time() >= kUiFlushMs) {
            uiFlushClock_.Start();
            if (!rxAccum_.empty()) {
                if (mode_ == Mode::Char)
                    txtRx_->AppendText(wxString::FromUTF8(rxAccum_)); 
                else {
                    wxString line;
                    line.reserve(rxAccum_.size() * 5);
                    for (unsigned char c : rxAccum_) {
                        line.Append(wxString::Format("0x%02X ", c));
                    }
                    line.Append("\n");
                    txtRx_->AppendText(line);
                }
                rxAccum_.clear();
                static const long kMax = 200000;
                if (txtRx_->GetLastPosition() > kMax) {
                    txtRx_->Remove(0, txtRx_->GetLastPosition() - kMax);
                }
            }
        }
        });
    Bind(wxEVT_TEXT_ENTER, [this](wxCommandEvent&) {
        if (!port_.isOpen()) {
            wxMessageBox("Connect to port first.", "Error", wxICON_ERROR, this);
            return;
        }
        const wxString text = txtTx_->GetValue();
        if (text.empty()) return;
        const std::string utf8 = std::string(text.ToUTF8());
        unsigned long written = 0;
        if (!port_.write(utf8.data(), static_cast<unsigned long>(utf8.size()), &written))
            wxMessageBox("Write failed.", "Error", wxICON_ERROR, this);
        if (cbCrlf_->IsChecked()) {
            const char crlf[] = "\n";
            port_.write(crlf, 1, &written);
		}
        txtTx_->Clear();
        });
    txtTx_->Bind(wxEVT_CHAR_HOOK, [this](wxKeyEvent& e) {
        if (e.ControlDown()) {
            switch (e.GetKeyCode()) {
                case 'B': {
                    if (port_.isOpen()) {
                        unsigned long written = 0;
                        const char ctrlKey = 0x02;
                        port_.write(&ctrlKey, 1, &written);
                    }
                    return;
                }
                case 'C': {
                    if (port_.isOpen()) {
                        unsigned long written = 0;
                        const char ctrlKey = 0x03;
                        port_.write(&ctrlKey, 1, &written);
                    }
                    return;
                }
                case 'D': {
                    if (port_.isOpen()) {
                        unsigned long written = 0;
                        const char ctrlKey = 0x04;
                        port_.write(&ctrlKey, 1, &written);
                    }
                    return;
                }
            }
        }
        e.Skip();
        });

    lstPorts_->Freeze();
    lstPorts_->Clear();
    auto ports = EnumerateSerialPorts_Registry();
    // Uncomment the following code if you want to use SetupAPI for enumerating serial ports.
    // if (ports.empty()) {
    //     ports = EnumerateSerialPorts_SetupAPI();
    // }
    for (auto& p : ports) lstPorts_->Append(p);
    lstPorts_->Thaw();
    if (!ports.empty()) lstPorts_->SetSelection(0);

    Centre();
}

void mainFrame::OnClose(wxCloseEvent& e) {
    rxRunning_ = false;
    if (port_.isOpen()) {
        HANDLE h = port_.getHandle();
        SetCommMask(h, 0);
        CancelIoEx(h, nullptr);
    }
    if (rxThread_.joinable())
        rxThread_.join();
    port_.close();
    e.Skip();
}

void mainFrame::OnConnectToggle(wxCommandEvent&) {
    if (!port_.isOpen()) {
        const int sel = lstPorts_->GetSelection();
        if (sel == wxNOT_FOUND) {
            wxMessageBox("Select a COM port first.", "Warning", wxICON_WARNING, this);
            return;
        }
        const wxString portName = lstPorts_->GetString(sel);

        unsigned long baudRate = 0;
        if (!cbBaud_->GetValue().ToULong(&baudRate) || baudRate < 300 || baudRate > 256000) {
            wxMessageBox("Enter a valid baud rate (300~256000).", "Warning", wxICON_WARNING, this);
            return;
        }

        unsigned long byteSize = 0;
        if (!cbBytesize_->GetValue().ToULong(&byteSize) || byteSize < 4 || byteSize > 8) {
            wxMessageBox("Enter a valid byte size (4~8).", "Warning", wxICON_WARNING, this);
            return;
        }

        const wxString stopStr = cbStopbit_->GetValue();
        uint8_t stopBitsCfg = 1;
        if (stopStr == "1")   stopBitsCfg = 1;
        else if (stopStr == "1.5") stopBitsCfg = 15;
        else if (stopStr == "2")   stopBitsCfg = 2;
        else {
            wxMessageBox("Select valid stop bits (1, 1.5, or 2).", "Warning", wxICON_WARNING, this);
            return;
        }

        const wxString parityStr = cbParity_->GetValue();
        auto it = parityMap.find(parityStr);
        if (it == parityMap.end()) {
            wxMessageBox("Select a valid parity (None, Odd, Even, Mark, Space).", "Warning", wxICON_WARNING, this);
            return;
        }

        SerialConfig cfg;
        cfg.baud = baudRate;
        cfg.byteSize = static_cast<uint8_t>(byteSize);
        cfg.stopBits = stopBitsCfg;
        cfg.parity = it->second;
        cfg.dtr = true;
        cfg.rts = true;

        if (!port_.open(portName, cfg)) {
            wxMessageBox(wxString::Format("Failed to open %s @ %lu.", portName, baudRate),
                "Error", wxICON_ERROR, this);
            return;
        }

        rxRunning_ = true;
        rxThread_ = std::thread([this] {
            OVERLAPPED ovWait{};
            ovWait.hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);

            OVERLAPPED ovRead{};
            ovRead.hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);

            DWORD mask = 0;
            std::vector<char> buf(4096);

            while (rxRunning_) {
                ResetEvent(ovWait.hEvent);
                if (!WaitCommEvent(port_.getHandle(), &mask, &ovWait)) {
                    if (GetLastError() != ERROR_IO_PENDING) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(5));
                        continue;
                    }
                    WaitForSingleObject(ovWait.hEvent, INFINITE);
                }

                if ((mask & EV_RXCHAR) == 0) continue;

                while (true) {
                    ResetEvent(ovRead.hEvent);
                    DWORD bytesRead = 0;
                    BOOL result = ReadFile(port_.getHandle(), buf.data(), (DWORD)buf.size(), &bytesRead, &ovRead);
                    if (!result) {
                        if (GetLastError() == ERROR_IO_PENDING) {
                            WaitForSingleObject(ovRead.hEvent, INFINITE);
                            GetOverlappedResult(port_.getHandle(), &ovRead, &bytesRead, FALSE);
                        }
                        else {
                            break;
                        }
                    }
                    if (bytesRead == 0) break;

                    auto* ev = new wxThreadEvent(EVT_RX_DATA);
                    ev->SetString(wxString::FromUTF8(std::string(buf.data(), bytesRead)));
                    wxQueueEvent(this, ev);

                    if (bytesRead < buf.size()) break;
                }
            }

            CloseHandle(ovWait.hEvent);
            CloseHandle(ovRead.hEvent);
            });


        btnConnect_->SetLabel("Disconnect");
        SetStatusText(wxString::Format("Connected"));
    }
    else {
        rxRunning_ = false;

        if (port_.isOpen()) {
            HANDLE h = port_.getHandle();
            SetCommMask(h, 0);
            CancelIoEx(h, nullptr);
        }
        if (rxThread_.joinable())
            rxThread_.join();
        port_.close();
		timingTx_->Enable(true); // Enable timing input when disconnected.
        btnConnect_->SetLabel("Connect");
        SetStatusText("");
    }
}

void mainFrame::OnRepeatToggle(wxCommandEvent&) {
    if (!port_.isOpen()) {
        wxMessageBox("Connect to port first.", "Error", wxICON_ERROR, this);
        return;
    }

    unsigned long intervalMs = 0;
    if (!timingTx_->GetValue().ToULong(&intervalMs) || intervalMs < 10 || intervalMs > 60000) {
        wxMessageBox("Enter a valid period in ms (10 ~ 60000).", "Warning", wxICON_WARNING, this);
        return;
    }

    if (!isRepeating_) {
        repeatTimer_.Start(static_cast<int>(intervalMs), wxTIMER_CONTINUOUS);
        isRepeating_ = true;
        btnRepeat_->SetLabel("Stop");
        timingTx_->Enable(false);
    }
    else {
        repeatTimer_.Stop();
        isRepeating_ = false;
        btnRepeat_->SetLabel("Repeat");
        timingTx_->Enable(true);
        SetStatusText("");
    }
}

void mainFrame::OnRefresh(wxCommandEvent&) {
    lstPorts_->Freeze();
    lstPorts_->Clear();

    auto ports = EnumerateSerialPorts_Registry();
    // Uncomment the following code if you want to use SetupAPI for enumerating serial ports.
    // if (ports.empty()) {
    //     ports = EnumerateSerialPorts_SetupAPI();
    // }
    for (auto& p : ports) lstPorts_->Append(p);

    lstPorts_->Thaw();
    if (!ports.empty()) lstPorts_->SetSelection(0);
}

void mainFrame::OnSend(wxCommandEvent&) {
    if (!port_.isOpen()) {
        wxMessageBox("Connect to port first.", "Error", wxICON_ERROR, this);
        return;
    }
    const wxString text = txtTx_->GetValue();
    if (text.empty()) return;
    const std::string utf8 = std::string(text.ToUTF8());
    unsigned long written = 0;
    if (!port_.write(utf8.data(), static_cast<unsigned long>(utf8.size()), &written))
        wxMessageBox("Write failed.", "Error", wxICON_ERROR, this);
    if (cbCrlf_->IsChecked()) {
        const char crlf[] = "\n";
        port_.write(crlf, 1, &written);
    }
    txtTx_->Clear();
}

void mainFrame::OnRepeatTimer(wxTimerEvent&) {
	if (!isRepeating_)
        return;
    if (!port_.isOpen()) {
        btnRepeat_->SetLabel("Repeat");
        isRepeating_ = false;
        repeatTimer_.Stop();
        wxMessageBox("Connect to port first.", "Error", wxICON_ERROR, this);
        return;
    }
    const wxString text = macroTx_->GetValue();
    if (text.empty()) return;
    const std::string utf8 = std::string(text.ToUTF8());
    unsigned long written = 0;
    if (!port_.write(utf8.data(), static_cast<unsigned long>(utf8.size()), &written))
        wxMessageBox("Write failed.", "Error", wxICON_ERROR, this);
    if (macroCrlf_->IsChecked()) {
        const char crlf[] = "\n";
        port_.write(crlf, 1, &written);
    }
}

void mainFrame::OnChange(wxCommandEvent&) {
    if (!port_.isOpen())
        return;
    unsigned long baudRate = 0;
    if (!cbBaud_->GetValue().ToULong(&baudRate) || baudRate < 300 || baudRate > 256000) {
        wxMessageBox("Enter a valid baud rate (300~256000).", "Warning", wxICON_WARNING, this);
        return;
    }

    unsigned long byteSize = 0;
    if (!cbBytesize_->GetValue().ToULong(&byteSize) || byteSize < 4 || byteSize > 8) {
        wxMessageBox("Enter a valid byte size (4~8).", "Warning", wxICON_WARNING, this);
        return;
    }

    const wxString stopStr = cbStopbit_->GetValue();
    uint8_t stopBitsCfg = 1;
    if (stopStr == "1")   stopBitsCfg = 1;
    else if (stopStr == "1.5") stopBitsCfg = 15;
    else if (stopStr == "2")   stopBitsCfg = 2;
    else {
        wxMessageBox("Select valid stop bits (1, 1.5, or 2).", "Warning", wxICON_WARNING, this);
        return;
    }

    const wxString parityStr = cbParity_->GetValue();
    auto it = parityMap.find(parityStr);
    if (it == parityMap.end()) {
        wxMessageBox("Select a valid parity (None, Odd, Even, Mark, Space).", "Warning", wxICON_WARNING, this);
        return;
    }

    SerialConfig cfg;
    cfg.baud = baudRate;
    cfg.byteSize = static_cast<uint8_t>(byteSize);
    cfg.stopBits = stopBitsCfg;
    cfg.parity = it->second;
    cfg.dtr = true;
    cfg.rts = true;
    port_.changeConfig(cfg);
    SetStatusText(wxString::Format("Connected"));
}

void mainFrame::OnModeChar(wxCommandEvent&) { mode_ = Mode::Char; }
void mainFrame::OnModeHex(wxCommandEvent&) { mode_ = Mode::Hex; }
void mainFrame::OnClearTx(wxCommandEvent&) { txtTx_->Clear(); }
void mainFrame::OnClearRx(wxCommandEvent&) { txtRx_->Clear(); }

wxDEFINE_EVENT(EVT_RX_DATA, wxThreadEvent);
