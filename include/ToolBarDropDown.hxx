//
// ToolBarDropDown.hxx
//

#ifndef TOOLBARDROPDOWN_HXX
#define TOOLBARDROPDOWN_HXX

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
