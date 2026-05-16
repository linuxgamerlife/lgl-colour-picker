# Changelog

## 1.0.1 - 2026-05-16

- Cleaned up active portal requests, helper processes, and overlays when the app
  closes.
- Added bounded timeouts for interactive helper processes.
- Addressed Qt static-analysis feedback by reusing the compositor-output
  parsing regular expression.

## 1.0.0 - 2026-05-16

Initial v1.0.0 release of LGL Simple Colour Picker.

- Added the initial Qt 6/C++ MVP.
- Added screen colour picking with portal-first Wayland support.
- Added colour preview.
- Added HEX, RGB, and RGBA output fields.
- Added one-click clipboard copy buttons.
- Kept the picker window visible while selecting on Wayland.
- Added transparent app icons for packaged desktop integration.
- Documented runtime dependencies for fresh Fedora installs.
- Added COPR install instructions.
