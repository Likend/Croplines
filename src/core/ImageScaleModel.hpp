#pragma once
#include <opencv2/opencv.hpp>
#include <wx/gdicmn.h>
#include <wx/wx.h>

namespace croplines {

class ImageScaleModel {
   public:
    constexpr static double ZOOM_IN_RATE  = 1.2;
    constexpr static double ZOOM_OUT_RATE = 1 / ZOOM_IN_RATE;
    constexpr static double ZOOM_MIN      = 0.1;
    constexpr static double ZOOM_MAX      = 100;

    wxSize imageSize;
    wxSize windowSize;
    wxSize scaledSize;
    // matrix trans
    // [[scale,  0,      offset_x,
    //   0,      scale,  offset_y ]]
    double  scale;
    wxPoint offset;

    ImageScaleModel(wxSize imageSize, wxSize windowSize, double scale);
    ImageScaleModel(wxSize imageSize, wxSize windowSize)
        : ImageScaleModel(imageSize, windowSize, GetScaleSuitesPage()) {}

    void Scale(double factor, wxPoint center);
    void Scale(double factor) { Scale(factor, wxPoint{} + windowSize / 2); }
    void ScaleTo(double scale, wxPoint center);
    void ScaleTo(double scale) { ScaleTo(scale, wxPoint{} + windowSize / 2); }
    void Move(wxPoint dr);
    void MoveToCenter();
    void OnWindowResize(wxSize windowSizeNew);
    void OnImageResize(wxSize imageSizeNew);

    [[nodiscard]] double GetScaleSuitesPage() const;
    [[nodiscard]] double GetScaleSuitesWidth() const;
    [[nodiscard]] double GetScaleSuitesHeight() const;

    [[nodiscard]] cv::Mat GetTransformMatrix() const;

    [[nodiscard]] wxRealPoint Transform(wxRealPoint point) const;
    [[nodiscard]] double      TransformX(double x) const { return scale * x + offset.x; }
    [[nodiscard]] double      TransformY(double y) const { return scale * y + offset.y; }
    [[nodiscard]] wxRealPoint ReverseTransform(wxRealPoint point) const {
        return (point - wxRealPoint(offset)) / scale;
    }
    [[nodiscard]] double ReverseTransformX(double x) const { return (x - offset.x) / scale; }
    [[nodiscard]] double ReverseTransformY(double y) const { return (y - offset.y) / scale; }

    [[nodiscard]] bool IsInsideImage(wxRealPoint worldPoint) const;

   private:
    void Clamp();
};
}  // namespace croplines
