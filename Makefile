APP := vsc-mdview
SRC := main.c
SPEC_FILE := vsc-mdview.spec
VERSION := $(shell awk '/^Version:/{print $$2; exit}' $(SPEC_FILE))
TARBALL_DIR := $(APP)-$(VERSION)
TARBALL := $(TARBALL_DIR).tar.gz
RPMBUILD_TOPDIR := $(CURDIR)/.rpmbuild
RPM_SOURCES := Makefile LICENSE README.md main.c vsc-mdview.desktop vsc-mdview.png vsc-mdview.spec vsc-mdview.1
PREFIX ?= /usr/local
DESTDIR ?=
ICON_SRC := vsc-mdview.png
ICON_THEME_DIR := icons/hicolor
ICON_SIZES := 16 24 32 48 64 96 128 256 512
DESKTOP_FILE := vsc-mdview.desktop
MAN_PAGE := vsc-mdview.1

# Prefer GTK4/WebKitGTK6, then fall back to GTK3/WebKit2GTK variants.
WEBKIT_PKG := $(shell \
	if pkg-config --exists webkitgtk-6.0; then echo webkitgtk-6.0; \
	elif pkg-config --exists webkit2gtk-4.1; then echo webkit2gtk-4.1; \
	elif pkg-config --exists webkit2gtk-4.0; then echo webkit2gtk-4.0; \
	fi)

ifeq ($(WEBKIT_PKG),webkitgtk-6.0)
GTK_PKG := gtk4
else
GTK_PKG := gtk+-3.0
endif
CMARK_PKG := $(shell \
	if pkg-config --exists cmark; then echo cmark; \
	elif pkg-config --exists libcmark; then echo libcmark; \
	elif pkg-config --exists libcmark-gfm; then echo libcmark-gfm; \
	fi)

ifeq ($(strip $(WEBKIT_PKG)),)
$(error No WebKitGTK pkg-config package found: tried webkitgtk-6.0, webkit2gtk-4.1, webkit2gtk-4.0)
endif

ifeq ($(strip $(CMARK_PKG)),)
$(error No cmark pkg-config package found: tried cmark, libcmark, libcmark-gfm)
endif

PKGS := $(GTK_PKG) $(WEBKIT_PKG) $(CMARK_PKG)

CFLAGS += -O2 -Wall -Wextra -Wpedantic $(shell pkg-config --cflags $(PKGS))
LDLIBS += $(shell pkg-config --libs $(PKGS))

.PHONY: all clean icons install uninstall rpm

all: $(APP)

$(APP): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

icons: $(ICON_SRC)
	@for sz in $(ICON_SIZES); do \
		dir="$(ICON_THEME_DIR)/$${sz}x$${sz}/apps"; \
		mkdir -p "$$dir"; \
		magick "$(ICON_SRC)" -resize "$${sz}x$${sz}" "$$dir/$(APP).png"; \
	done

install: $(APP) icons $(DESKTOP_FILE) $(MAN_PAGE)
	install -Dm755 "$(APP)" "$(DESTDIR)$(PREFIX)/bin/$(APP)"
	install -Dm644 "$(DESKTOP_FILE)" "$(DESTDIR)$(PREFIX)/share/applications/$(DESKTOP_FILE)"
	install -Dm644 "$(MAN_PAGE)" "$(DESTDIR)$(PREFIX)/share/man/man1/$(MAN_PAGE)"
	@for sz in $(ICON_SIZES); do \
		install -Dm644 "$(ICON_THEME_DIR)/$${sz}x$${sz}/apps/$(APP).png" \
			"$(DESTDIR)$(PREFIX)/share/icons/hicolor/$${sz}x$${sz}/apps/$(APP).png"; \
	done

uninstall:
	rm -f "$(DESTDIR)$(PREFIX)/bin/$(APP)"
	rm -f "$(DESTDIR)$(PREFIX)/share/applications/$(DESKTOP_FILE)"
	rm -f "$(DESTDIR)$(PREFIX)/share/man/man1/$(MAN_PAGE)"
	@for sz in $(ICON_SIZES); do \
		rm -f "$(DESTDIR)$(PREFIX)/share/icons/hicolor/$${sz}x$${sz}/apps/$(APP).png"; \
	done

rpm:
	tar czf "$(TARBALL)" --transform 's,^,$(TARBALL_DIR)/,' $(RPM_SOURCES)
	rpmbuild -ba "$(SPEC_FILE)" \
		--define "_topdir $(RPMBUILD_TOPDIR)" \
		--define "_sourcedir $(CURDIR)" \
		--define "_srcrpmdir $(CURDIR)" \
		--define "_rpmdir $(CURDIR)" \
		--define '_build_name_fmt %%{NAME}-%%{VERSION}-%%{RELEASE}.%%{ARCH}.rpm'


clean:
	rm -f $(APP)
