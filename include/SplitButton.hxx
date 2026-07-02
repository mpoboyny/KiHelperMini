//
// SplitButton.hxx
//

#ifndef SPLITBUTTON_HXX
#define SPLITBUTTON_HXX

class SplitButton : public wxPanel
{
public:
    static const int s_defHeight = 32;
    static const int s_defButtHeight = s_defHeight +3;
    SplitButton(wxWindow* parent, wxWindowID idMain, const wxString& labelMain, wxWindowID idArrow = wxID_ANY);

    wxButton* GetMainButton();
    wxBitmapButton* GetArrowButton();

private:
    wxButton* m_main{nullptr};
    wxBitmapButton* m_arrow{nullptr};
};

#endif // SPLITBUTTON_HXX
