//
// DialogUnzipLlama.hxx
//

#ifndef DIALOGUNZIPLLAMA_HXX
#define DIALOGUNZIPLLAMA_HXX

#include "DialogCmdRunner.hxx"

class DialogUnzipLlama : public DialogCmdRunner
{
    wxString m_llamaSourceDir;
public:
    DialogUnzipLlama(wxWindow* parent, const wxString &llamaSorceDir);
    ~DialogUnzipLlama() = default;

private:
    void  SetupReplacements() override;
    void LoadScript() override;
};

#endif // DIALOGUNZIPLLAMA_HXX
