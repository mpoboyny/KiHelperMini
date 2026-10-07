//
// ModelMetaData.cxx
//

#include "prc.hxx"
#include "ModelMetaData.hxx"
#include "llama.h"
#include "gguf.h"

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

static wxString ModelChatTemplateValue(const llama_model* model)
{
    auto toWxString = [](const char* text) {
        if (!text) {
            return wxString();
        }

        wxString result = wxString::FromUTF8(text);
        if (result.IsEmpty() && text[0] != '\0') {
            result = wxString::From8BitData(text);
        }
        return result;
    };

    const char* defaultTemplate = llama_model_chat_template(model, nullptr);
    const char* toolUseTemplate = llama_model_chat_template(model, "tool_use");
    wxString chatTemplate = toWxString(defaultTemplate);

    if (chatTemplate.IsEmpty() || chatTemplate == "chatml") {
        wxString toolUse = toWxString(toolUseTemplate);
        if (!toolUse.IsEmpty()) {
            chatTemplate = toolUse;
        }
    }

    return chatTemplate;
}

static wxString FallbackChatTemplateForModel(const ModelMetaData::Properties& properties, const wxString& modelPath)
{
    auto matchesMistral = [](const wxString& text) {
        wxString lowerText = text.Lower();
        return lowerText.Contains("mistral") || lowerText.Contains("codestral");
    };

    const auto nameIt = properties.find("general.name");
    if (nameIt != properties.end() && matchesMistral(nameIt->second.second.GetString())) {
        return wxString::FromUTF8(R"({%- if messages[0]['role'] == 'system' %}
    {%- set system_message = messages[0]['content'] %}
    {%- set loop_messages = messages[1:] %}
{%- else %}
    {%- set loop_messages = messages %}
{%- endif %}

{{- bos_token }}
{%- for message in loop_messages %}
    {%- if (message['role'] == 'user') != (loop.index0 % 2 == 0) %}
        {{- raise_exception('After the optional system message, conversation roles must alternate user/assistant/user/assistant/...') }}
    {%- endif %}
    {%- if message['role'] == 'user' %}
        {%- if loop.last and system_message is defined %}
            {{- '[INST] ' + system_message + '\n\n' + message['content'] + '[/INST]' }}
        {%- else %}
            {{- '[INST] ' + message['content'] + '[/INST]' }}
        {%- endif %}
    {%- elif message['role'] == 'assistant' %}
        {{- ' ' + message['content'] + eos_token}}
    {%- else %}
        {{- raise_exception('Only user and assistant roles are supported, with the exception of an initial optional system message!') }}
    {%- endif %}
{%- endfor %})");
    }

    if (matchesMistral(wxFileName(modelPath).GetFullName())) {
        return wxString::FromUTF8(R"({%- if messages[0]['role'] == 'system' %}
    {%- set system_message = messages[0]['content'] %}
    {%- set loop_messages = messages[1:] %}
{%- else %}
    {%- set loop_messages = messages %}
{%- endif %}

{{- bos_token }}
{%- for message in loop_messages %}
    {%- if (message['role'] == 'user') != (loop.index0 % 2 == 0) %}
        {{- raise_exception('After the optional system message, conversation roles must alternate user/assistant/user/assistant/...') }}
    {%- endif %}
    {%- if message['role'] == 'user' %}
        {%- if loop.last and system_message is defined %}
            {{- '[INST] ' + system_message + '\n\n' + message['content'] + '[/INST]' }}
        {%- else %}
            {{- '[INST] ' + message['content'] + '[/INST]' }}
        {%- endif %}
    {%- elif message['role'] == 'assistant' %}
        {{- ' ' + message['content'] + eos_token}}
    {%- else %}
        {{- raise_exception('Only user and assistant roles are supported, with the exception of an initial optional system message!') }}
    {%- endif %}
{%- endfor %})");
    }

    return wxString();
}

static const std::map<std::string, std::string>& GetFriendlyNames()
{
    static const std::map<std::string, std::string> names = {
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
    return names;
}

ModelMetaData::ModelMetaData(const wxString& modelPath)
    : m_loaded(false)
{
    m_loaded = Load(modelPath);
}

bool ModelMetaData::Load(const wxString& modelPath)
{
    llama_backend_init();
    llama_model_params params = llama_model_default_params();
    params.vocab_only = true;
    llama_model* model = llama_model_load_from_file(modelPath.ToUTF8().data(), params);
    if (!model) {
        llama_backend_free();
        m_error = "Failed to load model file: " + modelPath;
        return false;
    }

    const auto& friendlyNames = GetFriendlyNames();
    gguf_init_params ggufParams{};
    ggufParams.no_alloc = true;
    ggufParams.ctx = nullptr;
    gguf_context* gguf = gguf_init_from_file(modelPath.ToUTF8().data(), ggufParams);
    if (gguf) {
        const int64_t count = gguf_get_n_kv(gguf);
        for (int64_t i = 0; i < count; ++i) {
            const char* rawKey = gguf_get_key(gguf, i);
            if (!rawKey) continue;
            const std::string key(rawKey);
            const auto friendly = friendlyNames.find(key);
            const std::string label = friendly == friendlyNames.end() ? key : friendly->second;
            m_properties[key] = {label, wxVariant(ModelMetadataValue(gguf, i))};
        }
        gguf_free(gguf);
    } else {
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
            m_properties[keyName] = {friendly == friendlyNames.end() ? keyName : friendly->second, wxVariant(value)};
        }
    }

    const llama_vocab* vocab = llama_model_get_vocab(model);
    if (vocab) {
        m_properties["api_info.n_vocab"] = {"API Info Vocab", wxVariant(llama_vocab_n_tokens(vocab))};
        m_properties["api_info.n_ctx_train"] = {"API Info Context Train", wxVariant(llama_model_n_ctx_train(model))};
        m_properties["api_info.n_embd"] = {"API Info Embedding", wxVariant(llama_model_n_embd(model))};
        m_properties["api_info.n_layer"] = {"API Info Layers", wxVariant(llama_model_n_layer(model))};
        m_properties["api_info.model_size"] = {"API Info Size", wxVariant(wxString::Format("%llu bytes", static_cast<unsigned long long>(llama_model_size(model))))};
        m_properties["api_info.n_params"] = {"API Info Parameters", wxVariant(wxString::Format("%llu", static_cast<unsigned long long>(llama_model_n_params(model))))};
        wxString chatTemplate = ModelChatTemplateValue(model);
        if (!chatTemplate.IsEmpty()) {
            m_properties["tokenizer.chat_template"] = {"Tokenizer Chat Template", wxVariant(chatTemplate)};
        }
    }

    if (m_properties.find("tokenizer.chat_template") == m_properties.end()) {
        wxString fallbackChatTemplate = FallbackChatTemplateForModel(m_properties, modelPath);
        if (!fallbackChatTemplate.IsEmpty()) {
            m_properties["tokenizer.chat_template"] = {"Tokenizer Chat Template", wxVariant(fallbackChatTemplate)};
        }
    }

    llama_model_free(model);
    llama_backend_free();
    return true;
}
