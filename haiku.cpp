#include <Application.h>
#include <InterfaceDefs.h>
#include <View.h>
#include <Window.h>

#include "puzzles.h"

const int WINDOW_SIZE = 375;

/* C++ classes */
class PuzzlesApp : public BApplication {
	public:
		PuzzlesApp()
		: BApplication("application/x-vnd.SimonTatham.Puzzles") {
			window = new BWindow(
			BRect(0, 0, WINDOW_SIZE, WINDOW_SIZE), "", B_TITLED_WINDOW,
			B_NOT_RESIZABLE | B_QUIT_ON_WINDOW_CLOSE);		
			window->Show();
		}
		
		BWindow *window;
		BView *view;
};

/* Frontend structure */
struct frontend {
	const game *gg;
	midend *me;
	PuzzlesApp *app;
};

/* Drawing functions */
static void be_draw_text(void *handle, int x, int y, int fonttype, int fontsize, int align, int colour, const char *text){}
static void be_draw_rect(void *handle, int x, int y, int w, int h, int colour){}
static void be_draw_line(void *handle, int x1, int y1, int x2, int y2, int colour){}
static void be_draw_polygon(void *handle, const int *coords, int npoints, int fillcolour, int outlinecolour){}
static void be_draw_circle(void *handle, int cx, int cy, int radius, int fillcolour, int outlinecolour){}
static void be_draw_update(void *handle, int x, int y, int w, int h){}
static void be_clip(void *handle, int x, int y, int w, int h){}
static void be_unclip(void *handle){}
static void be_start_draw(void *handle){}
static void be_end_draw(void *handle){}
static void be_status_bar(void *handle, const char *text){}
static blitter *be_blitter_new(void *handle, int w, int h){return NULL;}
static void be_blitter_free(void *handle, blitter *bl){}
static void be_blitter_save(void *handle, blitter *bl, int x, int y){}
static void be_blitter_load(void *handle, blitter *bl, int x, int y){}
static void be_begin_doc(void *handle, int pages){}
static void be_begin_page(void *handle, int number){}
static void be_begin_puzzle(void *handle, float xm, float xc, float ym, float yc, int pw, int ph, float wmm){}
static void be_end_puzzle(void *handle){}
static void be_end_page(void *handle, int number){}
static void be_end_doc(void *handle){}
static void be_line_width(void *handle, float width){}
static void be_line_dotted(void *handle, bool dotted){}
static char *be_text_fallback(void *handle, const char *const *strings, int nstrings){return NULL;}
static void be_draw_thick_line(void *handle, float thickness, float x1, float y1, float x2, float y2, int colour){}

const struct drawing_api be_drawing = {
	be_draw_text,
	be_draw_rect,
	be_draw_line,
	be_draw_polygon,
	be_draw_circle,
	be_draw_update,
	be_clip,
	be_unclip,
	be_start_draw,
	be_end_draw,
	be_status_bar,
	be_blitter_new,
	be_blitter_free,
	be_blitter_save,
	be_blitter_load,
	be_begin_doc,
	be_begin_page,
	be_begin_puzzle,
	be_end_puzzle,
	be_end_page,
	be_end_doc,
	be_line_width,
	be_line_dotted,
	be_text_fallback,
	be_draw_thick_line
};

/* Miscellaneous functions */
                  
void get_random_seed(void **randseed, int *randseedsize) {
	
}

void activate_timer(frontend *fe) {
	
}

void deactivate_timer(frontend *fe) {
	
}

void fatal(const char *fmt, ...) {
	
}

void frontend_default_colour(frontend *fe, float *output) {
	
}

/* Application class and main function */
int main() {
	frontend *fe;
	
	/* Initialize the front end */
	fe->gg = &thegame;
	fe->me = midend_new(fe, fe->gg, &be_drawing, fe);
	fe->app = new PuzzlesApp();
			
	fe->app->Run();
	
	/* Finalize the front end */
	fe->gg = NULL;
	midend_free(fe->me);
	delete fe->app;
	delete fe;
		
	return 0;
}	
