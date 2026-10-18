#include "ui/themewatcher.h"
#include "ui/window.h"
#include <glibmm/miscutils.h>
#include <gtkmm/application.h>
#include <memory>
int main(int argc, char *argv[]) {
    auto application = Gtk::Application::create("com.linuxgamerlife.lgl-colour-picker"); std::unique_ptr<colourpicker::Window> window;
    application->signal_startup().connect([&] { Glib::set_application_name("LGL Simple Colour Picker"); Gtk::Window::set_default_icon_name("lgl-colour-picker"); colourpicker::installApplicationStylesheet(); application->add_action("quit", [&] { if (window) window->set_visible(false); application->quit(); }); application->set_accel_for_action("app.quit", "<Control>q"); });
    application->signal_activate().connect([&] { if (!window) window = std::make_unique<colourpicker::Window>(application); window->present(); });
    const int status = application->run(argc, argv); window.reset(); return status;
}
