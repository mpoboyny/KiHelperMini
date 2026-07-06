//
// ToolBarDropDown.hxx
//

#ifndef TOOLBARDROPDOWN_HXX
#define TOOLBARDROPDOWN_HXX

/*------ ToolBarDropDownItem -----*/

class ToolBarDropDownItem : public wxPanel
{
public:
    ToolBarDropDownItem(wxWindow* parent,
                        const wxBitmap& bitmap,
                        const wxString& title,
                        const wxString& subtitle,
                        int commandId);

    static void PostToolbarCommand(wxWindow* source, int id);

private:
    int m_commandId;
    wxBitmap m_bitmap;
    wxString m_title;
    wxString m_subtitle;
    bool m_hover = false;

    void Highlight(bool on);
    void OnPaint(wxPaintEvent&);
    void OnEnterWindow(wxMouseEvent& event);
    void OnLeaveWindow(wxMouseEvent& event);
    void OnClick(wxMouseEvent& event);
};

/*------ ToolBarDropDownPopup -----*/

class ToolBarDropDown
{
public:
    struct Item
    {
        wxBitmap bitmap;
        wxString title;
        wxString subtitle;
        int commandId = wxID_ANY;
    };

    explicit ToolBarDropDown(wxWindow* parent);

    void AddItem(const wxBitmap& bitmap,
                 const wxString& title,
                 const wxString& subtitle,
                 int commandId);

    void Popup(const wxPoint& screenPosition);

private:
    wxWindow* m_parent;
    std::vector<Item> m_items;
};

#endif // TOOLBARDROPDOWN_HXX
