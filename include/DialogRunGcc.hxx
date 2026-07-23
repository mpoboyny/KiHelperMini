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

    void SetupReplacements(ScriptReplacementsList &replacements);
    void LoadScript() override;
public:
    DialogRunGcc(wxWindow* parent, const wxString& llamaSorceDir, bool isCudaEnabled);
    ~DialogRunGcc() = default;

private:
    
    
};

#endif // DIALOGRUNGCC_HXX
