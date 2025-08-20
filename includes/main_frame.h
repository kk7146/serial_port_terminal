
#pragma once

#include <wx/wx.h>
#include <wx/timer.h>
#include "serial_port.h"

class mainFrame : public wxFrame {
public:
    explicit mainFrame(wxWindow* parent, wxWindowID id = wxID_ANY);

private:
    enum class Mode { Char, Hex };
    bool isRepeating_ = false;
    int  repeatIntervalMs_ = 900;

    // Event handlers
    void OnClose(wxCloseEvent&);
    void OnConnectToggle(wxCommandEvent&);
    void OnRepeatToggle(wxCommandEvent&);
    void OnRefresh(wxCommandEvent&);
    void OnSend(wxCommandEvent&);
    void OnTimer(wxTimerEvent&);
    void OnRepeatTimer(wxTimerEvent& e);
    void OnModeChar(wxCommandEvent&);
    void OnModeHex(wxCommandEvent&);
    void OnClearTx(wxCommandEvent&);
    void OnClearRx(wxCommandEvent&);
    void OnChange(wxCommandEvent&);

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
    wxButton* btnRepeat_{ nullptr };
    wxButton* btnRefresh_{ nullptr };
    wxButton* btnClearRx_{ nullptr };
    wxComboBox* cbParity_{ nullptr };
    wxComboBox* cbStopbit_{ nullptr };
    wxComboBox* cbBytesize_{ nullptr };

    // Logic
    SerialPort port_;
    Mode mode_ = Mode::Char;
    wxTimer timer_{};
    wxTimer repeatTimer_;
};
