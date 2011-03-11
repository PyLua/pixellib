#include "npixel.h"

ipaint_t *paint;

void drawing(void)
{
	ipixel_point_t pts[3] = { { 100, 80 }, { 110, 120 }, { 210, 110 } };
	int i;

	ipaint_fill(paint, NULL, 0xffffffff);

	ipaint_anti_aliasing(paint, 3);
	ipaint_draw_polygon(paint, pts, 3);

	for (i = 0; i < 3; i++) pts[i].x += 320;

	ipaint_anti_aliasing(paint, 0);
	ipaint_draw_polygon(paint, pts, 3);

	ipaint_set_color(paint, 0x900000ff);
	ipaint_anti_aliasing(paint, 3);

	ipaint_text_color(paint, 0xff00aaaa);
	ipaint_text_background(paint, 0);

	ipaint_cprintf(paint, 100, 8, "ANTI ALIASING ON");
	ipaint_cprintf(paint, 420, 8, "ANTI ALIASING OFF");
	ipaint_cprintf(paint, 100, 220, "     ZOOM");
	ipaint_cprintf(paint, 420, 220, "     ZOOM");

	// (100, 80, 210, 110) -> (100, 80, 110, 30)
	// 110, 40, 
	ibitmap_stretch(cscreen, 20, 280, 280, 76, cscreen, 100, 80, 110, 40, 0);
	ibitmap_stretch(cscreen,350, 280, 280, 76, cscreen, 420, 80, 110, 40, 0);
}

//! lib: .
//! link: pixel
//! win: 
int main(void)
{
	int retval;

	if ((retval = iscreen_init(640, 480, 32)) != 0) {
		printf("error init\n");
		return -1;
	}

	paint = ipaint_create(cscreen);

	drawing();

	while (iscreen_dispatch() == 0) {
		iscreen_tick(32);
		if (iscreen_keyon(IKEY_ESCAPE)) break;

		iscreen_convert(NULL, 1);
		iscreen_update(NULL, 1);
	}

	return 0;
}

