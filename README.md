# vsc-mdview

Simple GNOME/Linux Markdown viewer in C:
- takes a filename as the first CLI argument
- reads the file
- converts Markdown to HTML in memory
- renders it in an embedded WebKit browser widget
- follows GNOME interface settings (dark/light preference and UI fonts)

## Build dependencies

Install development packages for:
- GTK (`gtk4` or `gtk+-3.0`)
- WebKitGTK (`webkitgtk-6.0` or `webkit2gtk-4.1` or `webkit2gtk-4.0`)
- cmark (`cmark` or `libcmark` or `libcmark-gfm`)
- `pkg-config`, `make`, and a C compiler

Example package names:
- Fedora: `gtk4-devel webkitgtk6.0-devel cmark-devel`
- Debian/Ubuntu (GTK3 path): `libgtk-3-dev libwebkit2gtk-4.1-dev libcmark-dev`

## Build and run

```bash
make
./vsc-mdview /path/to/file.md
```

## Generate GNOME icons from `vsc-mdview.png`

```bash
make icons
```

This creates resized icons under `icons/hicolor/*/apps/vsc-mdview.png`.

## Install app + launcher + icons

```bash
make install
```

If installing to a custom prefix:

```bash
make install PREFIX="$HOME/.local"
```

## Build RPM

```bash
make rpm
```

This creates:
- source tarball: `vsc-mdview-<version>.tar.gz`
- SRPM: `vsc-mdview-<version>-*.src.rpm`
- binary RPM: `x86_64/vsc-mdview-<version>-*.x86_64.rpm`

## Man page

```bash
man ./vsc-mdview.1
```
