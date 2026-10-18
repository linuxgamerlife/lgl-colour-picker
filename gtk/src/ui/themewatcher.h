#pragma once
#include <gtkmm/window.h>
namespace colourpicker { void installApplicationStylesheet(); class ThemeWatcher final { public: explicit ThemeWatcher(Gtk::Window &window); private: Gtk::Window &m_window; }; }
