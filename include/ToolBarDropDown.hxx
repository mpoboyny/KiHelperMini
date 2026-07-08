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
                        std::function<void()> onClick);

private:
    wxBitmap m_bitmap;
    wxString m_title;
    wxString m_subtitle;
    std::function<void()> m_onClick;
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
        std::function<void()> onClick;
    };

    explicit ToolBarDropDown(wxWindow* parent);

    void AddItem(const wxBitmap& bitmap,
                 const wxString& title,
                 const wxString& subtitle,
                 std::function<void()> onClick);

    void Popup(const wxPoint& screenPosition);

private:
    wxWindow* m_parent;
    std::vector<Item> m_items;
};

#endif // TOOLBARDROPDOWN_HXX
