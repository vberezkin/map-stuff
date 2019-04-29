#pragma once

#include <filesystem>
#include <string>

enum class Scheme { http, https };

struct Uri {
  Scheme scheme;
  std::string host;
  std::string target;
};

void download(const Uri& uri, const std::filesystem::path& file);
