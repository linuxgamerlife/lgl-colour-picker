#include "core/theme.h"
#include <glib.h>
int main(int argc, char **argv) { g_test_init(&argc, &argv, nullptr); g_test_add_func("/theme/luminance", [] { g_assert_true(colourpicker::theme::isDark(0.0, 0.0, 0.0)); g_assert_false(colourpicker::theme::isDark(1.0, 1.0, 1.0)); }); return g_test_run(); }
