//
// BuildToolBar.hxx
//

#ifndef BUILDTOOLBAR_HXX
#define BUILDTOOLBAR_HXX

class BuildToolBar : public wxToolBar
{
public:
    BuildToolBar(wxWindow* parent);

    template <typename T>
    void UseHandler(int toolId, void (T::*handler)(wxCommandEvent&))
    {
        T* owner = wxDynamicCast(GetParent(), T);
        wxCHECK_RET(owner != nullptr, "BuildToolBar parent does not match handler owner");

        m_handlers[toolId] = [this, owner, handler, toolId]() {
            wxCommandEvent event(wxEVT_TOOL, toolId);
            event.SetEventObject(this);
            (owner->*handler)(event);
        };

        Bind(wxEVT_TOOL, &BuildToolBar::OnTool, this, toolId);
    }

    bool RunHandler(int toolId);

private:
    void OnTool(wxCommandEvent& event);

    std::map<int, std::function<void()>> m_handlers;
};

#endif // BUILDTOOLBAR_HXX