/* macos.c */
/* implements Mac OS X specific functions */
#include "dive.h"
#include "display-gtk.h"
#include <CoreFoundation/CoreFoundation.h>
#include <mach-o/dyld.h>
#include "gtkosxapplication.h"

static GtkOSXApplication *osx_app;

/* macos defines CFSTR to create a CFString object from a constant,
 * but no similar macros if a C string variable is supposed to be
 * the argument. We add this here (hardcoding the default allocator
 * and MacRoman encoding */
#define CFSTR_VAR(_var) CFStringCreateWithCStringNoCopy(kCFAllocatorDefault,	\
					(_var), kCFStringEncodingMacRoman,	\
					kCFAllocatorNull)

#define SUBSURFACE_PREFERENCES CFSTR("org.hohndel.subsurface")
#define ICON_NAME "Subsurface.icns"
#define UI_FONT "Arial Unicode MS 12"
#define DIVELIST_MAC_DEFAULT_FONT "Arial Unicode MS 9"

void subsurface_open_conf(void)
{
	/* nothing at this time */
}

void subsurface_set_conf(char *name, pref_type_t type, const void *value)
{
	switch (type) {
	case PREF_BOOL:
		CFPreferencesSetAppValue(CFSTR_VAR(name),
			value == NULL ? kCFBooleanFalse : kCFBooleanTrue, SUBSURFACE_PREFERENCES);
		break;
	case PREF_STRING:
		CFPreferencesSetAppValue(CFSTR_VAR(name), CFSTR_VAR(value), SUBSURFACE_PREFERENCES);
	}
}

const void *subsurface_get_conf(char *name, pref_type_t type)
{
	Boolean boolpref;
	CFPropertyListRef strpref;

	switch (type) {
	case PREF_BOOL:
		boolpref = CFPreferencesGetAppBooleanValue(CFSTR_VAR(name), SUBSURFACE_PREFERENCES, FALSE);
		if (boolpref)
			return (void *) 1;
		else
			return NULL;
	case PREF_STRING:
		strpref = CFPreferencesCopyAppValue(CFSTR_VAR(name), SUBSURFACE_PREFERENCES);
		if (!strpref)
			return NULL;
		return strdup(CFStringGetCStringPtr(strpref, kCFStringEncodingMacRoman));
	}
	/* we shouldn't get here, but having this line makes the compiler happy */
	return NULL;
}

void subsurface_flush_conf(void)
{
	int ok = CFPreferencesAppSynchronize(SUBSURFACE_PREFERENCES);
	if (!ok)
		fprintf(stderr,"Could not save preferences\n");
}

void subsurface_close_conf(void)
{
	/* Nothing */
}

int subsurface_fill_device_list(GtkListStore *store)
{
	int i = 0;
	int index = -1;
	GtkTreeIter iter;
	GDir *dev;
	const char *name;
	char *buffer;
	gsize length;

	dev = g_dir_open("/dev", 0, NULL);
	while (dev && (name = g_dir_read_name(dev)) != NULL) {
		if (strstr(name, "usbserial")) {
			int len = strlen(name) + 6;
			char *devicename = malloc(len);
			snprintf(devicename, len, "/dev/%s", name);
			gtk_list_store_append(store, &iter);
			gtk_list_store_set(store, &iter,
					0, devicename, -1);
			if (is_default_dive_computer_device(devicename))
				index = i;
			i++;
		}
	}
	if (dev)
		g_dir_close(dev);
	dev = g_dir_open("/Volumes", 0, NULL);
	while (dev && (name = g_dir_read_name(dev)) != NULL) {
		if (strstr(name, "UEMISSDA")) {
			int len = strlen(name) + 10;
			char *devicename = malloc(len);
			snprintf(devicename, len, "/Volumes/%s", name);
			gtk_list_store_append(store, &iter);
			gtk_list_store_set(store, &iter,
					0, devicename, -1);
			if (is_default_dive_computer_device(devicename))
				index = i;
			i++;
		}
	}
	if (dev)
		g_dir_close(dev);
	if (i == 0) {
		/* if we can't find anything, use the default */
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter,
				0, "/dev/tty.SLAB_USBtoUART", -1);
		if (is_default_dive_computer_device("/dev/tty.SLAB_USBtoUART"))
			index = i;
	}
	return index;
}

const char *subsurface_icon_name()
{
	static char path[1024];

	snprintf(path, 1024, "%s/%s", quartz_application_get_resource_path(), ICON_NAME);

	return path;
}

const char *subsurface_default_filename()
{
	if (default_filename) {
		return strdup(default_filename);
	} else {
		const char *home, *user;
		char *buffer;
		int len;

		home = g_get_home_dir();
		user = g_get_user_name();
		len = strlen(home) + strlen(user) + 45;
		buffer = malloc(len);
		snprintf(buffer, len, "%s/Library/Application Support/Subsurface/%s.xml", home, user);
		return buffer;
	}
}

const char *subsurface_gettext_domainpath(char *argv0)
{
	/* on a Mac we ignore the argv0 argument and instead use the resource_path
	 * to figure out where to find the translation files */
	static char buffer[256];
	const char *resource_path = quartz_application_get_resource_path();
	if (resource_path) {
		snprintf(buffer, sizeof(buffer), "%s/share/locale", resource_path);
		return buffer;
	}
	return "./share/locale";
}

static void show_main_window(GtkWidget *w, gpointer data)
{
	gtk_widget_show(main_window);
	gtk_window_present(GTK_WINDOW(main_window));
}

void subsurface_ui_setup(GtkSettings *settings, GtkWidget *menubar,
		GtkWidget *vbox, GtkUIManager *ui_manager)
{
	GtkWidget *menu_item, *sep;

	if (!divelist_font)
		divelist_font = strdup(DIVELIST_MAC_DEFAULT_FONT);
	g_object_set(G_OBJECT(settings), "gtk-font-name", UI_FONT, NULL);

	osx_app = g_object_new(GTK_TYPE_OSX_APPLICATION, NULL);
	gtk_widget_hide (menubar);
	gtk_osxapplication_set_menu_bar(osx_app, GTK_MENU_SHELL(menubar));

	sep = gtk_ui_manager_get_widget(ui_manager, "/MainMenu/FileMenu/Separator3");
	if (sep)
		gtk_widget_destroy(sep);

	menu_item = gtk_ui_manager_get_widget(ui_manager, "/MainMenu/FileMenu/Quit");
	gtk_widget_hide (menu_item);
	menu_item = gtk_ui_manager_get_widget(ui_manager, "/MainMenu/Help/About");
	gtk_osxapplication_insert_app_menu_item(osx_app, menu_item, 0);

	sep = gtk_separator_menu_item_new();
	g_object_ref(sep);
	gtk_osxapplication_insert_app_menu_item (osx_app, sep, 1);

	menu_item = gtk_ui_manager_get_widget(ui_manager, "/MainMenu/FileMenu/Preferences");
	gtk_osxapplication_insert_app_menu_item(osx_app, menu_item, 2);

	sep = gtk_separator_menu_item_new();
	g_object_ref(sep);
	gtk_osxapplication_insert_app_menu_item (osx_app, sep, 3);

	gtk_osxapplication_set_use_quartz_accelerators(osx_app, TRUE);
	g_signal_connect(osx_app,"NSApplicationDidBecomeActive",G_CALLBACK(show_main_window),NULL);
	g_signal_connect(osx_app,"NSApplicationWillTerminate",G_CALLBACK(quit),NULL);

	gtk_osxapplication_ready(osx_app);
}

void subsurface_command_line_init(gint *argc, gchar ***argv)
{
	/* this is a no-op */
}

void subsurface_command_line_exit(gint *argc, gchar ***argv)
{
	/* this is a no-op */
}

gboolean subsurface_os_feature_available(os_feature_t f)
{
	return TRUE;
}
