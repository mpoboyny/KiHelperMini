//
// ModelMetaData.hxx
//

#ifndef MODEL_METADATA_HXX
#define MODEL_METADATA_HXX

#include "prc.hxx"

class ModelMetaData
{
public:
    using Properties = std::map<std::string, std::pair<std::string, wxVariant>>;

    explicit ModelMetaData(const wxString& modelPath);

    bool IsLoaded() const { return m_loaded; }
    const wxString& GetError() const { return m_error; }
    const Properties& GetProperties() const { return m_properties; }

private:
    bool m_loaded;
    wxString m_error;
    Properties m_properties;

    bool Load(const wxString& modelPath);
};

#endif // MODEL_METADATA_HXX
