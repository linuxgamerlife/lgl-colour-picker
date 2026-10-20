#include "core/settings.h"
#include <glib/gstdio.h>
#include <glibmm/miscutils.h>
#include <utility>
namespace colourpicker {
Settings::Settings() : Settings(Glib::build_filename(Glib::get_user_config_dir(), "lgl-colour-picker", "settings.ini")) {}
Settings::Settings(std::string path) : m_path(std::move(path)), m_file(Glib::KeyFile::create()) { try { m_file->load_from_file(m_path, Glib::KeyFile::Flags::KEEP_COMMENTS); } catch (const Glib::Error &) {} }
bool Settings::systemTitleBar() const { try { return m_file->get_boolean("window", "systemTitleBar"); } catch (const Glib::Error &) { return false; } }
void Settings::setSystemTitleBar(const bool enabled) { m_file->set_boolean("window", "systemTitleBar", enabled); save(); }
void Settings::save() { const auto directory = Glib::path_get_dirname(m_path); g_mkdir_with_parents(directory.c_str(), 0700); try { m_file->save_to_file(m_path); } catch (const Glib::Error &) {} }
} // namespace colourpicker
