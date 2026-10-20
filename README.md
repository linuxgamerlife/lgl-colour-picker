# LGL Simple Colour Picker v1.0.2

LGL Simple Colour Picker is a GTK 4/C++ desktop utility for sampling a screen
colour and copying it in ready-to-paste formats.

Version 1.0.2 is the GTK migration release. It keeps the package and executable
name `lgl-colour-picker`, follows the active GTK desktop theme, and uses the
desktop's standard portal for compositor-safe screen picking.

## Features

- GTK 4 (`gtkmm`) and libadwaita styling support, written in C++20.
- Pick a screen colour through the XDG Desktop Portal.
- Current-colour preview plus ready-to-copy HEX, RGB, and RGBA values.
- GTK clipboard integration for one-click copy actions.
- Standard LGL header bar, main menu, Help, About dialog, and system title-bar
  mode (`Ctrl+Shift+M`).
- Live desktop theme support is being completed as part of the GTK migration.

## Runtime Requirements

Wayland colour picking requires `xdg-desktop-portal` and a desktop portal
backend that implements `org.freedesktop.portal.Screenshot.PickColor`. The
backend owns the screen-selection UI because Wayland does not allow arbitrary
applications to read screen pixels directly.

The app does not depend on Qt, Niri, Noctalia, `grim`, `slurp`, or
`wl-clipboard`. GTK and libadwaita library requirements are resolved by Fedora
from the installed RPM.

## Install

The recommended Fedora install path is the
[LGL Toolkit COPR](https://copr.fedorainfracloud.org/coprs/linuxgamerlife/lgl-toolkit/):

```sh
sudo dnf copr enable linuxgamerlife/lgl-toolkit
sudo dnf install lgl-colour-picker
```

Launch it from the application menu or with:

```sh
lgl-colour-picker
```

## Build from Source

Build in the target Fedora development container:

```sh
distrobox enter fedora-dev1
sudo dnf install cmake gcc-c++ gtkmm4.0-devel libadwaita-devel glib2-devel \
  json-glib-devel desktop-file-utils appstream
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build -j$(nproc)
ctest --test-dir build --output-on-failure
```

Run the development build with:

```sh
./build/lgl-colour-picker
```

The maintained GTK application lives in `src/`, its resources in `data/`, and
its tests in `tests/`. The root `CMakeLists.txt` builds and packages that
application directly. `legacy/qt/` is a read-only historical reference; it is
not built, tested, packaged, or a runtime dependency.
