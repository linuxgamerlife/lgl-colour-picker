#include "ui/helpwindow.h"

#include <giomm/resource.h>
#include <gtkmm/headerbar.h>
#include <gtkmm/label.h>
#include <gtkmm/scrolledwindow.h>

namespace colourpicker {

HelpWindow::HelpWindow(Gtk::Window &parent)
{
    set_title("LGL Simple Colour Picker Help");
    set_transient_for(parent);
    set_default_size(640, 480);
    set_hide_on_close(true);

    auto *header = Gtk::make_managed<Gtk::HeaderBar>();
    auto *title = Gtk::make_managed<Gtk::Label>("HELP");
    title->add_css_class("wordmark");
    header->set_title_widget(*title);
    set_titlebar(*header);

    auto buffer = Gtk::TextBuffer::create();
    try {
        const auto bytes = Gio::Resource::lookup_data_global("/com/linuxgamerlife/colourpicker/HELP.md");
        gsize size = 0;
        const auto *data = static_cast<const char *>(bytes->get_data(size));
        buffer->set_text(std::string(data, size));
    } catch (const Glib::Error &) {
        buffer->set_text("The help file could not be opened.");
    }
    m_view.set_buffer(buffer);
    m_view.set_editable(false);
    m_view.set_cursor_visible(false);
    m_view.set_wrap_mode(Gtk::WrapMode::WORD_CHAR);
    m_view.set_left_margin(24);
    m_view.set_right_margin(24);
    m_view.set_top_margin(18);
    m_view.set_bottom_margin(18);
    m_view.add_css_class("help-view");

    auto *scroller = Gtk::make_managed<Gtk::ScrolledWindow>();
    scroller->set_child(m_view);
    set_child(*scroller);
}

} // namespace colourpicker
