//
// DialogUnzipLlama.hxx
//

#ifndef DIALOGUNZIPLLAMA_HXX
#define DIALOGUNZIPLLAMA_HXX

#include "DialogCmdRunner.hxx"

class DialogUnzipLlama : public DialogCmdRunner
{
    wxString m_llamaSourceDir;

    void LoadScript() override;

public:
    DialogUnzipLlama(wxWindow* parent, const wxString &llamaSorceDir);
    ~DialogUnzipLlama() = default;

};

#endif // DIALOGUNZIPLLAMA_HXX
