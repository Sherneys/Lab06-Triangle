#ifndef __IMAGE_H_INCLUDED__
#define __IMAGE_H_INCLUDED__

#include <cstdint>
#include <string>
#include <vector>

void writePng(const std::string& path,
              const std::vector<std::uint8_t>& rgba,
              std::uint32_t width,
              std::uint32_t height);

double coverage(const std::vector<std::uint8_t>& rgba);

double diffFromReference(const std::string& name,
                         const std::vector<std::uint8_t>& rgba,
                         std::uint32_t width,
                         std::uint32_t height);

std::vector<std::uint8_t> pixelAt(const std::vector<std::uint8_t>& rgba,
                                  std::uint32_t width,
                                  std::uint32_t x,
                                  std::uint32_t y);

#endif
