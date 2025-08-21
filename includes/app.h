#ifndef SERIAL_APP_H
#define SERIAL_APP_H

#include <wx/app.h>

class serialApp : public wxApp {
public:
    virtual bool OnInit();
};

#endif