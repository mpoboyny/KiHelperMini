//
// NetHelper.hxx
//

#ifndef NETHELPER_HXX
#define NETHELPER_HXX

class NetHelper
{
public:
    static bool IsUrlAvaliable(const wxString& url);
    static bool Download(wxWindow *caller, const wxString& url, const wxString& to);
};

#endif // NETHELPER_HXX
