#pragma once

namespace Croplines {
namespace Config {
constexpr double ZOOM_IN_RATE_DEFAULT = 1.2;
constexpr double ZOOM_OUT_RATE_DEFAULT = 1 / ZOOM_IN_RATE_DEFAULT;
constexpr double ZOOM_MIN_DEFAULT = 0.1;
constexpr double ZOOM_MAX_DEFAULT = 100;

extern double zoom_in_rate;
extern double zoom_out_rate;
extern double zoom_min;
extern double zoom_max;
}  // namespace Config
}  // namespace Croplines
