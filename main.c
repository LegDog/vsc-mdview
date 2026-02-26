#include <glib.h>
#include <gio/gio.h>
#include <gtk/gtk.h>
#include <pango/pango.h>

#if __has_include(<webkit/webkit.h>)
#include <webkit/webkit.h>
#define VSC_MDVIEW_WEBKITGTK6 1
#elif __has_include(<webkit2/webkit2.h>)
#include <webkit2/webkit2.h>
#define VSC_MDVIEW_WEBKIT2GTK 1
#else
#error "WebKitGTK headers not found. Install webkitgtk/webkit2gtk development package."
#endif

#if __has_include(<cmark.h>)
#include <cmark.h>
#elif __has_include(<cmark-gfm.h>)
#include <cmark-gfm.h>
#else
#error "cmark headers not found. Install cmark development package."
#endif

typedef struct {
    gchar *html_doc;
    gchar *base_uri;
    gchar *window_title;
} AppData;

typedef struct {
    gboolean prefers_dark;
    gchar *font_name;
    gchar *monospace_font_name;
} InterfaceAppearance;

static InterfaceAppearance get_interface_appearance(void) {
    InterfaceAppearance appearance = {
        .prefers_dark = FALSE,
        .font_name = g_strdup("Sans 11"),
        .monospace_font_name = g_strdup("Monospace 11"),
    };

    GSettingsSchemaSource *source = g_settings_schema_source_get_default();
    if (source == NULL) {
        return appearance;
    }

    GSettingsSchema *schema = g_settings_schema_source_lookup(source, "org.gnome.desktop.interface", TRUE);
    if (schema == NULL) {
        return appearance;
    }

    GSettings *settings = g_settings_new("org.gnome.desktop.interface");

    if (g_settings_schema_has_key(schema, "color-scheme")) {
        g_autofree gchar *color_scheme = g_settings_get_string(settings, "color-scheme");
        appearance.prefers_dark = g_strcmp0(color_scheme, "prefer-dark") == 0;
    } else {
        GtkSettings *gtk_settings = gtk_settings_get_default();
        gboolean prefer_dark_theme = FALSE;
        if (gtk_settings != NULL &&
            g_object_class_find_property(G_OBJECT_GET_CLASS(gtk_settings), "gtk-application-prefer-dark-theme") != NULL) {
            g_object_get(gtk_settings, "gtk-application-prefer-dark-theme", &prefer_dark_theme, NULL);
            appearance.prefers_dark = prefer_dark_theme;
        }
    }

    if (g_settings_schema_has_key(schema, "font-name")) {
        g_autofree gchar *font_name = g_settings_get_string(settings, "font-name");
        if (font_name != NULL && *font_name != '\0') {
            g_free(appearance.font_name);
            appearance.font_name = g_strdup(font_name);
        }
    }

    if (g_settings_schema_has_key(schema, "monospace-font-name")) {
        g_autofree gchar *mono_name = g_settings_get_string(settings, "monospace-font-name");
        if (mono_name != NULL && *mono_name != '\0') {
            g_free(appearance.monospace_font_name);
            appearance.monospace_font_name = g_strdup(mono_name);
        }
    }

    g_object_unref(settings);
    g_settings_schema_unref(schema);
    return appearance;
}

static void free_interface_appearance(InterfaceAppearance *appearance) {
    g_free(appearance->font_name);
    g_free(appearance->monospace_font_name);
    appearance->font_name = NULL;
    appearance->monospace_font_name = NULL;
}

static gchar *font_css_from_pango_desc(const gchar *font_desc, const gchar *fallback_family) {
    PangoFontDescription *desc = pango_font_description_from_string(font_desc);
    const gchar *family = pango_font_description_get_family(desc);
    if (family == NULL || *family == '\0') {
        family = fallback_family;
    }

    gint size = pango_font_description_get_size(desc);
    if (size <= 0) {
        size = 11 * PANGO_SCALE;
    }
    gdouble size_value = (gdouble)size / PANGO_SCALE;
    const gchar *size_unit = pango_font_description_get_size_is_absolute(desc) ? "px" : "pt";

    PangoStyle style = pango_font_description_get_style(desc);
    const gchar *style_css = (style == PANGO_STYLE_ITALIC || style == PANGO_STYLE_OBLIQUE) ? "italic" : "normal";

    PangoWeight weight = pango_font_description_get_weight(desc);
    gint weight_css = weight >= PANGO_WEIGHT_BOLD ? 700 : 400;

    gchar *css = g_strdup_printf(
        "font-family:'%s';font-size:%.1f%s;font-style:%s;font-weight:%d;",
        family, size_value, size_unit, style_css, weight_css);

    pango_font_description_free(desc);
    return css;
}

static gchar *build_base_uri_from_filename(const gchar *filename) {
    g_autofree gchar *abs_path = g_canonicalize_filename(filename, NULL);
    g_autofree gchar *dir_path = g_path_get_dirname(abs_path);
    return g_filename_to_uri(dir_path, NULL, NULL);
}

static gchar *wrap_html_document(const gchar *title,
                                 const gchar *body_html,
                                 const InterfaceAppearance *appearance) {
    g_autofree gchar *body_font_css = font_css_from_pango_desc(appearance->font_name, "Sans");
    g_autofree gchar *mono_font_css = font_css_from_pango_desc(appearance->monospace_font_name, "Monospace");
    const gchar *page_bg = appearance->prefers_dark ? "#1f2126" : "#f6f7f8";
    const gchar *text_color = appearance->prefers_dark ? "#e6e8ec" : "#1b1b1b";
    const gchar *surface_bg = appearance->prefers_dark ? "#2b3037" : "#eceff3";
    const gchar *blockquote_color = appearance->prefers_dark ? "#b8c0cc" : "#364152";
    const gchar *border_color = appearance->prefers_dark ? "#4d5766" : "#a7b3c3";
    const gchar *link_color = appearance->prefers_dark ? "#78b3ff" : "#005f9e";
    const gchar *table_border = appearance->prefers_dark ? "#586171" : "#c8d0da";
    const gchar *color_scheme = appearance->prefers_dark ? "dark" : "light";

    return g_strdup_printf(
        "<!doctype html>"
        "<html><head>"
        "<meta charset=\"utf-8\">"
        "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
        "<meta name=\"color-scheme\" content=\"light dark\">"
        "<title>%s</title>"
        "<style>"
        ":root{color-scheme:%s;}"
        "body{max-width:860px;margin:2rem auto;padding:0 1rem;"
        "%sline-height:1.6;color:%s;background:%s;}"
        "pre,code{%s}"
        "pre{padding:0.9rem;overflow:auto;background:%s;border-radius:8px;}"
        "blockquote{margin-left:0;padding-left:1rem;border-left:4px solid %s;color:%s;}"
        "img{max-width:100%%;height:auto;}"
        "a{color:%s;text-decoration:none;}a:hover{text-decoration:underline;}"
        "table{border-collapse:collapse;}th,td{border:1px solid %s;padding:0.35rem 0.5rem;}"
        "</style>"
        "</head><body>%s</body></html>",
        title,
        color_scheme,
        body_font_css, text_color, page_bg,
        mono_font_css,
        surface_bg, border_color, blockquote_color,
        link_color, table_border,
        body_html);
}

static void activate(GtkApplication *app, gpointer user_data) {
    AppData *data = (AppData *)user_data;
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), data->window_title);
    gtk_window_set_default_size(GTK_WINDOW(window), 1100, 760);
#if !GTK_CHECK_VERSION(4, 0, 0)
    gtk_window_set_icon_name(GTK_WINDOW(window), "vsc-mdview");
#endif

    GtkWidget *webview = webkit_web_view_new();

#if GTK_CHECK_VERSION(4, 0, 0)
    gtk_window_set_child(GTK_WINDOW(window), webview);
    gtk_window_present(GTK_WINDOW(window));
#else
    gtk_container_add(GTK_CONTAINER(window), webview);
    gtk_widget_show_all(window);
#endif

    webkit_web_view_load_html(WEBKIT_WEB_VIEW(webview), data->html_doc, data->base_uri);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        g_printerr("Usage: %s <markdown-file>\n", argv[0]);
        return 1;
    }

    const gchar *filename = argv[1];
    g_autofree gchar *markdown = NULL;
    gsize markdown_len = 0;

    GError *err = NULL;
    if (!g_file_get_contents(filename, &markdown, &markdown_len, &err)) {
        g_printerr("Failed to read '%s': %s\n", filename, err->message);
        g_clear_error(&err);
        return 1;
    }

    g_autofree gchar *body_html = cmark_markdown_to_html(markdown, markdown_len, CMARK_OPT_DEFAULT);
    if (body_html == NULL) {
        g_printerr("Failed to convert markdown to HTML.\n");
        return 1;
    }

    g_autofree gchar *basename = g_path_get_basename(filename);
    g_autofree gchar *title = g_strdup_printf("vsc-mdview - %s", basename);
    InterfaceAppearance appearance = get_interface_appearance();
    g_autofree gchar *doc_html = wrap_html_document(title, body_html, &appearance);
    g_autofree gchar *base_uri = build_base_uri_from_filename(filename);

    AppData data = {
        .html_doc = doc_html,
        .base_uri = base_uri,
        .window_title = title,
    };

    GtkApplication *app = gtk_application_new("com.example.vsc_mdview", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), &data);
    int status = g_application_run(G_APPLICATION(app), 1, argv);
    g_object_unref(app);
    free_interface_appearance(&appearance);
    return status;
}
