#ifndef APP_H
#define APP_H

#include <wx/app.h>

class MyApp : public wxApp {
public:
    bool OnInit() override;
};

wxDECLARE_APP(MyApp);

#endif
