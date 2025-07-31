#include <cstdint>
#include <utility>

namespace Croplines {

class ImageScaleModel {
    using coord_int = int64_t;
    using coord_type = std::pair<coord_int, coord_int>;
    // using size_int
    using size_type = std::pair<uint64_t, uint64_t>;

   private:
    double scale;
    coord_type left_top;
    size_type view_size;
    size_type image_size;
    size_type scaled_size;

   public:
    constexpr inline double getScale() { return scale; }
    constexpr inline void setScale(double scale) { this->scale = scale; }

    constexpr inline coord_type getLeftTop() { return left_top; }
    constexpr inline void setLeftTop(coord_int x, coord_int y) {
        left_top = {x, y};
    }
    constexpr inline void setLeftTopX(coord_int x) { left_top.first = x; }
    constexpr inline void setLeftTopY(coord_int y) { left_top.second = y; }

    constexpr inline size_type getViewSize() { return view_size; }
    constexpr inline void setViewSize()
};

}  // namespace Croplines
