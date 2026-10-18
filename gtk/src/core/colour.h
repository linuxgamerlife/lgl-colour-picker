#pragma once

#include <array>
#include <optional>
#include <string>

namespace colourpicker {
struct Colour final { unsigned char red{0}; unsigned char green{0}; unsigned char blue{0}; unsigned char alpha{255}; };
std::string hexText(const Colour &colour);
std::string rgbText(const Colour &colour);
std::string rgbaText(const Colour &colour);
std::optional<Colour> colourFromPortal(const std::array<double, 3> &rgb);
} // namespace colourpicker
