#include "ui/themewatcher.h"
#include <adwaita.h>
#include <gdkmm/display.h>
#include <gtkmm/cssprovider.h>
#include <gtk/gtk.h>
namespace colourpicker {
void installApplicationStylesheet() { adw_init(); auto provider = Gtk::CssProvider::create(); provider->load_from_resource("/com/linuxgamerlife/colourpicker/style.css"); gtk_style_context_add_provider_for_display(Gdk::Display::get_default()->gobj(), GTK_STYLE_PROVIDER(provider->gobj()), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION); }
ThemeWatcher::ThemeWatcher(Gtk::Window &window) : m_window(window) {}
} // namespace colourpicker
