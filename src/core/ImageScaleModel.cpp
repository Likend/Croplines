#include "core/ImageScaleModel.hpp"

using namespace croplines;

ImageScaleModel::ImageScaleModel(wxSize imageSize, wxSize windowSize, double scale)
    : imageSize(imageSize), windowSize(windowSize), scale(scale) {
    scaledSize = imageSize * scale;
    MoveToCenter();
}

ImageScaleModel::ImageScaleModel(wxSize imageSize, wxSize windowSize)
    : imageSize(imageSize), windowSize(windowSize) {
    scale      = GetScaleSuitesPage();
    scaledSize = imageSize * scale;
    MoveToCenter();
}

void ImageScaleModel::Clamp() {
    wxSize border = windowSize - scaledSize;
    if (border.x < 0) {
        offset.x = std::clamp(offset.x, border.x, 0);
    } else {
        offset.x = std::clamp(offset.x, 0, border.x);
    }
    if (border.y < 0) {
        offset.y = std::clamp(offset.y, border.y, 0);
    } else {
        offset.y = std::clamp(offset.y, 0, border.y);
    }
}

void ImageScaleModel::Scale(double factor, wxPoint center) {
    double scaleNew = scale * factor;
    if (scaleNew > ZOOM_MAX) {
        factor = ZOOM_MAX / scale;
    } else if (scaleNew < ZOOM_MIN) {
        factor = ZOOM_MIN / scale;
    }
    scale *= factor;
    offset     = center + factor * (offset - center);
    scaledSize = imageSize * scale;
    Clamp();
}

void ImageScaleModel::ScaleTo(double scale, wxPoint center) {
    scale         = std::clamp(scale, ZOOM_MIN, ZOOM_MAX);
    double factor = scale / this->scale;
    this->scale   = scale;
    offset        = center + factor * (offset - center);
    scaledSize    = imageSize * scale;
    Clamp();
}

void ImageScaleModel::Move(wxPoint dr) {
    offset += dr;
    Clamp();
}

void ImageScaleModel::MoveToCenter() { offset = wxPoint{} + (windowSize - scaledSize) / 2; }

void ImageScaleModel::OnWindowResize(wxSize windowSizeNew) {
    offset += (windowSizeNew - windowSize) / 2;
    windowSize = windowSizeNew;
    Clamp();
}

void ImageScaleModel::OnImageResize(wxSize imageSizeNew) {
    offset += (imageSize - imageSizeNew) / 2;
    imageSize  = imageSizeNew;
    scaledSize = scale * imageSize;
    Clamp();
}

double ImageScaleModel::GetScaleSuitesWidth() const {
    return static_cast<double>(windowSize.GetWidth()) / imageSize.GetWidth();
}

double ImageScaleModel::GetScaleSuitesHeight() const {
    return static_cast<double>(windowSize.GetHeight()) / imageSize.GetHeight();
}

double ImageScaleModel::GetScaleSuitesPage() const {
    return std::min(GetScaleSuitesWidth(), GetScaleSuitesHeight());
}

cv::Mat ImageScaleModel::GetTransformMatrix() const {
    return (cv::Mat_<double>(2, 3) << scale, 0, offset.x, 0, scale, offset.y);
}

bool ImageScaleModel::IsInsideImage(wxRealPoint worldPoint) const {
    wxRealPoint imagePoint = ReverseTransform(worldPoint);
    return 0 <= imagePoint.x && imagePoint.x <= imageSize.GetWidth() && 0 <= imagePoint.y &&
           imagePoint.y <= imageSize.GetHeight();
}
wxRealPoint croplines::ImageScaleModel::Transform(wxRealPoint point) const {
    return scale * point + wxRealPoint(offset);
}
