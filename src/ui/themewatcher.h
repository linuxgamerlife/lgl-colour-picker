#pragma once

#include "core/theme.h"

#include <giomm/filemonitor.h>
#include <giomm/settings.h>
#include <gtkmm/cssprovider.h>
#include <gtkmm/window.h>

#include <map>
#include <string>
#include <vector>

namespace colourpicker {
class ThemeWatcher final : public sigc::trackable {
public:
    explicit ThemeWatcher(Gtk::Window &window);
    ~ThemeWatcher();
    ThemeWatcher(const ThemeWatcher &) = delete;
    ThemeWatcher &operator=(const ThemeWatcher &) = delete;
    void queueChange() { queue(); }
private:
    void watch(); void watchFiles(); void queue(); bool changed(); void followDesktopWithoutPortal(); void apply(bool reloadUserCss); theme::Theme read() const; std::string interfaceString(const char *key) const;
    Gtk::Window &m_window; theme::Theme m_theme; Glib::RefPtr<Gtk::CssProvider> m_userProvider; Glib::RefPtr<Gtk::CssProvider> m_fontProvider; Glib::RefPtr<Gio::Settings> m_interface; std::map<std::string, Glib::RefPtr<Gio::FileMonitor>> m_monitors; sigc::connection m_timeout; std::vector<gulong> m_styleHandlers;
};
void installApplicationStylesheet();
} // namespace colourpicker
