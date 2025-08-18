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

    while (true) {
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
    : wxFrame(parent, id, "Serial Terminal", wxDefaultPosition, wxSize(420, 450)) {
    SetMinSize(wxSize(420, 450));
    SetMaxSize(wxSize(420, 450));

    auto* panel = new wxPanel(this);

    // Groups
    new wxStaticBox(panel, wxID_ANY, "Transmit data", wxPoint(16, 16), wxSize(243, 192));
    new wxStaticBox(panel, wxID_ANY, "Received data", wxPoint(16, 224), wxSize(368, 152));
    new wxStaticBox(panel, wxID_ANY, "Port Setup", wxPoint(264, 16), wxSize(120, 192));

	// Control buttons
    btnSend_ = new wxButton(panel, wxID_ANY, "Send", wxPoint(176, 176), wxSize(72, 23));
    btnClearTx_ = new wxButton(panel, wxID_ANY, "Clear", wxPoint(96, 176), wxSize(72, 23));
    btnConnect_ = new wxButton(panel, wxID_ANY, "Connect", wxPoint(280, 176), wxSize(88, 23));
    btnRefresh_ = new wxButton(panel, wxID_ANY, "Refresh", wxPoint(280, 150), wxSize(88, 23));
    btnClearRx_ = new wxButton(panel, wxID_ANY, "Clear", wxPoint(160, 244), wxSize(72, 23));

	// Text send receive
    txtTx_ = new wxTextCtrl(panel, wxID_ANY, "", wxPoint(24, 90), wxSize(224, 80), wxTE_MULTILINE);
    txtRx_ = new wxTextCtrl(panel, wxID_ANY, "", wxPoint(32, 280), wxSize(336, 80), wxTE_MULTILINE | wxTE_READONLY);

    // Receive mode
    rbChar_ = new wxRadioButton(panel, wxID_ANY, "Char", wxPoint(32, 248), wxDefaultSize, wxRB_GROUP);
    rbHex_ = new wxRadioButton(panel, wxID_ANY, "Hex", wxPoint(104, 248));
    rbChar_->SetValue(true);

    //Listening check box
    cbListen_ = new wxCheckBox(panel, wxID_ANY, "Listening", wxPoint(240, 248));
    cbListen_->SetValue(true);

    // Port
    lstPorts_ = new wxListBox(panel, wxID_ANY, wxPoint(272, 37), wxSize(104, 110));

    new wxStaticText(panel, wxID_ANY, "Parity Bit", wxPoint(20, 35));
    cbParity_ = new wxComboBox(panel, wxID_ANY, "", wxPoint(20, 55), wxSize(60, 20));
	cbParity_->Append("none");
    cbParity_->Append("even");
    cbParity_->Append("odd");
	cbParity_->Append("mark");
	cbParity_->Append("space");
	cbParity_->SetValue("none");

    new wxStaticText(panel, wxID_ANY, "Stop Bit", wxPoint(85, 35));
    cbStopbit_ = new wxComboBox(panel, wxID_ANY, "", wxPoint(85, 55), wxSize(50, 20));
	cbStopbit_->Append("1");
	cbStopbit_->Append("1.5");
	cbStopbit_->Append("2");
	cbStopbit_->SetValue("1");

    new wxStaticText(panel, wxID_ANY, "Byte Size", wxPoint(140, 35));
    cbBytesize_ = new wxComboBox(panel, wxID_ANY, "", wxPoint(140, 55), wxSize(50, 20));
	cbBytesize_->Append("5");
	cbBytesize_->Append("6");
	cbBytesize_->Append("7");
	cbBytesize_->Append("8");
	cbBytesize_->SetValue("8");

    new wxStaticText(panel, wxID_ANY, "Baud Rate", wxPoint(195, 35));
    cbBaud_ = new wxComboBox(panel, wxID_ANY, "", wxPoint(195, 55), wxSize(60, 20));
    cbBaud_->Append("1200");
    cbBaud_->Append("4800");
    cbBaud_->Append("9600");
    cbBaud_->Append("19200");
    cbBaud_->Append("38400");
    cbBaud_->Append("57600");
    cbBaud_->SetValue("9600");

    CreateStatusBar(1);

    // Timer
    timer_.SetOwner(this);
    timer_.Start(100, wxTIMER_CONTINUOUS);

    // Bindings
    btnSend_->Bind(wxEVT_BUTTON, &mainFrame::OnSend, this);
    btnClearTx_->Bind(wxEVT_BUTTON, &mainFrame::OnClearTx, this);
    btnConnect_->Bind(wxEVT_BUTTON, &mainFrame::OnConnectToggle, this);
    btnRefresh_->Bind(wxEVT_BUTTON, &mainFrame::OnRefresh, this);
    btnClearRx_->Bind(wxEVT_BUTTON, &mainFrame::OnClearRx, this);
    rbChar_->Bind(wxEVT_RADIOBUTTON, &mainFrame::OnModeChar, this);
    rbHex_->Bind(wxEVT_RADIOBUTTON, &mainFrame::OnModeHex, this);
    Bind(wxEVT_TIMER, &mainFrame::OnTimer, this);
    Bind(wxEVT_CLOSE_WINDOW, &mainFrame::OnClose, this);

    Centre();
}

void mainFrame::OnClose(wxCloseEvent& e) {
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

        unsigned long baud = 0;
        if (!cbBaud_->GetValue().ToULong(&baud) || baud < 300 || baud > 460800) {
            wxMessageBox("Enter a valid baud rate (300~460800).", "Warning", wxICON_WARNING, this);
            return;
        }

        unsigned long byteSizeUL = 0;
        if (!cbBytesize_->GetValue().ToULong(&byteSizeUL) || byteSizeUL < 4 || byteSizeUL > 8) {
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
        cfg.baud = baud;
        cfg.byteSize = static_cast<uint8_t>(byteSizeUL);
        cfg.stopBits = stopBitsCfg;
        cfg.parity = it->second;
        cfg.dtr = true;
        cfg.rts = true;

        if (!port_.open(portName, cfg)) {
            wxMessageBox(wxString::Format("Failed to open %s @ %lu.", portName, baud),
                "Error", wxICON_ERROR, this);
            return;
        }

        btnConnect_->SetLabel("Disconnect");
        SetStatusText(wxString::Format("Connected: %s @ %lu", portName, baud));
    }
    else {
        port_.close();
        btnConnect_->SetLabel("Connect");
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
}

void mainFrame::OnTimer(wxTimerEvent&) {
    if (!cbListen_->IsChecked() || !port_.isOpen()) return;

    char buf[256];
    const unsigned long r = port_.read(buf, sizeof(buf));
    if (r == 0) return;

    if (mode_ == Mode::Char) {
        wxString s = wxString::FromUTF8(buf, r);
        if (!s.empty()) txtRx_->AppendText(s);
    }
    else {
        wxString line;
        line.reserve(r * 5);
        for (unsigned long i = 0; i < r; ++i)
            line.Append(wxString::Format("0x%02X ", static_cast<unsigned char>(buf[i])));
        txtRx_->AppendText(line);
    }
}

void mainFrame::OnModeChar(wxCommandEvent&) { mode_ = Mode::Char; }
void mainFrame::OnModeHex(wxCommandEvent&) { mode_ = Mode::Hex; }
void mainFrame::OnClearTx(wxCommandEvent&) { txtTx_->Clear(); }
void mainFrame::OnClearRx(wxCommandEvent&) { txtRx_->Clear(); }
