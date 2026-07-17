//
// DialogRunCmake.hxx
//

#ifndef DIALOGRUNCMAKE_HXX
#define DIALOGRUNCMAKE_HXX

#include "DialogCmdRunner.hxx"

class DialogRunCmake : public DialogCmdRunner
{
    wxString m_llamaSourceDir;
    wxString m_defOutDir;
    bool m_isCudaEnabled;
public:
    DialogRunCmake(wxWindow* parent, const wxString& llamaSorceDir, const wxString& defOutDir, bool isCudaEnabled);
    ~DialogRunCmake() = default;

private:
    void SetupReplacements() override;
    void LoadScript() override;
};

#endif // DIALOGRUNCMAKE_HXX