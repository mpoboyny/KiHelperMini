//
// ModelInfoDialog.hxx
//

#ifndef MODEL_INFO_DIALOG_HXX
#define MODEL_INFO_DIALOG_HXX

#include "prc.hxx"

class ModelInfoDialog : public wxDialog
{
public:
    ModelInfoDialog(wxWindow* parent, const std::map<std::string, std::pair<std::string, wxVariant>>& metadata);

private:
    static constexpr size_t s_MaxValLenBeforeHighlight = 96;
    static const wxColour s_HighlightLongValue;
    wxGrid* m_grid;
    std::map<std::string, std::pair<std::string, wxVariant>> m_metadata;
    std::vector<std::string> m_rowKeys;

    void OnCellDoubleClicked(wxGridEvent& event);
};

#endif // MODEL_INFO_DIALOG_HXX
