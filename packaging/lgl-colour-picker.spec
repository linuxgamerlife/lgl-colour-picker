Name:           lgl-colour-picker
Version:        1.0.2
Release:        1%{?dist}
Summary:        Simple GTK colour picker with clipboard-ready formats

License:        MIT
URL:            https://github.com/linuxgamerlife/lgl-colour-picker
Source0:        %{url}/archive/refs/tags/v%{version}/%{name}-%{version}.tar.gz

BuildRequires:  cmake >= 3.20
BuildRequires:  cmake-rpm-macros
BuildRequires:  gcc-c++
BuildRequires:  ninja-build
BuildRequires:  pkgconfig(gtkmm-4.0) >= 4.20
BuildRequires:  pkgconfig(libadwaita-1) >= 1.6
BuildRequires:  glib2-devel
BuildRequires:  json-glib-devel
BuildRequires:  desktop-file-utils
BuildRequires:  appstream

Requires:       xdg-desktop-portal

%description
LGL Simple Colour Picker is a GTK 4 utility for sampling a screen pixel and
copying ready-to-paste HEX, RGB, and RGBA colour values. Wayland picking uses
the XDG Desktop Portal, so the desktop provides compositor integration.

%prep
%autosetup -n %{name}-%{version}

%build
%cmake -DBUILD_TESTING=ON
%cmake_build

%install
%cmake_install

%check
%ctest
desktop-file-validate %{buildroot}%{_datadir}/applications/com.linuxgamerlife.%{name}.desktop
appstreamcli validate --no-net %{buildroot}%{_datadir}/metainfo/com.linuxgamerlife.%{name}.metainfo.xml

%files
%license LICENSE
%doc README.md CHANGELOG.md HELP.md
%{_bindir}/%{name}
%{_datadir}/applications/com.linuxgamerlife.%{name}.desktop
%{_datadir}/metainfo/com.linuxgamerlife.%{name}.metainfo.xml
%{_datadir}/icons/hicolor/48x48/apps/%{name}.png
%{_datadir}/icons/hicolor/64x64/apps/%{name}.png
%{_datadir}/icons/hicolor/128x128/apps/%{name}.png
%{_datadir}/icons/hicolor/256x256/apps/%{name}.png

%changelog
* Sun Oct 18 2026 LinuxGamerLife - 1.0.2-1
- Migrate the packaged application from Qt 6 to GTK 4
- Use the desktop portal for environment-agnostic screen colour picking
- Add Help, About, and a persisted system title-bar mode
- Remove Qt Wayland and compositor-helper dependencies

* Sat May 16 2026 linuxgamerlife <linuxgamerlife@users.noreply.github.com> - 1.0.1-1
- Clean up active portal requests, helper processes, and overlays on shutdown
- Add bounded timeouts for interactive helper processes
- Address Qt static-analysis feedback for compositor-output parsing

* Sat May 16 2026 linuxgamerlife <linuxgamerlife@users.noreply.github.com> - 1.0.0-1
- Initial Qt 6/C++ MVP with screen picking, preview, and copyable colour formats
- Use XDG Desktop Portal color picking first on Wayland
- Keep the picker window visible while selecting on Wayland
- Keep compositor/helper fallbacks for Wayland environments without portal support
- Add transparent app icons for packaged desktop integration
