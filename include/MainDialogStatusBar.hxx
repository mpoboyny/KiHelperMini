//
// MainDialogStatusBar.hxx
//

#ifndef MAINDIALOGSTATUSBAR_HXX
#define MAINDIALOGSTATUSBAR_HXX

#include "prc.hxx"
#include <map>

class CMainDialogStatusBar : public wxStatusBar
{
    public:
        // map: status -> font, colour and prefix text
        struct StatusFontInfo {
            wxFont font;
            wxColour colour;
            wxString prefix;
        };

        static const std::map<GuiStatus, StatusFontInfo>& Fonts();

        explicit CMainDialogStatusBar(wxWindow* parent);
        void ShowReady();
        void ShowMessage(GuiStatus status, const wxString& msg);
};

#endif // MAINDIALOGSTATUSBAR_HXX
