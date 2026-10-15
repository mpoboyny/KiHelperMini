//
// DialogRunOpencode.hxx
//

#ifndef DIALOGRUNOPENCODE_HXX
#define DIALOGRUNOPENCODE_HXX

#include "DialogCmdRunner.hxx"

class DialogRunOpencode : public DialogCmdRunner
{
    void LoadScript() override;

public:
    DialogRunOpencode(wxWindow* parent);
    ~DialogRunOpencode() = default;
};

#endif // DIALOGRUNOPENCODE_HXX