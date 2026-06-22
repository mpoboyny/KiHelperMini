//
// MainDialogStatusBar.hxx
//

#ifndef MAINDIALOGSTATUSBAR_HXX
#define MAINDIALOGSTATUSBAR_HXX

#include "prc.hxx"

class CMainDialogStatusBar : public wxStatusBar
{
    public:
        explicit CMainDialogStatusBar(wxWindow* parent);
        void ShowMessage(const wxString& msg, unsigned int field = 0);
};

#endif // MAINDIALOGSTATUSBAR_HXX
