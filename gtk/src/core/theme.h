#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace colourpicker::theme {

using Colors = std::map<std::string, std::string>;

struct Theme final {
    Colors colors;
    bool dark{false};
    std::string monoFont;
};

std::vector<std::string> gtkCssPaths(const std::string &configDir);
Colors gtkColors(const std::vector<std::string> &paths);
std::string fontFamily(const std::string &description);
std::optional<double> luminance(const std::string &color);
bool isDark(const Colors &colors, bool preferDark);
std::string fontStylesheet(const Theme &theme);

} // namespace colourpicker::theme
