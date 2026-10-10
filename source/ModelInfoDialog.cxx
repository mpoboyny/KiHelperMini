//
// ModelInfoDialog.cxx
//

#include "prc.hxx"
#include "ModelInfoDialog.hxx"
#include "../resources/app.xpm"

const wxColour ModelInfoDialog::s_HighlightLongValue(220, 255, 220);

ModelInfoDialog::ModelInfoDialog(wxWindow* parent, const std::map<std::string, std::pair<std::string, wxVariant>>& metadata)
    : wxDialog(parent, wxID_ANY, "Model Metadata", wxDefaultPosition, wxSize(900, 800), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
    , m_grid(nullptr)
    , m_metadata(metadata)
{
    SetIcon(wxIcon(app_xpm));

    auto nameIt = metadata.find("general.name");
    if (nameIt != metadata.end()) {
        const wxString name = nameIt->second.second.GetString();
        if (!name.IsEmpty()) {
            SetTitle(GetTitle() + " (" + name + ")");
        }
    }

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    m_grid = new wxGrid(this, wxID_ANY);
    m_grid->CreateGrid(metadata.size(), 2);
    m_grid->SetColLabelValue(0, "Key");
    m_grid->SetColLabelValue(1, "Value");
    m_grid->HideRowLabels();
    m_grid->EnableEditing(false);
    m_grid->EnableDragRowSize(false);
    m_grid->SetSelectionMode(wxGrid::wxGridSelectRows);
    m_grid->SetCellHighlightPenWidth(0);

    int row = 0;
    for (const auto& item : metadata) {
        wxString label = wxString::FromUTF8(item.second.first.data(), item.second.first.length());
        if (label.IsEmpty() && !item.second.first.empty()) {
            label = wxString::From8BitData(item.second.first.data(), item.second.first.length());
        }
        wxString value = item.second.second.GetString();
        m_rowKeys.push_back(item.first);
        if (value.length() > s_MaxValLenBeforeHighlight) {
            label += " ...";
            m_grid->SetCellBackgroundColour(row, 0, s_HighlightLongValue);
            m_grid->SetCellBackgroundColour(row, 1, s_HighlightLongValue);
            m_grid->SetCellTextColour(row, 0, *wxBLACK);
            m_grid->SetCellTextColour(row, 1, *wxBLACK);
            value = value.Left(64) + "...";
        }
        m_grid->SetCellValue(row, 0, label);
        m_grid->SetCellValue(row, 1, value);
        row++;
    }

    m_grid->AutoSizeColumn(0);
    m_grid->SetColSize(1, 250);
    m_grid->Bind(wxEVT_SIZE, [this](wxSizeEvent& event) {
        event.Skip();
        const int availableWidth = m_grid->GetClientSize().GetWidth() - m_grid->GetColSize(0);
        if (availableWidth > 0) {
            m_grid->SetColSize(1, availableWidth);
        }
    });
    m_grid->Bind(wxEVT_GRID_CELL_LEFT_DCLICK, &ModelInfoDialog::OnCellDoubleClicked, this);
    sizer->Add(m_grid, 1, wxEXPAND | wxALL, 10);
    sizer->Add(CreateButtonSizer(wxOK), 0, wxALIGN_RIGHT | wxRIGHT | wxBOTTOM, 10);
    SetSizer(sizer);
    SetSize(wxSize(900, 800));
    SetMinSize(wxSize(600, 400));
    Layout();
}

void ModelInfoDialog::OnCellDoubleClicked(wxGridEvent& event)
{
    const int row = event.GetRow();
    if (row < 0 || static_cast<size_t>(row) >= m_rowKeys.size() ||
        !m_grid->GetCellValue(row, 0).EndsWith(" ...")) {
        event.Skip();
        return;
    }

    const auto item = m_metadata.find(m_rowKeys[row]);
    if (item == m_metadata.end()) {
        return;
    }

    wxString label = m_grid->GetCellValue(row, 0);
    if (label.EndsWith(" ...")) label = label.Left(label.length() - 4);
    wxString fullValue = item->second.second.GetString();
    fullValue.Replace(wxString(wxT('\0')), "");

    wxString displayValue = fullValue;
    if (displayValue.Length() > 32768) {
        displayValue = displayValue.Left(32768) + "\n\n... [Truncated for display; use Copy to Clipboard for the full value.]";
    }

    wxDialog valueDialog(this, wxID_ANY, label, wxDefaultPosition, wxSize(600, 400), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    wxTextCtrl* text = new wxTextCtrl(&valueDialog, wxID_ANY, displayValue, wxDefaultPosition, wxDefaultSize,
                                     wxTE_MULTILINE | wxTE_READONLY | wxBORDER_NONE | wxTE_RICH2);
    text->SetFont(wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    sizer->Add(text, 1, wxEXPAND | wxALL, 10);

    wxBoxSizer* buttons = new wxBoxSizer(wxHORIZONTAL);
    wxButton* copyButton = new wxButton(&valueDialog, wxID_ANY, "Copy to Clipboard");
    copyButton->Bind(wxEVT_BUTTON, [fullValue](wxCommandEvent&) {
        if (wxTheClipboard->Open()) {
            wxTheClipboard->SetData(new wxTextDataObject(fullValue));
            wxTheClipboard->Close();
        }
    });
    buttons->Add(copyButton, 0, wxALL, 5);
    buttons->AddStretchSpacer();
    buttons->Add(valueDialog.CreateButtonSizer(wxOK), 0, wxALL, 5);
    sizer->Add(buttons, 0, wxEXPAND | wxBOTTOM, 5);
    valueDialog.SetSizer(sizer);
    valueDialog.ShowModal();
}
