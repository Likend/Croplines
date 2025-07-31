#include <wx/event.h>
#include <wx/gdicmn.h>
#include <wx/wx.h>

#include <cstddef>
#include <opencv2/opencv.hpp>
#include <optional>

namespace Croplines {

class ImageScaleModel {
   public:
    wxSize imageSize;
    wxSize windowSize;
    wxSize scaledSize;
    // matrix trans
    // [[scale,  0,      offset_x,
    //   0,      scale,  offset_y ]]
    double scale;
    wxPoint offset;

    ImageScaleModel(wxSize imageSize, wxSize windowSize);
    ImageScaleModel(wxSize imageSize, wxSize windowSize, double scale);

   private:
    bool Clip();

   public:
    void Scale(double factor, wxPoint center);
    void Scale(double factor);
    void ScaleTo(double scale, wxPoint center);
    void ScaleTo(double scale);
    void Move(wxPoint dr);

    void WindowResize(wxSize windowSizeNew);

    double GetScaleSuitesPage() const;
    double GetScaleSuitesWidth() const;
    double GetScaleSuitesHeight() const;
};

class Canvas : public wxPanel {
   private:
    wxSize currentSize;
    wxPoint imageCoord;

    struct ImageBundle {
        wxImage imageSrc;
        cv::UMat uimageSrc;
        wxImage imageDst;
        ImageScaleModel scaleModel;
    };

    std::optional<ImageBundle> imageInfo;

   public:
    Canvas(wxWindow* parent, wxWindowID id);
    ~Canvas();
    void SetImage(wxImage image);

   private:
    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnMouseWheel(wxMouseEvent& event);

    std::optional<wxPoint> mouse_drag_start;
    void OnMouseLeftDown(wxMouseEvent& event);
    void OnMouseLeftUp(wxMouseEvent& event);
    void OnMouseMotion(wxMouseEvent& event);

    void OnScroll(wxScrollWinEvent& event);

    wxDECLARE_EVENT_TABLE();
};
}  // namespace Croplines