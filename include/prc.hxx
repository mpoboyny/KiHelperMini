//
// prc.hxx
//

#ifndef PRC_HXX
#define PRC_HXX

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include <wx/aboutdlg.h>
#include <wx/richtext/richtextctrl.h>
#include <wx/richtext/richtextbuffer.h>
#include <wx/richtext/richtextxml.h>
#include <wx/stdpaths.h>
#include <wx/xml/xml.h>
#include <wx/sstream.h>
#include <wx/stc/stc.h> // wxStyledTextCtrl header
#include <wx/dynlib.h>
#include <wx/filename.h>
#include <wx/notebook.h>
#include <wx/artprov.h>
#include <wx/event.h>
#include <wx/thread.h>
#include <wx/msgqueue.h>
#include <wx/listctrl.h>
#include <wx/grid.h>
#include <wx/richtooltip.h>
#include <wx/filedlg.h>
#include <wx/generic/filedlgg.h> 
#include <wx/file.h>
#include <wx/accel.h>
#include <wx/datetime.h>
#include <wx/dc.h>
#include <wx/settings.h>
#include <wx/tokenzr.h>
#include <wx/url.h>
#include <wx/webrequest.h>
#include <wx/filefn.h>
#include <wx/stattext.h>
#include <wx/dir.h>
#include <wx/statline.h>
#include <wx/variant.h>
#include <wx/clipbrd.h>
#include <wx/dataobj.h>
#include <wx/utils.h>
#include <wx/evtloop.h>
#include <wx/popupwin.h>
#include <wx/dcbuffer.h>
#include <wx/textfile.h>
#include <wx/display.h>

#include <memory>
#include <utility>
#include <type_traits>
#include <cctype>
#include <algorithm>
#include <fstream>
#include <map>
#include <cstring>
#include <string>
#include <sstream>
#include <list>
#include <vector>
#include <thread>
#include <atomic>
#include <filesystem>
#include <limits.h>
#include <chrono>
#include <regex>
#include <cstdio>
#include <array>
#include <mutex>

#if defined(__gnu_linux__)
#include <unistd.h>
#endif

#ifdef _WIN32 
#   ifndef MP_UNICODE
#       define MP_UNICODE
#   endif
#endif

// frameMP
#ifndef _MPTRACE_
#define _MPTRACE_
#endif

#include "str.hxx"
#include "trace.hxx"

using namespace frameMP;

extern const int               g_DefaultTxtSize;
extern const wchar_t           *g_DefaultTxtFontName;

extern const wchar_t           *g_APP_NAME;
extern const char              *g_APP_NAME_A;
extern wxString                g_WorkDir;
extern const wxString          g_ConfDir;
extern const wxString          g_ConfFileName;
extern const wxString          g_ConfFile;
extern const wxString          g_ScriptDir;
extern const wxString          g_ScriptExtraLlamaPath;
extern const wxString          g_ScriptBuildCmakePath;
extern const wxString          g_ScriptRunCmakePath;
extern const wxString          g_ScriptRunLlamaPath;

extern wxColour g_ColorLightGreen;

enum GuiStatus
{
    GuiStatus_Info,
    GuiStatus_Warn,
    GuiStatus_Erro
};

enum MenusIDs
{
    ID_SETTINGS   = wxID_HIGHEST + 50,
    ID_MODEL_INFO = wxID_HIGHEST + 51,
    ID_RESET      = wxID_HIGHEST + 52,
    ID_BUILD      = wxID_HIGHEST + 53,
    ID_BUILD_LLAMA = wxID_HIGHEST + 54
};

enum ButtIDs
{
    ID_SHOW_CONF_FOLDER  = wxID_HIGHEST + 1000,
    ID_DONLOAD,
    ID_CANCEL_DOWNLOAD,
    ID_UNZIP_LLAMA,
    ID_OPEN_LLAMA_SOURCE,
    ID_SHOW_FILES,
    ID_CHECK_CMAKE,
    ID_DOWNLOAD_LLAMA,
    ID_CHECK_GCC,
    ID_BUILD_CMAKE,
    ID_BUILD_GCC,
    ID_SELECT_CHAT_FILE,
    ID_RUN_CHAT,
    ID_RUN_CHAT_HELP,
    ID_SELECT_SERVER_FILE,
    ID_RUN_SERVER,
    ID_RUN_SERVER_HELP,
};

typedef std::list<std::pair<wxString, wxString>> ScriptReplacementsList;

void SetWorkingDir();
int ShowGenericMessageBox(const wxString& message, const wxString& caption, int style, wxWindow* parent);
bool RemoveDirectoryContents(const wxString& path);

#endif
