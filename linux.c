/* linux.c */
/* implements Linux specific functions */
#include "dive.h"
#include "display-gtk.h"
#include <gconf/gconf-client.h>
#include <string.h>

#define DIVELIST_DEFAULT_FONT "Sans 8"

GConfClient *gconf;

static char *gconf_name(char *name)
{
	static char buf[255] = "/apps/subsurface/";

	snprintf(buf, 255, "/apps/subsurface/%s", name);

	return buf;
}

void subsurface_open_conf(void)
{
	gconf = gconf_client_get_default();
}

void subsurface_set_conf(char *name, pref_type_t type, const void *value)
{
	switch (type) {
	case PREF_BOOL:
		gconf_client_set_bool(gconf, gconf_name(name), value != NULL, NULL);
		break;
	case PREF_STRING:
		gconf_client_set_string(gconf, gconf_name(name), value, NULL);
	}
}

const void *subsurface_get_conf(char *name, pref_type_t type)
{
	switch (type) {
	case PREF_BOOL:
		return gconf_client_get_bool(gconf, gconf_name(name), NULL) ? (void *) 1 : NULL;
	case PREF_STRING:
		return gconf_client_get_string(gconf, gconf_name(name), NULL);
	}
	/* we shouldn't get here */
	return NULL;
}

void subsurface_flush_conf(void)
{
	/* this is a no-op */
}

void subsurface_close_conf(void)
{
	/* this is a no-op */
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
		if (strstr(name, "USB")) {
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
	if (g_file_get_contents("/proc/mounts", &buffer, &length, NULL) &&
		length > 0) {
		char *ptr = strstr(buffer, "UEMISSDA");
		if (ptr) {
			char *end = ptr, *start = ptr;
			while (start > buffer && *start != ' ')
				start--;
			if (*start == ' ')
				start++;
			while (*end != ' ' && *end != '\0')
				end++;
			*end = '\0';
			name = strdup(start);
			gtk_list_store_append(store, &iter);
			gtk_list_store_set(store, &iter,
					0, name, -1);
			if (is_default_dive_computer_device(name))
				index = i;
			i++;
			free((void *)name);
		}
		g_free(buffer);
	}
	if (i == 0) {
		/* if we can't find anything, use the default */
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter,
				0, "/dev/ttyUSB0", -1);
		if (is_default_dive_computer_device("/dev/ttyUSB0"))
			index = i;
	}
	return index;
}

const char *subsurface_icon_name()
{
	return "subsurface.svg";
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
		len = strlen(home) + strlen(user) + 17;
		buffer = malloc(len);
		snprintf(buffer, len, "%s/subsurface/%s.xml", home, user);
		return buffer;
	}
}

const char *subsurface_gettext_domainpath(char *argv0)
{
	if (argv0[0] == '.') {
		/* we're starting a local copy */
		return "./share/locale";
	} else {
		/* subsurface is installed, so system dir should be fine */
		return NULL;
	}
}

void subsurface_ui_setup(GtkSettings *settings, GtkWidget *menubar,
		GtkWidget *vbox, GtkUIManager *ui_manager)
{
	if (!divelist_font)
		divelist_font = strdup(DIVELIST_DEFAULT_FONT);
	gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);
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
