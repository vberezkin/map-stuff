#include <array>
#include <iostream>

#include "downloader.h"
#include "image.h"
#include "util.h"

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "Usage: access_token\n";
    return EXIT_FAILURE;
  }
  std::string accessToken{argv[1]};

  Uri navDay{Scheme::https, "api.mapbox.com",
             "/styles/v1/rimidalv/cjmxmvqmu12nb2snrq4gdlufg/static/"
             "37.5663,55.7355,7.55,0,0/"
             "512x512?access_token=" +
                 accessToken};
  Uri navNight{Scheme::https, "api.mapbox.com",
               "/styles/v1/rimidalv/cjmxmwso58gkb2snx7frm23tr/static/"
               "37.5663,55.7355,7.55,0,0/"
               "512x512?access_token=" +
                   accessToken};
  Uri relief{Scheme::https, "api.mapbox.com",
             "/styles/v1/rimidalv/cjftr9lie1pu22sqc05sdhp99/static/"
             "37.5663,55.7355,7.55,0,0/"
             "512x512?access_token=" +
                 accessToken};
  Uri streets{Scheme::https, "api.mapbox.com",
              "/styles/v1/mapbox/streets-v11/static/"
              "37.5663,55.7355,7.55,0,0/"
              "512x512?access_token=" +
                  accessToken};
  Uri satellite{Scheme::https, "api.mapbox.com",
                "/styles/v1/mapbox/satellite-v9/static/"
                "37.5663,55.7355,7.55,0,0/"
                "512x512?access_token=" +
                    accessToken};
  std::array us = {std::make_pair(navDay, Type::png),
                   std::make_pair(navNight, Type::png),
                   std::make_pair(streets, Type::png),
                   std::make_pair(satellite, Type::jpeg)};

  std::array<Image, us.size()> imgs;
  std::transform(
      us.begin(), us.end(), imgs.begin(), [](const std::pair<Uri, Type>& s) {
        auto fileName =
            std::to_string(std::hash<std::string>{}(s.first.target)) +
            extension(s.second);
        std::filesystem::path filePath{fileName};
        auto img = safeTry<Image>(load, filePath, s.second);
        if (isFailure(img)) {
          img = safeTry<Image>([uri = s.first, type = s.second, filePath]() {
            download(uri, filePath);
            return load(filePath, type);
          });
        }
        return std::get<Image>(img);
      });
  process(imgs);

  return 0;
}
