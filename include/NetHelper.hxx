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

private:
    static bool HasZipSignature(const wxString& filePath);
    static bool CopyStreamToFile(wxInputStream& input, const wxString& targetPath);
    static bool DownloadWithRequest(wxWebRequestSync& request, const wxString& targetPath);
    static bool DownloadWithUrl(const wxString& url, const wxString& targetPath);
    static bool UpdateDownloadProgress(wxGauge* progress, wxStaticText* text, wxWebRequestSync& request);
};

#endif // NETHELPER_HXX
