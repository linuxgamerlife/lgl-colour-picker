# LGL Simple Colour Picker v1.0.1

LGL Simple Colour Picker is a small Qt 6 desktop utility for sampling a colour
from the screen and copying it in ready-to-paste formats.

Version 1.0.1 is the current release. It focuses on fast screen picking,
format display, and clipboard copy while keeping the executable and package
name as `lgl-colour-picker`. It also includes lifecycle cleanup fixes for
portal requests and helper processes.

## Features

- Native Qt 6/C++ application.
- Click `Pick Colour`, then click a screen pixel to sample its colour.
- Keeps the picker window visible while selecting on Wayland.
- Shows the selected colour preview.
- Displays ready-to-copy HEX, RGB, and RGBA values.
- Copy buttons place colour values on the clipboard.
- Wayland picking uses the XDG Desktop Portal color picker first, with
  compositor/helper fallbacks where available.
- Clipboard support uses Qt clipboard data first, with Wayland/X11 command-line
  fallbacks where available.
- The UI follows the active Qt platform theme and palette.

## Not In This Release

The original project notes include future ideas such as smart paste detection,
expanded format conversion, enlarged hover preview, original/current colour
comparison, and contrast checking. Those are not part of v1.0.x.

## Runtime Dependencies

When installed from an RPM package, Fedora automatically pulls in the linked Qt
runtime libraries. The package also requires:

- `qt6-qtwayland` for native Qt Wayland windows.
- `xdg-desktop-portal` for portal-first Wayland colour picking.

A desktop portal backend is also needed. Fedora Workstation and Fedora KDE
normally provide one through their desktop environment. Minimal installs or
custom compositor setups need a configured backend that implements the
Screenshot portal's color picker. The app also has helper fallbacks:

- `niri` is used as a fallback when running under Niri.
- `grim` and `slurp` are used as fallbacks on compositors that support them.
- `wl-clipboard` is optional and can improve clipboard behavior on Wayland.

On a fresh Fedora Workstation or KDE install, the packaged app should work after
installing it through DNF. On a fresh minimal Fedora install, install and
configure the Wayland portal/backend stack first.

## Install

The recommended install path on Fedora is COPR:

```sh
sudo dnf copr enable linuxgamerlife/lgl-colour-picker
sudo dnf install lgl-colour-picker
```

Then run it from your app launcher or with:

```sh
lgl-colour-picker
```

## Build From Source

On Fedora, install build dependencies:

```sh
sudo dnf install cmake gcc-c++ qt6-qtbase-devel desktop-file-utils appstream
```

For running a source build on Wayland, also install the runtime pieces listed
above, for example:

```sh
sudo dnf install qt6-qtwayland xdg-desktop-portal wl-clipboard
```

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Run from the build directory:

```sh
./build/lgl-colour-picker
```

Install from source:

```sh
sudo cmake --install build
```

The installed desktop file runs `lgl-colour-picker`, so it can be bound to a
keyboard shortcut such as `Super+Alt+C`.
