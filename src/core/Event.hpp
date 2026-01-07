#include <wx/event.h>
#include <wx/thread.h>

namespace croplines {

wxDECLARE_EVENT(EVT_DOCUMENT_CHANGED, wxCommandEvent);

class DocumentEvent final : public wxCommandEvent {
   public:
    enum Type { Loaded, Saved, UndoDone, RedoDone, Modified };

    DocumentEvent(Type actionType) : wxCommandEvent(EVT_DOCUMENT_CHANGED), m_type(actionType) {}
    DocumentEvent(const DocumentEvent& other) = default;

    DocumentEvent* Clone() const override { return new DocumentEvent(*this); }

    Type GetActionType() const { return m_type; }

   private:
    Type m_type;
};

}  // namespace croplines
