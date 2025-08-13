#include "main_frame.h"
#include <wx/msgdlg.h>

mainFrame::mainFrame(wxWindow* parent, wxWindowID id)
    : wxFrame(parent, id, "Serial Terminal", wxDefaultPosition, wxSize(420, 450)) {
    SetMinSize(wxSize(420, 450));
    SetMaxSize(wxSize(420, 450));

    auto* panel = new wxPanel(this);

    // Groups
    new wxStaticBox(panel, wxID_ANY, "Transmit data", wxPoint(16, 16), wxSize(243, 192));
    new wxStaticBox(panel, wxID_ANY, "Received data", wxPoint(16, 224), wxSize(368, 152));
    new wxStaticBox(panel, wxID_ANY, "Port Setup", wxPoint(264, 16), wxSize(120, 192));

    // Controls
    btnSend_ = new wxButton(panel, wxID_ANY, "Send", wxPoint(176, 176), wxSize(72, 23));
    btnClearTx_ = new wxButton(panel, wxID_ANY, "Clear", wxPoint(96, 176), wxSize(72, 23));
    btnConnect_ = new wxButton(panel, wxID_ANY, "Connect", wxPoint(280, 176), wxSize(88, 23));
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
    lstPorts_ = new wxListBox(panel, wxID_ANY, wxPoint(272, 37), wxSize(104, 133));
    lstPorts_->Append("COM1");
    lstPorts_->Append("COM2");
    lstPorts_->Append("COM3");
    lstPorts_->Append("COM4");
    lstPorts_->Append("COM5");
	lstPorts_->Append("COM6");
	lstPorts_->Append("COM7");
	lstPorts_->Append("COM8");
	lstPorts_->Append("COM9");
	lstPorts_->Append("COM10");
	lstPorts_->Append("COM11");
	lstPorts_->Append("COM12");
	lstPorts_->Append("COM13");

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
        wxString portName = lstPorts_->GetString(sel);
        unsigned long baud = 0;
        if (!cbBaud_->GetValue().ToULong(&baud) || baud < 300 || baud > 460800) {
            wxMessageBox("Enter a valid baud rate (300~460800).", "Warning", wxICON_WARNING, this);
            return;
        }
        if (!port_.open(portName, baud)) {
            wxMessageBox(wxString::Format("%s is not available.", portName), "Error", wxICON_ERROR, this);
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

void mainFrame::OnSend(wxCommandEvent&) {
    if (!port_.isOpen()) {
        wxMessageBox("Connect to port first.", "Error", wxICON_ERROR, this);
        return;
    }
    const wxString text = txtTx_->GetValue();
    if (text.empty()) return;
    const std::string utf8 = std::string(text.ToUTF8());
    unsigned long written = 0;
    if (!port_.write(utf8.data(), static_cast<unsigned long>(utf8.size()), &written)) {
        wxMessageBox("Write failed.", "Error", wxICON_ERROR, this);
    }
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
        for (unsigned long i = 0; i < r; ++i) {
            line.Append(wxString::Format("0x%02X ", static_cast<unsigned char>(buf[i])));
        }
        txtRx_->AppendText(line);
    }
}

void mainFrame::OnModeChar(wxCommandEvent&) { mode_ = Mode::Char; }
void mainFrame::OnModeHex(wxCommandEvent&) { mode_ = Mode::Hex; }
void mainFrame::OnClearTx(wxCommandEvent&) { txtTx_->Clear(); }
void mainFrame::OnClearRx(wxCommandEvent&) { txtRx_->Clear(); }
