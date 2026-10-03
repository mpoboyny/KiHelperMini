//
// MyDialog.cxx
//

#include "prc.hxx"
#include "MainDialogMenu.hxx"
#include "ConfigFile.hxx"
#include "ConfigDlg.hxx"
#include "MainDialogPanel.hxx"
#include "MainRunChatPanel.hxx"
#include "MainRunServerPanel.hxx"

#include "MainDialog.hxx"
#include "MainDialogStatusBar.hxx"
#include "BuildLlama.hxx"
#include "BuildDialog.hxx"
#include "DialogSuggestion.hxx"
#include "ModelInfoDialog.hxx"
#include "llama.h"
#include "gguf.h"

#include "../resources/app.xpm"

static wxString ModelMetadataValue(const gguf_context* context, int64_t keyId)
{
    std::ostringstream value;
    const gguf_type type = gguf_get_kv_type(context, keyId);
    if (type == GGUF_TYPE_ARRAY) {
        const gguf_type elementType = gguf_get_arr_type(context, keyId);
        const size_t count = gguf_get_arr_n(context, keyId);
        value << "[";
        for (size_t i = 0; i < count; ++i) {
            if (i) value << ", ";
            if (elementType == GGUF_TYPE_STRING) {
                const char* item = gguf_get_arr_str(context, keyId, i);
                value << '"' << (item ? item : "") << '"';
                continue;
            }

            const void* data = gguf_get_arr_data(context, keyId);
            if (!data) break;
            switch (elementType) {
                case GGUF_TYPE_UINT8: value << static_cast<unsigned>(static_cast<const uint8_t*>(data)[i]); break;
                case GGUF_TYPE_INT8: value << static_cast<int>(static_cast<const int8_t*>(data)[i]); break;
                case GGUF_TYPE_UINT16: value << static_cast<const uint16_t*>(data)[i]; break;
                case GGUF_TYPE_INT16: value << static_cast<const int16_t*>(data)[i]; break;
                case GGUF_TYPE_UINT32: value << static_cast<const uint32_t*>(data)[i]; break;
                case GGUF_TYPE_INT32: value << static_cast<const int32_t*>(data)[i]; break;
                case GGUF_TYPE_UINT64: value << static_cast<const uint64_t*>(data)[i]; break;
                case GGUF_TYPE_INT64: value << static_cast<const int64_t*>(data)[i]; break;
                case GGUF_TYPE_FLOAT32: value << static_cast<const float*>(data)[i]; break;
                case GGUF_TYPE_FLOAT64: value << static_cast<const double*>(data)[i]; break;
                case GGUF_TYPE_BOOL: value << (static_cast<const int8_t*>(data)[i] ? "true" : "false"); break;
                default: value << "<unsupported array type>"; break;
            }
            if (value.tellp() > 16 * 1024 * 1024) {
                value << " ... <array display truncated>";
                break;
            }
        }
        value << "] (Total: " << count << ")";
    } else {
        switch (type) {
            case GGUF_TYPE_UINT8: value << static_cast<unsigned>(gguf_get_val_u8(context, keyId)); break;
            case GGUF_TYPE_INT8: value << static_cast<int>(gguf_get_val_i8(context, keyId)); break;
            case GGUF_TYPE_UINT16: value << gguf_get_val_u16(context, keyId); break;
            case GGUF_TYPE_INT16: value << gguf_get_val_i16(context, keyId); break;
            case GGUF_TYPE_UINT32: value << gguf_get_val_u32(context, keyId); break;
            case GGUF_TYPE_INT32: value << gguf_get_val_i32(context, keyId); break;
            case GGUF_TYPE_UINT64: value << gguf_get_val_u64(context, keyId); break;
            case GGUF_TYPE_INT64: value << gguf_get_val_i64(context, keyId); break;
            case GGUF_TYPE_FLOAT32: value << gguf_get_val_f32(context, keyId); break;
            case GGUF_TYPE_FLOAT64: value << gguf_get_val_f64(context, keyId); break;
            case GGUF_TYPE_BOOL: value << (gguf_get_val_bool(context, keyId) ? "true" : "false"); break;
            case GGUF_TYPE_STRING: value << gguf_get_val_str(context, keyId); break;
            default: value << "<unsupported metadata type>"; break;
        }
    }

    const std::string text = value.str();
    wxString result = wxString::FromUTF8(text.data(), text.length());
    if (result.IsEmpty() && !text.empty()) {
        result = wxString::From8BitData(text.data(), text.length());
    }
    return result;
}

/* static */
const wxSize CMainDialog::s_defMinSize = wxSize(1000, 700);


CMainDialog::CMainDialog()
    : wxFrame(NULL, wxID_ANY, g_APP_NAME, wxDefaultPosition, s_defMinSize)
    , m_ConfFile(std::make_unique<ConfigFile>())
{
    TrFu;
    
   SetMinSize(s_defMinSize);
   #ifdef _WIN32
        // Explicitly load the icon named APP_ICON from resources
        SetIcon(wxICON(APP_ICON));
    #else
        SetIcon(wxIcon(app_xpm));
    #endif

    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));

    SetFont(wxFont(wxFontInfo(g_DefaultTxtSize).FaceName(g_DefaultTxtFontName)));
    
    m_menuBar = new CMainDialogMenu();
    SetMenuBar(m_menuBar);

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    
    m_mainPanel = new CMainDialogPanel(this, m_ConfFile.get());
    mainSizer->Add(m_mainPanel, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 5);

    m_runPanel = new CMainRunChatPanel(this, *m_ConfFile);
    mainSizer->Add(m_runPanel, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);

    m_runServerPanel = new CMainRunServerPanel(this, *m_ConfFile);
    mainSizer->Add(m_runServerPanel, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);

    SetSizer(mainSizer);

    // create and register status bar so wxFrame lays it out at the bottom
    m_statusBar = new CMainDialogStatusBar(this);
    SetStatusBar(m_statusBar);

    Bind(wxEVT_MENU, &CMainDialog::OnExit, this, wxID_EXIT);
    Bind(wxEVT_MENU, &CMainDialog::OnReset, this, ID_RESET);
    Bind(wxEVT_MENU, &CMainDialog::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &CMainDialog::OnSettings, this, ID_SETTINGS);
    Bind(wxEVT_MENU, &CMainDialog::OnBuild, this, ID_BUILD);
    Bind(wxEVT_MENU, &CMainDialog::OnBuildLlama, this, ID_BUILD_LLAMA);
    Bind(wxEVT_MENU, &CMainDialog::OnParameterSuggestion, this, ID_PARAMETER_SUGGESTION);
    Bind(wxEVT_MENU, &CMainDialog::OnModelInfo, this, ID_MODEL_INFO);
    Bind(wxEVT_CLOSE_WINDOW, &CMainDialog::OnClose, this);

    wxString msg;
    if (m_ConfFile->HaveError(msg))
        m_statusBar->ShowMessage(GuiStatus::GuiStatus_Erro, msg);
    else
        m_statusBar->ShowReady();
}

void CMainDialog::OnExit(wxCommandEvent &event)
{
    Close(true);
}

void CMainDialog::OnReset(wxCommandEvent &event)
{
    // ask user for confirmation before deleting current config
    wxString ask = wxString::Format("Are you sure you want to delete current\n%s\nfile and overwrite with default?", g_ConfFile);
    int answer = ShowGenericMessageBox(ask, "Confirm Reset", wxYES_NO | wxNO_DEFAULT | wxICON_WARNING, this);
    TrVar(answer);
    if (answer != wxID_YES) {
        return;
    }

    if (wxFileName::FileExists(g_ConfFile)) {
        wxRemoveFile(g_ConfFile);
    }

    wxExecute(wxStandardPaths::Get().GetExecutablePath());
    Close(true);
}

void CMainDialog::OnAbout(wxCommandEvent& event) 
{
     wxAboutDialogInfo aboutInfo;
    
    aboutInfo.SetName(g_APP_NAME);
    aboutInfo.SetVersion(g_APP_VERSION);
    aboutInfo.SetDescription(g_APP_DESCRIPTION);
    aboutInfo.SetCopyright(g_APP_COPYRIGHT);
    aboutInfo.SetWebSite(g_APP_WEB);
    aboutInfo.SetIcon(wxIcon(app_xpm));
    wxAboutBox(aboutInfo, this);
}

void CMainDialog::OnSettings(wxCommandEvent& event) 
{
    TrFu;
    ConfigDlg* dlg = new ConfigDlg(this);
    if (dlg->ShowModal() == wxID_OK) {
        if (dlg->IsModified()) {
            dlg->SaveConfig();
            wxExecute(wxStandardPaths::Get().GetExecutablePath());
            dlg->Destroy();
            Close(true);
            return;
        }
    }
    dlg->Destroy();
}

void CMainDialog::OnBuild(wxCommandEvent& event)
{
    ShowGenericMessageBox("Build is not implemented yet.", "Build", wxOK | wxICON_INFORMATION, this);
}

void CMainDialog::OnBuildLlama(wxCommandEvent& event)
{
#if defined(_WIN32) || defined(__gnu_linux__)
    BuildDialog winDlg(this, m_mainPanel->GetLlamaBinPath());
    winDlg.ShowModalLike();
#else
    BuildLlama dlg(this);
    dlg.ShowModal();
    dlg.Destroy();
#endif
}

void CMainDialog::OnParameterSuggestion(wxCommandEvent& event)
{
    DialogSuggestion dlg(this, *m_ConfFile);
    dlg.ShowModal();
}

void CMainDialog::OnModelInfo(wxCommandEvent& event)
{
    ConfigFile::ModelList models = m_ConfFile->GetModels();
    auto currentModel = std::find_if(models.begin(), models.end(), [](const ConfigFile::ModelInfo& model) {
        return model.Current;
    });
    if (currentModel == models.end()) {
        ShowGenericMessageBox("No current model is configured.", "Model Info", wxOK | wxICON_INFORMATION, this);
        return;
    }

    llama_backend_init();
    llama_model_params params = llama_model_default_params();
    params.vocab_only = true;
    llama_model* model = llama_model_load_from_file(currentModel->Path.ToUTF8().data(), params);
    if (!model) {
        llama_backend_free();
        ShowGenericMessageBox("Failed to load the configured model file:\n" + currentModel->Path, "Model Info", wxOK | wxICON_ERROR, this);
        return;
    }

    std::map<std::string, std::pair<std::string, wxVariant>> metadata;
    const std::map<std::string, std::string> friendlyNames = {
        {"general.architecture", "General Architecture"}, {"general.name", "General Name"},
        {"general.parameter_count", "General Parameter Count"}, {"general.type", "General Type"},
        {"general.file_type", "General File Type"}, {"general.quantization_version", "General Quantization Version"},
        {"general.license", "General License"}, {"general.organization", "General Organization"},
        {"general.basename", "General Basename"}, {"general.size_label", "General Size Label"},
        {"general.tags", "General Tags"}, {"general.languages", "General Languages"},
        {"general.finetune", "General Finetune"}, {"general.source.url", "General Source URL"},
        {"general.license.link", "General License Link"},
        {"tokenizer.ggml.model", "Tokenizer Model"}, {"tokenizer.ggml.tokens", "Tokenizer Tokens"},
        {"tokenizer.ggml.token_type", "Tokenizer Token Type"}, {"tokenizer.ggml.scores", "Tokenizer Scores"},
        {"tokenizer.ggml.merges", "Tokenizer Merges"}, {"tokenizer.ggml.bos_token_id", "Tokenizer BOS Token ID"},
        {"tokenizer.ggml.eos_token_id", "Tokenizer EOS Token ID"}, {"tokenizer.ggml.eog_tokens", "Tokenizer EOG Tokens"},
        {"tokenizer.ggml.add_bos_token", "Tokenizer Add BOS Token"}, {"tokenizer.ggml.add_eos_token", "Tokenizer Add EOS Token"},
        {"tokenizer.chat_template", "Tokenizer Chat Template"}, {"tokenizer.ggml.pre", "Tokenizer Pre-tokenizer"}
    };

    gguf_init_params ggufParams{};
    ggufParams.no_alloc = true;
    ggufParams.ctx = nullptr;
    gguf_context* gguf = gguf_init_from_file(currentModel->Path.ToUTF8().data(), ggufParams);
    if (gguf) {
        const int64_t count = gguf_get_n_kv(gguf);
        for (int64_t i = 0; i < count; ++i) {
            const char* rawKey = gguf_get_key(gguf, i);
            if (!rawKey) continue;
            const std::string key(rawKey);
            const auto friendly = friendlyNames.find(key);
            const std::string label = friendly == friendlyNames.end() ? key : friendly->second;
            metadata[key] = {label, wxVariant(ModelMetadataValue(gguf, i))};
        }
        gguf_free(gguf);
    } else {
        // Fall back to llama.cpp's scalar metadata API if direct GGUF reading fails.
        const int32_t count = llama_model_meta_count(model);
        for (int32_t i = 0; i < count; ++i) {
            char key[1024] = {};
            const int32_t keyLength = llama_model_meta_key_by_index(model, i, key, sizeof(key));
            if (keyLength < 0) continue;
            const std::string keyName(key, std::min(static_cast<size_t>(keyLength), sizeof(key) - 1));
            const int32_t valueLength = llama_model_meta_val_str_by_index(model, i, nullptr, 0);
            std::vector<char> buffer(valueLength > 0 && valueLength < 16 * 1024 * 1024 ? valueLength + 1 : 1, '\0');
            wxString value = valueLength < 0 ? "<GGUF array; unavailable>" : "<value too large>";
            if (valueLength >= 0 && valueLength < 16 * 1024 * 1024 &&
                llama_model_meta_val_str_by_index(model, i, buffer.data(), buffer.size()) >= 0) {
                value = wxString::FromUTF8(buffer.data());
            }
            const auto friendly = friendlyNames.find(keyName);
            metadata[keyName] = {friendly == friendlyNames.end() ? keyName : friendly->second, wxVariant(value)};
        }
    }

    // Add runtime-derived entries that are not ordinary GGUF metadata fields.
    const llama_vocab* vocab = llama_model_get_vocab(model);
    if (vocab) {
        const int32_t tokenCount = llama_vocab_n_tokens(vocab);
        metadata["api_info.n_vocab"] = {"API Info Vocab", wxVariant(tokenCount)};
        metadata["api_info.n_ctx_train"] = {"API Info Context Train", wxVariant(llama_model_n_ctx_train(model))};
        metadata["api_info.n_embd"] = {"API Info Embedding", wxVariant(llama_model_n_embd(model))};
        metadata["api_info.n_layer"] = {"API Info Layers", wxVariant(llama_model_n_layer(model))};
        metadata["api_info.model_size"] = {"API Info Size", wxVariant(wxString::Format("%llu bytes", static_cast<unsigned long long>(llama_model_size(model))))};
        metadata["api_info.n_params"] = {"API Info Parameters", wxVariant(wxString::Format("%llu", static_cast<unsigned long long>(llama_model_n_params(model))))};
        const char* templateText = llama_model_chat_template(model, nullptr);
        if (templateText) {
            metadata["tokenizer.chat_template"] = {"Tokenizer Chat Template", wxVariant(wxString::FromUTF8(templateText))};
        }
    }

    llama_model_free(model);
    llama_backend_free();

    ModelInfoDialog dlg(this, metadata);
    dlg.ShowModal();
}

void CMainDialog::OnClose(wxCloseEvent& event) 
{
    if (wxTheApp && wxTheApp->IsMainLoopRunning()) {
        wxTheApp->ExitMainLoop();
    }
    Destroy();
}
