//
// DialogUnzipLlama.hxx
//

#ifndef DIALOGUNZIPLLAMA_HXX
#define DIALOGUNZIPLLAMA_HXX

#include <wx/dialog.h>

class wxStyledTextCtrl;

class DialogUnzipLlama : public wxDialog
{
public:
    DialogUnzipLlama(wxWindow* parent);
    ~DialogUnzipLlama() = default;

private:
    void SetupHighlighting();
    void LoadScript();

    wxStyledTextCtrl* m_textCtrl;
};

#endif // DIALOGUNZIPLLAMA_HXX
