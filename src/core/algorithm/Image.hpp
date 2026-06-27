#include <cstddef>
#include <cstdint>
#include <memory>

#include <wx/gdicmn.h>

namespace croplines {
std::unique_ptr<uint8_t[]> ConvertToGrayscale(const uint8_t* image, int width, int height);
std::unique_ptr<uint8_t[]> ThresholdOstu(const uint8_t* gray_data, int width, int height);

struct FillArea {
    size_t area{};
    wxRect range;
};

std::generator<FillArea> FloodFillArea(const uint8_t* binary_image, int width, int height);
}  // namespace croplines
