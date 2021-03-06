#ifndef DISPLAY_H
#define DISPLAY_H

#include <cairo.h>

#define DPI_SCREEN 72.0
#define SCALE_SCREEN 1.0
#define SCALE_PRINT (1.0 / DPI_SCREEN)

extern void repaint_dive(void);
extern void do_print(void);

/*
 * Cairo scaling really is horribly horribly mis-designed.
 *
 * Which is sad, because I really like Cairo otherwise. But
 * the fact that the line width is scaled with the same scale
 * as the coordinate system is a f*&%ing disaster. So we
 * can't use it, and instead have this butt-ugly wrapper thing..
 */
struct graphics_context {
	int printer;
	cairo_t *cr;
	cairo_rectangle_t drawing_area;
	double maxx, maxy;
	double leftx, rightx;
	double topy, bottomy;
	unsigned int maxtime;
	void *plot_info;
};

typedef enum { SC_SCREEN, SC_PRINT } scale_mode_t;

extern void plot(struct graphics_context *gc, struct dive *dive, scale_mode_t scale);
extern void init_profile_background(struct graphics_context *gc);
extern void attach_tooltip(int x, int y, int w, int h, const char *text);
extern void get_plot_details(struct graphics_context *gc, int time, char *buf, size_t bufsize);

struct options {
	enum { PRETTY, TABLE, ONEPERPAGE } type;
	int print_selected;
};

extern char zoomed_plot;

#endif
