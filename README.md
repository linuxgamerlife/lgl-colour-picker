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

The recommended Fedora install path is COPR:

```sh
sudo dnf copr enable linuxgamerlife/lgl-colour-picker
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
sudo dnf install cmake gcc-c++ ninja-build gtkmm4.0-devel libadwaita-devel \
  glib2-devel desktop-file-utils appstream
cmake -S . -B build-gtk -G Ninja -DBUILD_TESTING=ON
cmake --build build-gtk
ctest --test-dir build-gtk --output-on-failure
```

Run the development build with:

```sh
./build-gtk/lgl-colour-picker
```

The legacy Qt sources remain in the repository for reference during the
migration, but the root CMake build and package use only the GTK application in
`gtk/`.
