//
// DialogOpencodeJson.hxx
//

#ifndef DIALOGOPENCODEJSON_HXX
#define DIALOGOPENCODEJSON_HXX

#include "DialogCmdRunner.hxx"

class ConfigFile;
class DialogOpencodeJson : public DialogCmdRunner
{
    const ConfigFile &m_ConfFile;

    void LoadScript() override;
    void SetupJsonHighlighting();
    void SetupReplacements(ScriptReplacementsList &replacements);

public:
    DialogOpencodeJson(wxWindow* parent, const ConfigFile &confFile);
    ~DialogOpencodeJson() = default;
};

#endif // DIALOGOPENCODEJSON_HXX