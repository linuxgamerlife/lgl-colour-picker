#include "ui/window.h"
#include "ui/themewatcher.h"
#include "ui/pickercontroller.h"
#include "ui/helpwindow.h"
#include <gdkmm/texture.h>
#include <giomm/simpleaction.h>
#include <giomm/menu.h>
#include <gtkmm/application.h>
#include <gtkmm/button.h>
#include <gtkmm/image.h>
#include <gtkmm/menubutton.h>
namespace colourpicker {
namespace {
Gtk::Label *label(const char *text, const char *cssClass) { auto *result = Gtk::make_managed<Gtk::Label>(text); result->add_css_class(cssClass); result->set_halign(Gtk::Align::START); return result; }
Glib::RefPtr<Gdk::Texture> appIcon() { static const auto icon = Gdk::Texture::create_from_resource("/com/linuxgamerlife/colourpicker/lgl-colour-picker.png"); return icon; }
}
Window::Window(const Glib::RefPtr<Gtk::Application> &application) {
    set_application(application); set_title("LGL Simple Colour Picker"); set_icon_name("lgl-colour-picker"); set_default_size(560, 380);
    m_theme = std::make_unique<ThemeWatcher>(*this); buildHeader(); buildBody(); buildActions(application); applyTitleBar(m_settings.systemTitleBar()); setColour(m_colour);
    m_picker = std::make_unique<PickerController>([this](const std::optional<Colour> &colour, const std::string &message) { if (colour) setColour(*colour); m_status.set_text(message); });
}
Window::~Window() = default;
void Window::buildHeader() {
    auto *brand = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 10); brand->set_margin_end(12);
    auto *icon = Gtk::make_managed<Gtk::Image>(appIcon()); icon->set_pixel_size(28); icon->add_css_class("brand-icon"); brand->append(*icon); brand->append(*label("LGL SIMPLE COLOUR PICKER", "wordmark")); m_header.pack_start(*brand); m_header.set_title_widget(*Gtk::make_managed<Gtk::Box>());
    auto menu = Gio::Menu::create(); auto actions = Gio::Menu::create(); actions->append("Pick Colour", "win.pick-colour"); menu->append_section(actions); auto view = Gio::Menu::create(); view->append("Use System Title Bar", "win.system-title-bar"); menu->append_section(view); auto help = Gio::Menu::create(); help->append("Help", "win.help"); help->append("About LGL Simple Colour Picker", "win.about"); menu->append_section(help); auto quit = Gio::Menu::create(); quit->append("Quit", "app.quit"); menu->append_section(quit);
    auto *menuButton = Gtk::make_managed<Gtk::MenuButton>(); menuButton->set_icon_name("open-menu-symbolic"); menuButton->set_menu_model(menu); menuButton->set_tooltip_text("Main menu"); menuButton->set_primary(true); m_header.pack_end(*menuButton);
    m_pickButton = Gtk::make_managed<Gtk::Button>("Pick Colour"); m_pickButton->set_action_name("win.pick-colour"); m_pickButton->set_tooltip_text("Pick a colour from the screen (Ctrl+P)"); m_pickButton->add_css_class("suggested-action"); m_pickButton->add_css_class("cp-primary"); m_header.pack_end(*m_pickButton);
}
void Window::buildBody() {
    m_content.add_css_class("body"); m_content.set_spacing(12); set_child(m_content);
    auto *top = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 12); top->add_css_class("cp-card"); m_preview.add_css_class("colour-preview");
    m_preview.set_draw_func([this](const Cairo::RefPtr<Cairo::Context> &context, const int width, const int height) {
        context->set_source_rgb(static_cast<double>(m_colour.red) / 255.0,
                                static_cast<double>(m_colour.green) / 255.0,
                                static_cast<double>(m_colour.blue) / 255.0);
        context->rectangle(1.0, 1.0, static_cast<double>(width - 2), static_cast<double>(height - 2));
        context->fill();
    });
    top->append(m_preview);
    auto *description = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 6); description->append(*label("CURRENT COLOUR", "section-label")); m_status.set_text("Ready."); m_status.set_wrap(true); m_status.add_css_class("muted"); m_status.set_halign(Gtk::Align::START); description->append(m_status); top->append(*description); m_content.append(*top);
    auto *formats = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 10); formats->add_css_class("cp-card"); formats->append(*label("COPY FORMAT", "section-label"));
    for (const auto &[name, entry] : {std::pair{"HEX", &m_hex}, {"RGB", &m_rgb}, {"RGBA", &m_rgba}}) { auto *row = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 8); auto *field = label(name, "mono"); field->set_size_request(54, -1); row->append(*field); entry->set_editable(false); entry->set_hexpand(true); entry->add_css_class("mono"); row->append(*entry); auto *button = Gtk::make_managed<Gtk::Button>("Copy"); button->signal_clicked().connect([this, entry] { copy(*entry); }); row->append(*button); formats->append(*row); }
    m_content.append(*formats);
}

void Window::buildActions(const Glib::RefPtr<Gtk::Application> &application)
{
    add_action("pick-colour", [this] {
        m_status.set_text("Picking colour…");
        m_picker->start();
    });
    add_action("help", sigc::mem_fun(*this, &Window::showHelp));
    add_action("about", sigc::mem_fun(*this, &Window::showAbout));
    auto titleBar = add_action_bool("system-title-bar", false);
    titleBar->signal_activate().connect([titleBar](const Glib::VariantBase &) {
        bool system = false;
        titleBar->get_state(system);
        titleBar->change_state(!system);
    });
    titleBar->signal_change_state().connect([this, titleBar](const Glib::VariantBase &value) {
        const bool system = Glib::VariantBase::cast_dynamic<Glib::Variant<bool>>(value).get();
        titleBar->set_state(value);
        m_settings.setSystemTitleBar(system);
        applyTitleBar(system);
    });
    titleBar->set_state(Glib::Variant<bool>::create(m_settings.systemTitleBar()));
    application->set_accel_for_action("win.pick-colour", "<Control>p");
    application->set_accel_for_action("win.help", "F1");
    application->set_accel_for_action("win.system-title-bar", "<Control><Shift>m");
}

void Window::applyTitleBar(const bool system)
{
    const bool realized = get_realized();
    const auto application = get_application();
    if (realized) {
        application->hold();
        set_visible(false);
        Gtk::Widget::unrealize();
    }
    if (system) {
        unset_titlebar();
        m_header.set_show_title_buttons(false);
        m_content.prepend(m_header);
    } else {
        if (m_header.get_parent())
            m_content.remove(m_header);
        m_header.set_show_title_buttons(true);
        set_titlebar(m_header);
    }
    if (realized) {
        set_application(application);
        present();
        m_pickButton->grab_focus();
        application->release();
    }
}

void Window::showHelp()
{
    if (!m_help)
        m_help = std::make_unique<HelpWindow>(*this);
    m_help->present();
}

void Window::showAbout()
{
    if (!m_about) {
        m_about = std::make_unique<Gtk::AboutDialog>();
        m_about->set_transient_for(*this);
        m_about->set_modal(true);
        m_about->set_hide_on_close(true);
        m_about->set_program_name("LGL Simple Colour Picker");
        m_about->set_version(LGL_COLOUR_PICKER_VERSION);
        m_about->set_comments("Pick colours from your screen and copy ready-to-paste values.\n\nBuilt with GTK 4.");
        m_about->set_website("https://github.com/linuxgamerlife/lgl-colour-picker");
        m_about->set_website_label("Project page and issue tracker");
        m_about->set_authors({"LinuxGamerLife https://www.youtube.com/@linuxgamerlife"});
        m_about->add_credit_section("Support", {"Support development on Ko-fi https://ko-fi.com/G2G3V70LW"});
        m_about->set_license_type(Gtk::License::MIT_X11);
        m_about->set_logo(appIcon());
    }
    m_about->present();
}

void Window::setColour(const Colour &colour) { m_colour = colour; m_hex.set_text(hexText(colour)); m_rgb.set_text(rgbText(colour)); m_rgba.set_text(rgbaText(colour)); m_preview.set_tooltip_text(hexText(colour)); m_preview.queue_draw(); }
void Window::copy(const Gtk::Entry &entry) { get_clipboard()->set_text(entry.get_text()); m_status.set_text("Copied " + entry.get_text()); }
} // namespace colourpicker
