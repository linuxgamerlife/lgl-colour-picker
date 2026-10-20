#pragma once

#include <gtkmm/textview.h>
#include <gtkmm/window.h>

namespace colourpicker {

class HelpWindow final : public Gtk::Window {
public:
    explicit HelpWindow(Gtk::Window &parent);

private:
    Gtk::TextView m_view;
};

} // namespace colourpicker
