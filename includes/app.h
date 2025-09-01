#ifndef APP_H
#define APP_H

#include <wx/app.h>

class serialApp : public wxApp {
public:
    virtual bool OnInit();
};

#endif