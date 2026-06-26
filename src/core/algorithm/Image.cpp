#include "core/algorithm/Image.hpp"

#include <cstddef>
#include <cstdint>
#include <generator>
#include <memory>
#include <queue>
#include <vector>

#include <wx/gdicmn.h>

// replace `cv::threshold(img_dst, img_dst, 0, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU);`
std::unique_ptr<uint8_t[]> croplines::ThresholdOstu(const uint8_t* gray_data, int width,
                                                    int height) {
    size_t total_pixels = static_cast<size_t>(width) * height;
    auto   binary_img   = std::make_unique<uint8_t[]>(total_pixels);

    // 计算直方图
    size_t histogram[256] = {0};
    for (size_t i = 0; i < total_pixels; i++) {
        histogram[gray_data[i]]++;
    }

    // 大津法计算最佳阈值
    size_t sum = 0;
    for (int t = 0; t < 256; t++) sum += t * histogram[t];

    double varMax        = 0;
    int    otsuThreshold = 0;
    size_t sumB          = 0;
    size_t wB            = 0;
    for (int t = 0; t < 256; t++) {
        wB += histogram[t];  // 背景像素数
        if (wB == 0) continue;

        size_t wF = total_pixels - wB;  // 前景像素数
        if (wF == 0) break;

        sumB += t * histogram[t];
        size_t sumF = (sum - sumB);

        double mB = static_cast<double>(sumB) / static_cast<double>(wB);  // 背景平均灰度
        double mF = static_cast<double>(sumF) / static_cast<double>(wF);  // 前景平均灰度

        // 计算类间方差
        double varBetween =
            static_cast<double>(wB) * static_cast<double>(wF) * (mB - mF) * (mB - mF);

        if (varBetween > varMax) {
            varMax        = varBetween;
            otsuThreshold = t;
        }
    }

    // 阈值化
    for (size_t i = 0; i < total_pixels; i++) {
        binary_img[i] = (gray_data[i] <= otsuThreshold) ? 0 : 255;
    }
    return binary_img;
}

std::unique_ptr<uint8_t[]> croplines::ConvertToGrayscale(const uint8_t* image, int width,
                                                         int height) {
    size_t total_pixels = static_cast<size_t>(width) * height;
    auto   gray_data    = std::make_unique<uint8_t[]>(total_pixels);

    for (size_t i = 0; i < total_pixels; i++) {
        uint8_t r = image[i * 3];
        uint8_t g = image[i * 3 + 1];
        uint8_t b = image[i * 3 + 2];
        // Gray = 0.299 * R + 0.587 * G + 0.114 * B
        uint8_t gray = (r * 77 + g * 150 + b * 29) / 256;
        gray_data[i] = gray;
    }

    return gray_data;
}

std::generator<croplines::FillArea> croplines::FloodFillArea(const uint8_t* binary_image, int width,
                                                             int height) {
    size_t            total_pixel = static_cast<size_t>(width) * height;
    std::vector<bool> visited(total_pixel, false);

    constexpr uint8_t target_pixel = 0;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            size_t idx = y * width + x;

            if (binary_image[idx] == target_pixel && !visited[idx]) {
                size_t area   = 0;
                int    c_xmin = x, c_xmax = x;
                int    c_ymin = y, c_ymax = y;

                std::queue<wxPoint> q;
                q.emplace(x, y);
                visited[idx] = true;

                while (!q.empty()) {
                    wxPoint c = q.front();
                    q.pop();
                    area++;

                    // 更新当前连通域的局部边界
                    if (c.x < c_xmin) c_xmin = c.x;
                    if (c.x > c_xmax) c_xmax = c.x;
                    if (c.y < c_ymin) c_ymin = c.y;
                    if (c.y > c_ymax) c_ymax = c.y;

                    // 遍历邻域
                    const static wxPoint ds[] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}};
                    for (wxPoint d : ds) {
                        wxPoint n = c + d;
                        if (n.x >= 0 && n.x < width && n.y >= 0 && n.y < height) {
                            int n_idx = n.y * width + n.x;
                            if (binary_image[n_idx] == target_pixel && !visited[n_idx]) {
                                visited[n_idx] = true;
                                q.push(n);
                            }
                        }
                    }
                }

                co_yield FillArea{
                    .area  = area,
                    .range = wxRect{wxPoint{c_xmin, c_ymin}, wxPoint{c_xmax + 1, c_ymax + 1}}};
            }
        }
    }
}
