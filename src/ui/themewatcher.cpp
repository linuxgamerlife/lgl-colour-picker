#include "ui/themewatcher.h"

#include <adwaita.h>
#include <giomm/file.h>
#include <giomm/settingsschemasource.h>
#include <glibmm/fileutils.h>
#include <glibmm/main.h>
#include <glibmm/miscutils.h>
#include <gtkmm/settings.h>
#include <gtkmm/styleprovider.h>

#include <algorithm>
#include <cctype>

namespace colourpicker {
namespace {
constexpr unsigned int debounceMs = 200;
Glib::RefPtr<Gtk::CssProvider> createProvider(const unsigned int priority) { auto provider = Gtk::CssProvider::create(); provider->signal_parsing_error().connect([](const Glib::RefPtr<const Gtk::CssSection> &, const Glib::Error &) {}); Gtk::StyleProvider::add_provider_for_display(Gdk::Display::get_default(), provider, priority); return provider; }
bool endsWith(const std::string &text, const std::string &suffix) { return text.size() >= suffix.size() && text.compare(text.size() - suffix.size(), suffix.size(), suffix) == 0; }
void styleManagerChanged(GObject *, GParamSpec *, gpointer watcher) { static_cast<ThemeWatcher *>(watcher)->queueChange(); }
}
void installApplicationStylesheet() { adw_init(); auto provider = Gtk::CssProvider::create(); provider->signal_parsing_error().connect([](const Glib::RefPtr<const Gtk::CssSection> &section, const Glib::Error &error) { g_warning("style.css:%u: %s", section ? static_cast<unsigned>(section->get_start_location().get_lines() + 1) : 0u, error.what()); }); provider->load_from_resource("/com/linuxgamerlife/colourpicker/style.css"); Gtk::StyleProvider::add_provider_for_display(Gdk::Display::get_default(), provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION); }
ThemeWatcher::ThemeWatcher(Gtk::Window &window) : m_window(window), m_userProvider(createProvider(GTK_STYLE_PROVIDER_PRIORITY_USER)), m_fontProvider(createProvider(GTK_STYLE_PROVIDER_PRIORITY_APPLICATION + 1)) { watch(); apply(false); }
ThemeWatcher::~ThemeWatcher() { auto *manager = adw_style_manager_get_default(); for (const auto handler : m_styleHandlers) g_signal_handler_disconnect(manager, handler); }
void ThemeWatcher::watch() { auto *manager = adw_style_manager_get_default(); for (const char *signal : {"notify::dark", "notify::accent-color"}) m_styleHandlers.push_back(g_signal_connect(manager, signal, G_CALLBACK(styleManagerChanged), this)); const auto source = Gio::SettingsSchemaSource::get_default(); if (source && source->lookup("org.gnome.desktop.interface", true)) { m_interface = Gio::Settings::create("org.gnome.desktop.interface"); m_interface->signal_changed().connect([this](const Glib::ustring &) { queue(); }); } watchFiles(); }
void ThemeWatcher::watchFiles() { const auto config = Glib::get_user_config_dir(); std::vector<std::string> folders{Glib::build_filename(config, "gtk-4.0")}; for (const auto &path : theme::gtkCssPaths(config)) folders.push_back(Glib::path_get_dirname(path)); for (const auto &folder : folders) { if (m_monitors.count(folder)) continue; try { auto monitor = Gio::File::create_for_path(folder)->monitor_directory(Gio::FileMonitorFlags::WATCH_MOVES); monitor->signal_changed().connect([this](const Glib::RefPtr<Gio::File> &file, const Glib::RefPtr<Gio::File> &other, Gio::FileMonitor::Event) { for (const auto &changed : {file, other}) if (changed && endsWith(changed->get_basename(), ".css")) { queue(); return; } }); m_monitors[folder] = monitor; } catch (const Glib::Error &) {} } }
void ThemeWatcher::queue() { m_timeout.disconnect(); m_timeout = Glib::signal_timeout().connect(sigc::mem_fun(*this, &ThemeWatcher::changed), debounceMs); }
bool ThemeWatcher::changed() { watchFiles(); apply(true); return false; }
std::string ThemeWatcher::interfaceString(const char *key) const { if (!m_interface) return {}; const auto schema = m_interface->property_settings_schema().get_value(); return schema && schema->has_key(key) ? m_interface->get_string(key).raw() : std::string{}; }
void ThemeWatcher::followDesktopWithoutPortal() { auto *manager = adw_style_manager_get_default(); if (adw_style_manager_get_system_supports_color_schemes(manager)) return; const auto settings = Gtk::Settings::get_default(); std::string themeName = settings->property_gtk_theme_name().get_value(); std::transform(themeName.begin(), themeName.end(), themeName.begin(), [](const unsigned char character) { return static_cast<char>(std::tolower(character)); }); const bool dark = interfaceString("color-scheme") == "prefer-dark" || settings->property_gtk_application_prefer_dark_theme().get_value() || endsWith(themeName, "-dark"); adw_style_manager_set_color_scheme(manager, dark ? ADW_COLOR_SCHEME_FORCE_DARK : ADW_COLOR_SCHEME_DEFAULT); }
theme::Theme ThemeWatcher::read() const { theme::Theme current; current.colors = theme::gtkColors(theme::gtkCssPaths(Glib::get_user_config_dir())); current.dark = theme::isDark(current.colors, adw_style_manager_get_dark(adw_style_manager_get_default())); current.monoFont = theme::fontFamily(interfaceString("monospace-font-name")); return current; }
void ThemeWatcher::apply(const bool reloadUserCss) { followDesktopWithoutPortal(); m_theme = read(); if (reloadUserCss) { const auto userCss = Glib::build_filename(Glib::get_user_config_dir(), "gtk-4.0", "gtk.css"); if (Glib::file_test(userCss, Glib::FileTest::IS_REGULAR)) m_userProvider->load_from_path(userCss); else m_userProvider->load_from_string(""); } m_fontProvider->load_from_string(theme::fontStylesheet(m_theme)); if (m_theme.dark) m_window.add_css_class("dark"); else m_window.remove_css_class("dark"); }
} // namespace colourpicker
