#include <cstddef>
#include <cstdint>
#include <generator>
#include <memory>

#include <wx/gdicmn.h>

namespace croplines {
std::unique_ptr<uint8_t[]> ConvertToGrayscale(const uint8_t* img, int width, int height);
std::unique_ptr<uint8_t[]> ThresholdOstu(const uint8_t* gray_img, int width, int height);

struct FillArea {
    size_t area{};
    wxRect range;
};

std::generator<FillArea> FloodFillArea(const uint8_t* binary_img, int width, int height);
}  // namespace croplines
