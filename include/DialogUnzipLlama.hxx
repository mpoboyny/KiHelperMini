//
// DialogUnzipLlama.hxx
//

#ifndef DIALOGUNZIPLLAMA_HXX
#define DIALOGUNZIPLLAMA_HXX

#include "DialogCmdRunner.hxx"

class DialogUnzipLlama : public DialogCmdRunner
{
public:
    DialogUnzipLlama(wxWindow* parent);
    ~DialogUnzipLlama() = default;

private:
    void LoadScript() override;
};

#endif // DIALOGUNZIPLLAMA_HXX
