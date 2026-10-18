#include "core/theme.h"
#include <algorithm>
#include <cmath>
namespace colourpicker::theme {
bool isDark(const double red, const double green, const double blue) {
    const auto channel = [](const double value) { const double v = std::clamp(value, 0.0, 1.0); return v <= 0.04045 ? v / 12.92 : std::pow((v + 0.055) / 1.055, 2.4); };
    return 0.2126 * channel(red) + 0.7152 * channel(green) + 0.0722 * channel(blue) < 0.5;
}
} // namespace colourpicker::theme
