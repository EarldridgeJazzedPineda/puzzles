/*
 * Haiku front end for Simon Tatham's Portable Puzzle Collection
 * by Earldridge Jazzed Pineda
 */
 
/* List of known bugs:
   * Puzzles can sometimes crash at startup.
   * Puzzles that use animation render it too fast.
   * In Black Box, Light Up, Signpost, Singles, Towers, and Unequal,
   squares that are normally grey are rendered black or green. This can
   make these puzzles unplayable.
   * In Bridges, the text is missing.
   * When trying to play Guess, Pegs, or Signpost, the app crashes.
   * Inertia crashes at startup.
   * Some vertical lines can show up in the grid for Map.
   * Right-click doesn't work for Mines.
   * Arrow keys don't work for Net.
   * Some animation glitches can occur when playing Netslide, Sixteen,
   or Twiddle.
   * In Tracks, Some tracks render outside of the grid.
   * In Undead, the controls are broken and monsters aren't rendered
   properly.
*/

/* To do:
   * Add preset menu
   * Make windows resizable
   * Add a status bar
   * Add dialog boxes for puzzle configuration
   * Add support for save files
   * Add support for printing
   * Use .rsrc files
   */
 
#include <Application.h>
#include <Alert.h>
#include <Clipboard.h>
#include <InterfaceDefs.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <Message.h>
#include <Polygon.h>
#include <View.h>
#include <Window.h>
	
#include <ctime>
#include "puzzles.h"

const int PULSE_RATE = 20;
const int WINDOW_SIZE = 375;
const int MENU_BAR_HEIGHT = 22;

enum {
	NEW_GAME = 'gNew',
	RESTART_GAME = 'gRst',
	UNDO_GAME = 'gUnd',
	REDO_GAME = 'gRed',
	COPY_GAME = 'gCpy',
	SOLVE_GAME = 'gSlv'
};

/* Prototypes */
class PuzzlesApp;

void change_undo_redo_menus();

/* Frontend structure */
struct frontend {
	const game *gg;
	midend *me;
	float *colours;
	int *ncolours;
	PuzzlesApp *app;
};

frontend *fe;

/* C++ classes */
class PuzzlesView : public BView {
	public:
		PuzzlesView(BRect frame) : BView(frame, NULL,
		B_FOLLOW_LEFT_RIGHT | B_FOLLOW_BOTTOM,
		B_WILL_DRAW | B_PULSE_NEEDED) {}
		void Draw(BRect updaterect) {
			midend_force_redraw(fe->me);
		}
		void MouseDown(BPoint point) {
			unsigned int button;
			GetMouse(NULL, &button);
			switch (button) {
				case B_PRIMARY_MOUSE_BUTTON:
					midend_process_key(fe->me,
					point.x, point.y, LEFT_BUTTON);
					break;
				case B_SECONDARY_MOUSE_BUTTON:
					midend_process_key(fe->me,
					point.x, point.y, RIGHT_BUTTON);
					break;
				case B_TERTIARY_MOUSE_BUTTON:
					midend_process_key(fe->me,
					point.x, point.y, MIDDLE_BUTTON);
					break;
			}
			midend_redraw(fe->me);
			change_undo_redo_menus();
		}
		void MouseMoved(BPoint point, uint32 transit, const BMessage* message) {
			unsigned int button;
			GetMouse(NULL, &button);
			
			if (button == 0) return;
			
			if (transit == B_INSIDE_VIEW) {
				switch (button) {
					case B_PRIMARY_MOUSE_BUTTON:
						midend_process_key(fe->me,
						point.x, point.y, LEFT_DRAG);
						break;
					case B_SECONDARY_MOUSE_BUTTON:
						midend_process_key(fe->me,
						point.x, point.y, RIGHT_DRAG);
						break;
					case B_TERTIARY_MOUSE_BUTTON:
						midend_process_key(fe->me,
						point.x, point.y, MIDDLE_DRAG);
						break;
				}
				midend_redraw(fe->me);
				change_undo_redo_menus();
			}
		}
		void MouseUp(BPoint point) {
			unsigned int button;
			GetMouse(NULL, &button);
			switch (button) {
				case B_PRIMARY_MOUSE_BUTTON:
					midend_process_key(fe->me,
					point.x, point.y, LEFT_RELEASE);
					break;
				case B_SECONDARY_MOUSE_BUTTON:
					midend_process_key(fe->me,
					point.x, point.y, RIGHT_RELEASE);
					break;
				case B_TERTIARY_MOUSE_BUTTON:
					midend_process_key(fe->me,
					point.x, point.y, MIDDLE_RELEASE);
					break;
			}
			midend_redraw(fe->me);
			change_undo_redo_menus();
		}
		void Pulse() {
			midend_timer(fe->me, PULSE_RATE / 1000.0F);
		}
};

class PuzzlesWindow : public BWindow {
	public:
		PuzzlesWindow(int width, int height) : BWindow(
		BRect(0, 0, width, height + MENU_BAR_HEIGHT), fe->gg->name,
		B_TITLED_WINDOW, B_NOT_RESIZABLE | B_QUIT_ON_WINDOW_CLOSE) {
			SetPulseRate(0);
		}
		
		void MessageReceived(BMessage *message) {
			switch (message->what) {
				case B_KEY_DOWN: {
					int key, rawChar, modifiers, ctrl = 0, shift = 0;
					message->FindInt32("key", &key);
					message->FindInt32("modifiers", &modifiers);
					message->FindInt32("raw_char", &rawChar);
					if (modifiers | B_COMMAND_KEY) {
						ctrl = MOD_CTRL;
					}
					if (modifiers | B_SHIFT_KEY) {
						shift = MOD_SHFT;
					}
					switch (key) {	
						/* Cursor keys */
						case 0x57: midend_process_key(fe->me, 0, 0, ctrl | shift | CURSOR_UP); break;
						case 0x62: midend_process_key(fe->me, 0, 0, ctrl | shift | CURSOR_DOWN); break;
						case 0x61: midend_process_key(fe->me, 0, 0, ctrl | shift | CURSOR_LEFT); break;
						case 0x63: midend_process_key(fe->me, 0, 0, ctrl | shift | CURSOR_RIGHT); break;
						/* Selection keys */
						case 0x47: midend_process_key(fe->me, 0, 0, CURSOR_SELECT); break;
						case 0x5e: midend_process_key(fe->me, 0, 0, CURSOR_SELECT2); break;
						/* Numeric keypad */
						case 0x64: midend_process_key(fe->me, 0, 0, MOD_NUM_KEYPAD | '0'); break;
						case 0x58: midend_process_key(fe->me, 0, 0, MOD_NUM_KEYPAD | '1'); break;
						case 0x59: midend_process_key(fe->me, 0, 0, MOD_NUM_KEYPAD | '2'); break;
						case 0x5a: midend_process_key(fe->me, 0, 0, MOD_NUM_KEYPAD | '3'); break;
						case 0x48: midend_process_key(fe->me, 0, 0, MOD_NUM_KEYPAD | '4'); break;
						case 0x49: midend_process_key(fe->me, 0, 0, MOD_NUM_KEYPAD | '5'); break;
						case 0x4a: midend_process_key(fe->me, 0, 0, MOD_NUM_KEYPAD | '6'); break;
						case 0x37: midend_process_key(fe->me, 0, 0, MOD_NUM_KEYPAD | '7'); break;
						case 0x38: midend_process_key(fe->me, 0, 0, MOD_NUM_KEYPAD | '8'); break;
						case 0x39: midend_process_key(fe->me, 0, 0, MOD_NUM_KEYPAD | '9'); break;
						/* ASCII keys */
						default:
						if (rawChar >= 33 && rawChar <= 126) {
							midend_process_key(fe->me, 0, 0, ctrl | shift | rawChar);
						}
					}
					break;
				} case NEW_GAME:
					midend_process_key(fe->me, 0, 0, UI_NEWGAME);
					break;
				case RESTART_GAME:
					midend_restart_game(fe->me);
					break;
				case UNDO_GAME:
					midend_process_key(fe->me, 0, 0, UI_UNDO);
					change_undo_redo_menus();
					break;
				case REDO_GAME:
					midend_process_key(fe->me, 0, 0, UI_REDO);
					change_undo_redo_menus();
					break;
				case COPY_GAME:
					if (midend_can_format_as_text_now(fe->me)) {
						if (be_clipboard->Lock()) {
							be_clipboard->Clear();
							BMessage *clipboard = be_clipboard->Data();
							BString format(midend_text_format(fe->me));
							clipboard->AddData("text/plain",
							B_MIME_TYPE, format, format.Length());
							be_clipboard->Commit();
							be_clipboard->Unlock();
						}
					}
					break;
				case SOLVE_GAME:
					midend_process_key(fe->me, 0, 0, UI_SOLVE);
					break;
			}
		}
};

class PuzzlesApp : public BApplication {
	public:
		PuzzlesApp()
		: BApplication("application/x-vnd.SimonTatham.Puzzles") {
			int width = WINDOW_SIZE, height = WINDOW_SIZE;
			midend_size(fe->me, &width, &height, true, 1);
			window = new PuzzlesWindow(width, height);
			view = new PuzzlesView(BRect(0, MENU_BAR_HEIGHT, width,
			       height + MENU_BAR_HEIGHT));
			// view->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
			window->AddChild(view);
				
			BMenuBar *menuBar = new BMenuBar(
			BRect(0, 0, width, MENU_BAR_HEIGHT), "");
			
			BMenu *puzzleMenu = new BMenu("File");
			puzzleMenu->AddItem(new BMenuItem("New", new BMessage(NEW_GAME), 'N'));
			puzzleMenu->AddItem(new BSeparatorItem());
			puzzleMenu->AddItem(new BMenuItem("Restart", new BMessage(RESTART_GAME)));
			menuBar->AddItem(puzzleMenu);
			
			BMenu *editMenu = new BMenu("Edit");
			undoMenuItem = new BMenuItem("Undo", new BMessage(UNDO_GAME), 'Z');
			editMenu->AddItem(undoMenuItem);
			redoMenuItem = new BMenuItem("Redo", new BMessage(REDO_GAME), 'Z', B_SHIFT_KEY);
			editMenu->AddItem(redoMenuItem);
			if (fe->gg->can_format_as_text_ever) {
				editMenu->AddItem(new BSeparatorItem());
				editMenu->AddItem(new BMenuItem("Copy", new BMessage(COPY_GAME), 'C'));
			}
			if (fe->gg->can_solve) {
				editMenu->AddItem(new BSeparatorItem());
				editMenu->AddItem(new BMenuItem("Solve", new BMessage(SOLVE_GAME)));
			}
			menuBar->AddItem(editMenu);
			
			window->AddChild(menuBar);
			
			window->CenterOnScreen();
			window->Show();
		}
		PuzzlesWindow *window;
		PuzzlesView *view;
		BMenuItem *undoMenuItem;
		BMenuItem *redoMenuItem;
};

/* Drawing functions */
static void be_draw_text(void *handle, int x, int y, int fonttype, int fontsize, int align, int colour, const char *text){
	/* Select global BFont based on fonttype */
	BFont font;
	switch (fonttype) {
		case FONT_FIXED: font = BFont(be_fixed_font); break;
		case FONT_VARIABLE: font = BFont(be_plain_font); break;
	}
	
	/* Set font size */
	font.SetSize(fontsize);
	
	/* Determine width of text (used for alignment) */
	float width = font.StringWidth(text);
	
	/* Set the font */
	fe->app->view->SetFont(&font);
	
	/* Create the point */
	BPoint point(x, y);
	
	/* Do vertical alignment */
	if (align & ALIGN_VCENTRE) {
		point.y += fontsize / 2;
	}
	
	/* Do horizontal alignment */
	if (align & ALIGN_HLEFT) {
		point.x += width / 2;
	}
	if (align & ALIGN_HRIGHT) {
		point.x -= width / 2;
	}
	
	/* Draw the text */
	int r = fe->colours[colour * 3] * 255;
	int g = fe->colours[colour * 3 + 1] * 255;
	int b = fe->colours[colour * 3 + 2] * 255;
	fe->app->view->SetHighColor(r, g, b);
	fe->app->view->DrawString(text, point);
}
static void be_draw_rect(void *handle, int x, int y, int w, int h, int colour) {
	int r = fe->colours[colour * 3] * 255;
	int g = fe->colours[colour * 3 + 1] * 255;
	int b = fe->colours[colour * 3 + 2] * 255;
	fe->app->view->SetHighColor(r, g, b);
	fe->app->view->FillRect(BRect(x, y, x+w-1, y+h-1));
}
static void be_draw_line(void *handle, int x1, int y1, int x2, int y2, int colour){
	int r = fe->colours[colour * 3] * 255;
	int g = fe->colours[colour * 3 + 1] * 255;
	int b = fe->colours[colour * 3 + 2] * 255;
	fe->app->view->SetHighColor(r, g, b);
	fe->app->view->StrokeLine(BPoint(x1, y1), BPoint(x2, y2));
}
static void be_draw_polygon(void *handle, const int *coords, int npoints, int fillcolour, int outlinecolour){
	BPoint points[npoints];
	
	for (int i = 0; i < npoints; i++) {
		points[i].Set(coords[i*2], coords[i*2+1]);
	}
	
	BPolygon *polygon = new BPolygon(points, npoints);
	
	int r = fe->colours[fillcolour * 3] * 255;
	int g = fe->colours[fillcolour * 3 + 1] * 255;
	int b = fe->colours[fillcolour * 3 + 2] * 255;
	fe->app->view->SetHighColor(r, g, b);
	fe->app->view->FillPolygon(polygon);
	r = fe->colours[outlinecolour * 3] * 255;
	g = fe->colours[outlinecolour * 3 + 1] * 255;
	b = fe->colours[outlinecolour * 3 + 2] * 255;
	fe->app->view->SetHighColor(r, g, b);
	fe->app->view->StrokePolygon(polygon);
	
	delete polygon;
}
static void be_draw_circle(void *handle, int cx, int cy, int radius, int fillcolour, int outlinecolour){
	int r = fe->colours[fillcolour * 3] * 255;
	int g = fe->colours[fillcolour * 3 + 1] * 255;
	int b = fe->colours[fillcolour * 3 + 2] * 255;
	fe->app->view->SetHighColor(r, g, b);
	fe->app->view->FillEllipse(BPoint(cx, cy), radius, radius);
	r = fe->colours[outlinecolour * 3] * 255;
	g = fe->colours[outlinecolour * 3 + 1] * 255;
	b = fe->colours[outlinecolour * 3 + 2] * 255;
	fe->app->view->SetHighColor(r, g, b);
	fe->app->view->StrokeEllipse(BPoint(cx, cy), radius, radius);
}
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

static void be_draw_thick_line(void *handle, float thickness, float x1, float y1, float x2, float y2, int colour){
	int r = fe->colours[colour * 3] * 255;
	int g = fe->colours[colour * 3 + 1] * 255;
	int b = fe->colours[colour * 3 + 2] * 255;
	fe->app->view->SetPenSize(thickness);
	fe->app->view->SetHighColor(r, g, b);
	fe->app->view->StrokeLine(BPoint(x1, y1), BPoint(x2, y2));
	
}

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
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	be_draw_thick_line
};

/* Miscellaneous functions */
                  
void get_random_seed(void **randseed, int *randseedsize) {
	time_t *t = new time_t;
	time(t);
	*randseed = t;
	*randseedsize = sizeof(time_t);
}

void activate_timer(frontend *fe) {
	if (fe->app != NULL) {
		fe->app->window->SetPulseRate(PULSE_RATE);
	}
}

void deactivate_timer(frontend *fe) {
	if (fe->app != NULL) {
		fe->app->window->SetPulseRate(0);
	}
}

void fatal(const char *fmt, ...) {
    char buf[2048];
    va_list ap;

    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    va_end(ap);
    
    BAlert *alert = new BAlert("Fatal error", buf, "Exit Puzzles", NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
    alert->Go();
    
    exit(1);
}

void frontend_default_colour(frontend *fe, float *output) {
	rgb_color c = ui_color(B_PANEL_BACKGROUND_COLOR);
	
	output[0] = c.red / 255.0F;
	output[1] = c.green / 255.0F;
	output[2] = c.blue / 255.0F;
}

/* Frontend-specific functions */

void change_undo_redo_menus() {
	fe->app->undoMenuItem->SetEnabled(midend_can_undo(fe->me));
	fe->app->redoMenuItem->SetEnabled(midend_can_redo(fe->me));
}

/* Application class and main function */

int main() {
	/* Initialize the front end */
	fe = new frontend;
	fe->gg = &thegame;
	fe->me = midend_new(fe, &thegame, &be_drawing, NULL);
	midend_new_game(fe->me);
	fe->ncolours = new int;
	fe->colours = midend_colours(fe->me, fe->ncolours);
	
	fe->app = new PuzzlesApp();	
	fe->app->Run();
	
	/* Finalize the front end */
	fe->gg = NULL;
	midend_free(fe->me);
	delete fe->app;
	delete fe;
	
	return 0;
}	
