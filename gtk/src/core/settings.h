#pragma once
#include <glibmm/keyfile.h>
#include <string>
namespace colourpicker {
class Settings final {
public: Settings(); explicit Settings(std::string path); bool systemTitleBar() const; void setSystemTitleBar(bool enabled);
private: void save(); std::string m_path; Glib::RefPtr<Glib::KeyFile> m_file;
};
} // namespace colourpicker
