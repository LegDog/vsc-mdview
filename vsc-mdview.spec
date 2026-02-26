Name:           vsc-mdview
Version:        0.1.0
Release:        1%{?dist}
Summary:        Very Simple C MarkDown viewer for Gnome

License:        MIT
URL:            https://github.com/LegDog/vsc-mdview
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  gcc
BuildRequires:  make
BuildRequires:  ImageMagick
BuildRequires:  pkgconfig(gtk+-3.0)
BuildRequires:  pkgconfig(webkit2gtk-4.1)
BuildRequires:  pkgconfig(libcmark)

%description
vsc-mdview is a small GNOME/Linux application written in C that reads a Markdown
file, converts it to HTML in memory, and displays it in an embedded WebKit view.

%prep
%autosetup -n %{name}-%{version}

%build
%make_build

%install
%make_install PREFIX=%{_prefix}

%files
%doc README.md
%{_bindir}/vsc-mdview
%{_datadir}/applications/vsc-mdview.desktop
%{_datadir}/man/man1/vsc-mdview.1*
%{_datadir}/icons/hicolor/16x16/apps/vsc-mdview.png
%{_datadir}/icons/hicolor/24x24/apps/vsc-mdview.png
%{_datadir}/icons/hicolor/32x32/apps/vsc-mdview.png
%{_datadir}/icons/hicolor/48x48/apps/vsc-mdview.png
%{_datadir}/icons/hicolor/64x64/apps/vsc-mdview.png
%{_datadir}/icons/hicolor/96x96/apps/vsc-mdview.png
%{_datadir}/icons/hicolor/128x128/apps/vsc-mdview.png
%{_datadir}/icons/hicolor/256x256/apps/vsc-mdview.png
%{_datadir}/icons/hicolor/512x512/apps/vsc-mdview.png

%changelog
* Thu Feb 26 2026 Luiz Gava <legdog@gmail.com> - 0.1.0-1
- Initial RPM packaging
