#include "app.h"
#include <wx/wx.h>
#include "main_frame.h"

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit() {
    auto* frame = new mainFrame(nullptr);
    frame->Show();
    return true;
}
