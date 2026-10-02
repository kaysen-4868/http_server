#pragma once

#include <cctype>
#include <string>
#include <unordered_map>

inline const std::string &getMimeType(const std::string &path) {
  static const std::unordered_map<std::string, std::string> mime = {
      {".html", "text/html"},        {".htm", "text/html"},
      {".css", "text/css"},          {".js", "application/javascript"},
      {".json", "application/json"}, {".png", "image/png"},
      {".jpg", "image/jpeg"},        {".jpeg", "image/jpeg"},
      {".gif", "image/gif"},         {".svg", "image/svg+xml"},
      {".ico", "image/x-icon"},      {".txt", "text/plain"},
      {".pdf", "application/pdf"},   {".xml", "application/xml"},
      {".woff", "font/woff"},        {".woff2", "font/woff2"},
      {".ttf", "font/ttf"},
  };
  static const std::string default_type = "application/octet-stream";

  size_t dot = path.find_last_of('.');
  if (dot == std::string::npos)
    return default_type;

  std::string ext = path.substr(dot);
  for (char &c : ext)
    c = static_cast<char>(std::tolower((unsigned char)c));

  auto it = mime.find(ext);
  return (it != mime.end()) ? it->second : default_type;
}