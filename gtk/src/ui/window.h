#pragma once
#include "core/colour.h"
#include "core/settings.h"
#include <gtkmm/applicationwindow.h>
#include <gtkmm/aboutdialog.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/entry.h>
#include <gtkmm/headerbar.h>
#include <gtkmm/label.h>
#include <memory>
namespace colourpicker {
class ThemeWatcher;
class PickerController;
class HelpWindow;
class Window final : public Gtk::ApplicationWindow {
public: explicit Window(const Glib::RefPtr<Gtk::Application> &application);
    ~Window() override;
private:
    void buildHeader(); void buildBody(); void buildActions(const Glib::RefPtr<Gtk::Application> &application); void applyTitleBar(bool system); void showHelp(); void showAbout(); void setColour(const Colour &colour); void copy(const Gtk::Entry &entry);
    Settings m_settings; Gtk::HeaderBar m_header; Gtk::Box m_content{Gtk::Orientation::VERTICAL}; Gtk::DrawingArea m_preview; Gtk::Label m_status; Gtk::Entry m_hex; Gtk::Entry m_rgb; Gtk::Entry m_rgba; Gtk::Button *m_pickButton{nullptr}; Colour m_colour{88, 166, 255, 255}; std::unique_ptr<ThemeWatcher> m_theme; std::unique_ptr<PickerController> m_picker; std::unique_ptr<HelpWindow> m_help; std::unique_ptr<Gtk::AboutDialog> m_about;
};
} // namespace colourpicker
