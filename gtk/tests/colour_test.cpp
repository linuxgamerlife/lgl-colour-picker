#include "core/colour.h"
#include <glib.h>
void formats() { const colourpicker::Colour colour{18, 52, 86, 255}; const auto hex = colourpicker::hexText(colour); const auto rgb = colourpicker::rgbText(colour); const auto rgba = colourpicker::rgbaText(colour); g_assert_cmpstr(hex.c_str(), ==, "#123456"); g_assert_cmpstr(rgb.c_str(), ==, "rgb(18, 52, 86)"); g_assert_cmpstr(rgba.c_str(), ==, "rgba(18, 52, 86, 255)"); }
void portalColour() { const auto colour = colourpicker::colourFromPortal({1.0, 0.5, 0.0}); g_assert_true(colour.has_value()); g_assert_cmpint(colour->green, ==, 128); g_assert_false(colourpicker::colourFromPortal({1.1, 0.0, 0.0}).has_value()); }
int main(int argc, char **argv) { g_test_init(&argc, &argv, nullptr); g_test_add_func("/colour/formats", formats); g_test_add_func("/colour/portal", portalColour); return g_test_run(); }
