#include "prc.hxx"

namespace frameMP 
{
}; // namespace frameMP

const int g_DefaultTxtSize = 10;
const wchar_t *g_DefaultTxtFontName = L"Sans";

const wchar_t *g_APP_NAME = L"KiHelper-Mini";
const char *g_APP_NAME_A = "KiHelper-Mini";

const wchar_t *g_APP_VERSION = L"0.0.1 Alpha";
const wchar_t *g_APP_DESCRIPTION = L"It is an Utility, that help you work with local AI models\n"
                                   L"This program use llama.cpp.\nBuilt with wxWidgets.\n"
                                   L"It is a free software, no license, no warranty.\n"
                                   L"You use it on own risk ;)\n";
const wchar_t *g_APP_COPYRIGHT = L"© 2026 KiHelper-Mini";
const wchar_t *g_APP_WEB = L"https://github.com/mpoboyny/KiHelperMini";

wxString g_WorkDir;

const wxString g_BinDir = []() {
    wxFileName exeFn(wxStandardPaths::Get().GetExecutablePath());
    return exeFn.GetPathWithSep();
}();

const wxString g_DefaultConfFileContent = wxString::FromUTF8(R"xml(<?xml version="1.0" encoding="UTF-8"?>
<Settings>
    <Systems>
        <System Name="Windows">
            <Models>
                <!-- Only / is to use as path separator --> 
                <!--Model File="C:/MyTools/llama/LocalKiModels/DeepSeek-R1-Distill-Qwen-1.5B.Q5_K_M.gguf" Current="true"/>
                <Model File="C:/MyTools/llama/LocalKiModels/Mistral-7B-Instruct-v0.3-Q5_K_M.gguf"  / -->
            </Models>
            <LLamaCpp>
                <Bin Path="C:/MyTools/llama-nocuda/" />
            </LLamaCpp>
        </System>
        <System Name="Linux">
            <Models>
                <!-- Model File="~/LocalKiModels/DeepSeek-R1-Distill-Qwen-1.5B.Q5_K_M.gguf" Current="true" />
                <Model File="~/LocalKiModels/Mistral-7B-Instruct-v0.3-Q5_K_M.gguf" />
                <Model File="~/LocalKiModels/Qwen2.5-14B-Instruct-Q4_K_M.gguf" / -->
            </Models>
            <LLamaCpp>
                <Bin Path="~/Dokumente/KiHelper-Mini/llama.cpp-source/llama.cpp/llama.cpp-master/" />
            </LLamaCpp>
            <LlamaChatParameters>
                <DefBin Path="~/Dokumente/KiHelper-Mini/llama.cpp-source/llama.cpp/llama.cpp-master/build_withCuda/bin/llama-cli" />
                
                <!-- Temperature: Controls randomness. Higher values (e.g., 1.0) make output more creative, lower values (e.g., 0.2) make it more deterministic. -->
                <Parameter Name="--temp" Value="0.7" />
                
                <!-- Top-K Sampling: Limits the next-token pool to the K most likely tokens. Modern llama.cpp uses a hyphen instead of an underscore. -->
                <Parameter Name="--top-k" Value="40" />
                
                <!-- Top-P Sampling (Nucleus): Limits token pool to a cumulative probability threshold. Modern llama.cpp uses a hyphen instead of an underscore. -->
                <Parameter Name="--top-p" Value="0.9" />
                
                <!-- Repeat Penalty: Discourages the model from repeating phrases. Modern llama.cpp uses a hyphen instead of an underscore. -->
                <Parameter Name="--repeat-penalty" Value="1.1" />
                
                <!-- CPU Threads: Number of physical CPU cores allocated for computing parts of the model not offloaded to the GPU. -->
                <Parameter Name="-t" Value="4" />
                
                <!-- GPU Layers: Number of neural network layers offloaded to the VRAM of your graphics card for faster hardware acceleration. -->
                <Parameter Name="-ngl" Value="24" /> 

                <!-- Chat Engine Template: Forces llama-cli to parse and apply the native Jinja template embedded inside Mistral GGUF metadata. -->
                <Parameter Name="--jinja" Value="" />

                <!-- Conversation Mode: Converts the text generator into a persistent, interactive chat session with memory. -->
                <Parameter Name="-cnv" Value="" />
            </LlamaChatParameters>
            <LlamaServerParameters>
                <!-- Path to your CUDA-compiled llama-server binary -->
                <DefBin Path="~/Dokumente/KiHelper-Mini/llama.cpp-source/llama.cpp/llama.cpp-master/build_withCuda/bin/llama-server" />
                
                <!-- CPU Threads: Match your physical core allocation from the CLI (8 cores) -->
                <Parameter Name="-t" Value="8" />
                
                <!-- GPU Layers: Keep your exact VRAM offloading layout (24 layers) for acceleration -->
                <Parameter Name="-ngl" Value="24" /> 

                <!-- Context Size: Set to 8192 for an extended local memory window without heavy performance drops -->
                <Parameter Name="-c" Value="8192" />

                <!-- Host Binding: Enforces IPv4 loopback bypasses slow local DNS resolution issues -->
                <Parameter Name="--host" Value="msi" />

                <!-- Port Allocation: Standard port for llama.cpp API communication -->
                <Parameter Name="--port" Value="8080" />
                
                <!-- Flash Attention: Highly recommended for local models with large context windows. -->
                <!-- It drastically reduces VRAM requirements for the context cache and speeds up generation. -->
                <Parameter Name="--flash-attn" Value="on" />
            </LlamaServerParameters>
       </System>
    </Systems>
</Settings>
)xml");

const wxString g_ConfDir = []() {
    wxFileName fn;
    fn.AssignDir(wxStandardPaths::Get().GetDocumentsDir());
    fn.AppendDir(g_APP_NAME);
    if (!fn.DirExists()) {
        fn.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
    }
    return fn.GetPathWithSep();
}();

const wxString g_ConfFileName = L"config.xml";

const wxString g_ConfFile = []() {
    wxString targetPath = g_ConfDir + g_ConfFileName;
    wxFileName targetFn(targetPath);
    if (!targetFn.FileExists()) {
        wxString srcPath = g_BinDir + g_ConfFileName;
        wxFileName srcFn(srcPath);
        if (srcFn.FileExists()) {
            wxCopyFile(srcPath, targetPath);
        }
        else {
            wxFile file(targetPath, wxFile::write);
            if (file.IsOpened()) {
                file.Write(g_DefaultConfFileContent);
                file.Close();
            }
        }
    }
    return targetPath;
}();

const wxString g_SrvLogFileName = L"server.log";

const wxString g_SrvLogFile = []() {
    return g_ConfDir + g_SrvLogFileName;
}();

const wxString g_ScriptDir = []() -> wxString {
    if (g_BinDir.IsEmpty()) {
        return wxEmptyString;
    }

    wxFileName scriptsFn;
    scriptsFn.AssignDir(g_BinDir);
    scriptsFn.AppendDir("scripts");

    if (!scriptsFn.DirExists()) {
        return wxEmptyString;
    }

    return scriptsFn.GetPathWithSep();
}();

const wxString g_ScriptExtraLlamaPath = []() -> wxString {
#if defined(__gnu_linux__)
    const wxString scriptFileName = "extract_llama.sh";
#elif defined(_WIN32)
    const wxString scriptFileName = "extract_llama.bat";
#endif

    if (g_ScriptDir.IsEmpty()) {
        return wxEmptyString;
    }

    wxString scriptPath = g_ScriptDir + scriptFileName;
    wxFileName scriptFn(scriptPath);
    if (!scriptFn.FileExists()) {
        return wxEmptyString;
    }

    return scriptPath;
}();

const wxString g_ScriptBuildCmakePath = []() -> wxString {
#if defined(__gnu_linux__)
    const wxString scriptFileName = "build_cmake.sh";
#elif defined(_WIN32)
    const wxString scriptFileName = "build_cmake.bat";
#endif

    if (g_ScriptDir.IsEmpty()) {
        return wxEmptyString;
    }

    wxString scriptPath = g_ScriptDir + scriptFileName;
    wxFileName scriptFn(scriptPath);
    if (!scriptFn.FileExists()) {
        return wxEmptyString;
    }

    return scriptPath;
}();

const wxString g_ScriptRunCmakePath = []() -> wxString {
#if defined(__gnu_linux__)
    const wxString scriptFileName = "run_cmake.sh";
#elif defined(_WIN32)
    const wxString scriptFileName = "run_cmake.bat";
#endif

    if (g_ScriptDir.IsEmpty()) {
        return wxEmptyString;
    }

    wxString scriptPath = g_ScriptDir + scriptFileName;
    wxFileName scriptFn(scriptPath);
    if (!scriptFn.FileExists()) {
        return wxEmptyString;
    }

    return scriptPath;
}();

const wxString g_ScriptRunLlamaPath = []() -> wxString {
#if defined(__gnu_linux__)
    const wxString scriptFileName = "run_llama.sh";
#elif defined(_WIN32)
    const wxString scriptFileName = "run_llama.bat";
#endif

    if (g_ScriptDir.IsEmpty()) {
        return wxEmptyString;
    }

    wxString scriptPath = g_ScriptDir + scriptFileName;
    wxFileName scriptFn(scriptPath);
    if (!scriptFn.FileExists()) {
        return wxEmptyString;
    }

    return scriptPath;
}();

const wxString g_ScriptRunLlamaPathServ = []() -> wxString {
#if defined(__gnu_linux__)
    const wxString scriptFileName = "run_llamaServer.sh";
#elif defined(_WIN32)
    const wxString scriptFileName = "run_llamaServer.bat";
#endif

    if (g_ScriptDir.IsEmpty()) {
        return wxEmptyString;
    }

    wxString scriptPath = g_ScriptDir + scriptFileName;
    wxFileName scriptFn(scriptPath);
    if (!scriptFn.FileExists()) {
        return wxEmptyString;
    }

    return scriptPath;
}();

void SetWorkingDir()
{
    TrFu;
    std::filesystem::path workDirPath;

#ifdef _WIN32
    wchar_t buffer[MAX_PATH];
    DWORD len = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    if (len != 0 && len < MAX_PATH) {
        workDirPath = std::filesystem::path(buffer).parent_path();
    }
#else
    char buffer[PATH_MAX];
    ssize_t len = ::readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (len > 0) {
        buffer[len] = '\0';
        workDirPath = std::filesystem::path(buffer).parent_path();
    }
#endif

    if (workDirPath.empty()) {
        workDirPath = std::filesystem::current_path();
    }

    std::error_code ec;
    std::filesystem::current_path(workDirPath, ec);
    
    std::filesystem::path currentPath = std::filesystem::current_path(ec);
    if (!ec) {
        g_WorkDir = wxString::FromUTF8(currentPath.string());
        if (!g_WorkDir.empty()) {
            wxFileName workFn(g_WorkDir);
            if (workFn.DirExists()) {
                g_WorkDir = workFn.GetPathWithSep();
            }
        }
    }
    TrStr(g_WorkDir);
}

int ShowGenericMessageBox(const wxString &message, const wxString &caption, int style, wxWindow *parent)
{
    wxGenericMessageDialog dlg(parent, message, caption, style);
    return dlg.ShowModal();
}

bool RemoveDirectoryContents(const wxString& path)
{
    wxDir dir(path);
    if (!dir.IsOpened()) {
        return false;
    }

    wxString name;
    bool cont = dir.GetFirst(&name, wxEmptyString, wxDIR_FILES | wxDIR_DIRS);
    while (cont) {
        wxString item = path + wxFileName::GetPathSeparator() + name;
        if (wxDirExists(item)) {
            if (!RemoveDirectoryContents(item) || !wxRmdir(item)) {
                return false;
            }
        } else {
            if (!wxRemoveFile(item)) {
                return false;
            }
        }
        cont = dir.GetNext(&name);
    }

    return true;
}

wxColour g_ColorLightGreen = wxColour(220, 245, 220); // Light green
