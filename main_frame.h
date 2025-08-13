
#pragma once

#include <wx/wx.h>
#include <wx/timer.h>
#include "serial_port.h"

class mainFrame : public wxFrame {
public:
    explicit mainFrame(wxWindow* parent, wxWindowID id = wxID_ANY);

private:
    enum class Mode { Char, Hex };

    // Event handlers
    void OnClose(wxCloseEvent&);
    void OnConnectToggle(wxCommandEvent&);
    void OnSend(wxCommandEvent&);
    void OnTimer(wxTimerEvent&);
    void OnModeChar(wxCommandEvent&);
    void OnModeHex(wxCommandEvent&);
    void OnClearTx(wxCommandEvent&);
    void OnClearRx(wxCommandEvent&);

    // UI widgets
    wxTextCtrl* txtTx_{ nullptr };
    wxTextCtrl* txtRx_{ nullptr };
    wxRadioButton* rbChar_{ nullptr };
    wxRadioButton* rbHex_{ nullptr };
    wxCheckBox* cbListen_{ nullptr };
    wxListBox* lstPorts_{ nullptr };
    wxComboBox* cbBaud_{ nullptr };
    wxButton* btnSend_{ nullptr };
    wxButton* btnClearTx_{ nullptr };
    wxButton* btnConnect_{ nullptr };
    wxButton* btnClearRx_{ nullptr };
    wxComboBox* cbParity_{ nullptr };
    wxComboBox* cbStopbit_{ nullptr };
    wxComboBox* cbBytesize_{ nullptr };

    // Logic
    SerialPort port_{};
    Mode mode_ = Mode::Char;
    wxTimer timer_{};
};
