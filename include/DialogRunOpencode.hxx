//
// DialogRunOpencode.hxx
//

#ifndef DIALOGRUNOPENCODE_HXX
#define DIALOGRUNOPENCODE_HXX

#include "DialogCmdRunner.hxx"

class ConfigFile;
class DialogRunOpencode : public DialogCmdRunner
{
    const ConfigFile &m_ConfFile;
    
    void SetupReplacements(ScriptReplacementsList &replacements);
    void LoadScript() override;
    void AddCustomButtons();
    void OnShowOpenCodeConfig(wxCommandEvent& event);

public:
    DialogRunOpencode(wxWindow* parent, const ConfigFile &confFile);
    ~DialogRunOpencode() = default;
};

#endif // DIALOGRUNOPENCODE_HXX