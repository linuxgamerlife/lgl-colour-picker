Name:           lgl-colour-picker
Version:        1.0.1
Release:        1%{?dist}
Summary:        Simple Qt colour picker with clipboard-ready formats

License:        MIT
URL:            https://github.com/linuxgamerlife/lgl-colour-picker
Source0:        %{url}/archive/refs/tags/v%{version}/%{name}-%{version}.tar.gz

BuildRequires:  cmake
BuildRequires:  gcc-c++
BuildRequires:  qt6-qtbase-devel
BuildRequires:  desktop-file-utils
BuildRequires:  appstream

Requires:       qt6-qtwayland
Requires:       xdg-desktop-portal

Recommends:     grim
Recommends:     slurp
Recommends:     wl-clipboard

%description
LGL Simple Colour Picker is a small Qt 6 utility for sampling a screen pixel
and copying ready-to-paste HEX, RGB, and RGBA colour values. Wayland picking
uses the XDG Desktop Portal color picker first, with compositor/helper
fallbacks where available.

%prep
%autosetup -n %{name}-%{version}

%build
%cmake
%cmake_build

%install
%cmake_install

%check
desktop-file-validate %{buildroot}%{_datadir}/applications/%{name}.desktop
appstreamcli validate --no-net %{buildroot}%{_datadir}/metainfo/%{name}.metainfo.xml

%files
%license LICENSE
%doc README.md CHANGELOG.md
%{_bindir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/metainfo/%{name}.metainfo.xml
%{_datadir}/icons/hicolor/48x48/apps/%{name}.png
%{_datadir}/icons/hicolor/64x64/apps/%{name}.png
%{_datadir}/icons/hicolor/128x128/apps/%{name}.png
%{_datadir}/icons/hicolor/256x256/apps/%{name}.png

%changelog
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
