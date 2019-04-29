#include <boost/gil/extension/io/jpeg.hpp>
#include <boost/gil/io/read_image.hpp>

#include "image.h"

Image load(const std::filesystem::path& path, Type type) {
  Image img;
  switch (type) {
    case Type::jpeg:
      gil::read_image(path.native(), img, gil::jpeg_tag());
      break;
    case Type::png:
      gil::read_image(path.native(), img, gil::png_tag());
      break;
  }
  return img;
}

std::string extension(Type type) {
  switch (type) {
    case Type::jpeg:
      return ".jpeg";
    case Type::png:
      return ".png";
  }
}

double accelDecelInterpolator(double x) {
  return (1. - abs(x - 0.5)) * (x - 0.5) * 2. + 0.5;
}

double sineInterpolator(double x) {
  return std::sin(x * 60.) / 2. + .5;
}

double sineInterpolatorXY(double x, double y) {
  auto a = x + y;
  return sineInterpolator(a);
}

namespace {
template <typename Color>
void mixPixels(Color color,
               const Pixel& a,
               const Pixel& b,
               double weight,
               Pixel& res) {
  gil::get_color(res, color) = gil::get_color(a, color) * weight +
                               gil::get_color(b, color) * (1. - weight);
}
}  // namespace

Pixel mixPixels(const Pixel& a, const Pixel& b, double weight) {
  Pixel res;
  mixPixels(gil::red_t(), a, b, weight, res);
  mixPixels(gil::green_t(), a, b, weight, res);
  mixPixels(gil::blue_t(), a, b, weight, res);
  return res;
}
