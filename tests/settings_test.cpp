#include "core/settings.h"
#include <glib.h>
#include <glib/gstdio.h>
void persistsSystemTitleBar() { GError *error = nullptr; char *folder = g_dir_make_tmp("lgl-colour-picker-test-XXXXXX", &error); g_assert_no_error(error); const std::string path = std::string(folder) + "/settings.ini"; colourpicker::Settings settings(path); g_assert_false(settings.systemTitleBar()); settings.setSystemTitleBar(true); g_assert_true(colourpicker::Settings(path).systemTitleBar()); g_remove(path.c_str()); g_rmdir(folder); g_free(folder); }
int main(int argc, char **argv) { g_test_init(&argc, &argv, nullptr); g_test_add_func("/settings/system-title-bar", persistsSystemTitleBar); return g_test_run(); }
