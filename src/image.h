#pragma once

#include <filesystem>

#include <boost/gil.hpp>
#include <boost/gil/extension/io/png.hpp>

namespace gil = boost::gil;

using Image = gil::rgb8_image_t;
using Pixel = gil::rgb8_pixel_t;

enum class Type { jpeg, png };

Image load(const std::filesystem::path& path, Type type);

std::string extension(Type type);

double accelDecelInterpolator(double x);
double sineInterpolator(double x);
double sineInterpolatorXY(double x, double y);

Pixel mixPixels(const Pixel& a, const Pixel& b, double weight);

template <size_t N>
Pixel mapPixel(double x, double y, const std::array<Pixel, N>& s) {
  auto r = std::sqrt((x - .5) * (x - .5) + (y - .5) * (y - .5)*2);
  auto sine = mixPixels(s[0], s[1], sineInterpolator(r));

  return mixPixels(s[2], sine, r / 2.);
  // auto j = sineInterpolatorXY(x, y);
  // auto k = sineInterpolatorXY(x, -y);
  // Pixel e = mixPixels(s[1], s[0], j);
  // Pixel d = mixPixels(s[1], s[0], k);
  ////Pixel e = mixPixels(s[3], s[2], j);
  ////Pixel d = mixPixels(s[3], s[2], k);
  // return mixPixels(e, d, .5);
}

template <size_t N>
void process(const std::array<Image, N>& srcImgs) {
  static_assert(N > 0);
  const auto& first = srcImgs.front();
  gil::point_t dims{first.width(), first.height()};
  Image dstImg{dims};
  std::array<Image::const_view_t, N> srcViews;
  std::transform(srcImgs.begin(), srcImgs.end(), srcViews.begin(),
                 [](const Image& img) { return gil::const_view(img); });
  auto& dstView = gil::view(dstImg);
  for (int y = 0; y < dims.y; ++y) {
    for (int x = 0; x < dims.x; ++x) {
      auto dx = double(x) / double(dims.x);
      auto dy = double(y) / double(dims.y);
      std::array<Pixel, N> srcPixs;
      std::transform(srcViews.begin(), srcViews.end(), srcPixs.begin(),
                     [x, y](const Image::const_view_t& v) { return v(x, y); });
      dstView(x, y) = mapPixel(dx, dy, srcPixs);
    }
  }
  gil::write_view("out.png", dstView, gil::png_tag());
}
