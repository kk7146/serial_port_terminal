#include <wx/wx.h>
#include "main_frame.h"

class MyApp : public wxApp {
public:
    bool OnInit() override {
        auto* frame = new mainFrame(nullptr);
        frame->Show();
        return true;
    }
};

wxIMPLEMENT_APP(MyApp);
