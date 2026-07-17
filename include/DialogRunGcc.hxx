//
// DialogRunGcc.hxx
//

#ifndef DIALOGRUNGCC_HXX
#define DIALOGRUNGCC_HXX

#include "DialogCmdRunner.hxx"

class DialogRunGcc : public DialogCmdRunner
{
    wxString m_llamaSourceDir;
    bool m_isCudaEnabled;

public:
    DialogRunGcc(wxWindow* parent, const wxString& llamaSorceDir, bool isCudaEnabled);
    ~DialogRunGcc() = default;

private:
    void SetupReplacements() override;
    void LoadScript() override;
};

#endif // DIALOGRUNGCC_HXX