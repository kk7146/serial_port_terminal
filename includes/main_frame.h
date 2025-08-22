#ifndef MAIN_FRAME_H
#define MAIN_FRAME_H

#include <wx/wx.h>
#include <wx/timer.h>
#include <thread>
#include "serial_port.h"

class mainFrame : public wxFrame {
public:
    explicit mainFrame(wxWindow* parent, wxWindowID id = wxID_ANY);

private:
    enum class Mode { Char, Hex };
    bool isRepeating_ = false;

    bool rxRunning_ = false;
    std::thread rxThread_;

    // Event handlers
    void OnClose(wxCloseEvent&);
    void OnConnectToggle(wxCommandEvent&);
    void OnRepeatToggle(wxCommandEvent&);
    void OnRefresh(wxCommandEvent&);
    void OnSend(wxCommandEvent&);
    void OnRepeatTimer(wxTimerEvent& e);
    void OnModeChar(wxCommandEvent&);
    void OnModeHex(wxCommandEvent&);
    void OnClearTx(wxCommandEvent&);
    void OnClearRx(wxCommandEvent&);
    void OnChange(wxCommandEvent&);

    // UI widgets
    wxTextCtrl* macroTx_{ nullptr };
    wxTextCtrl* timingTx_{ nullptr };
    wxTextCtrl* txtTx_{ nullptr };
    wxTextCtrl* txtRx_{ nullptr };
    wxRadioButton* rbChar_{ nullptr };
    wxRadioButton* rbHex_{ nullptr };
    wxCheckBox* cbListen_{ nullptr };
    wxCheckBox* cbCrlf_{ nullptr };
    wxCheckBox* macroCrlf_{ nullptr };
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

    std::string rxAccum_;
    wxStopWatch uiFlushClock_;
    const int kUiFlushMs = 30; // ~30Hz
};

wxDECLARE_EVENT(EVT_RX_DATA, wxThreadEvent);

#endif