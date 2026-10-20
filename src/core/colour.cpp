#include "core/colour.h"

#include <cmath>
#include <cstdio>

namespace colourpicker {
std::string hexText(const Colour &colour) { char text[8]; std::snprintf(text, sizeof text, "#%02X%02X%02X", colour.red, colour.green, colour.blue); return text; }
std::string rgbText(const Colour &colour) { return "rgb(" + std::to_string(colour.red) + ", " + std::to_string(colour.green) + ", " + std::to_string(colour.blue) + ")"; }
std::string rgbaText(const Colour &colour) { return "rgba(" + std::to_string(colour.red) + ", " + std::to_string(colour.green) + ", " + std::to_string(colour.blue) + ", " + std::to_string(colour.alpha) + ")"; }
std::optional<Colour> colourFromPortal(const std::array<double, 3> &rgb) {
    for (const double channel : rgb) if (!std::isfinite(channel) || channel < 0.0 || channel > 1.0) return std::nullopt;
    return Colour{static_cast<unsigned char>(std::lround(rgb[0] * 255.0)), static_cast<unsigned char>(std::lround(rgb[1] * 255.0)), static_cast<unsigned char>(std::lround(rgb[2] * 255.0)), 255};
}
} // namespace colourpicker
