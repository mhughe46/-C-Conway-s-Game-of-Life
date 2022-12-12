#pragma warning(push, 0)
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#pragma warning(pop)

#include <iostream>
#include <list>

using namespace std;

#define MAXROWS 200

int width;
int height;
int zoom = 16;
bool show_grid = 0;
bool running, editing = 1;
int x_pos = 0;
int y_pos = 0;
bool middle_mouse_down = 0;
bool show_UI = 0;

SDL_Renderer* renderer;
SDL_Window* window;

int frameCount, timerFPS, lastFrame, fps;

int values[MAXROWS][MAXROWS];
int valuesToBe[MAXROWS][MAXROWS];

class Square {
public:
	int x = 0;
	int y = 0;
	int value = 255;
	Square(int _x, int _y, int _value) {
		x = _x;
		y = _y;
		value = _value;
	}

};

int getNumberOfNeighbours(int x, int y) {

	int n = 0;

	if (x > 0) {
		if (values[x - 1][y] == 255) n++;
		if (y > 0 && values[x - 1][y - 1] == 255) n++;
		if (y < (MAXROWS - 1) && values[x - 1][y + 1] == 255) n++;
	}
	if (y > 0 && values[x][y - 1] == 255) n++;

	if (x < (MAXROWS - 1)) {
		if (values[x + 1][y] == 255) n++;
		if (y > 0 && values[x + 1][y - 1] == 255) n++;
		if (y < (MAXROWS - 1) && values[x + 1][y + 1] == 255) n++;
	}

	if (y < (MAXROWS - 1) && values[x][y + 1] == 255) n++;



	return n;
}

void update() {
	for (int x = 0; x < MAXROWS; x++) {
		for (int y = 0; y < MAXROWS; y++) {

			values[x][y] = valuesToBe[x][y];
		}
	}
}

void clean_valuesToBe() {
	for (int x = 0; x < MAXROWS; x++) {
		for (int y = 0; y < MAXROWS; y++) {

			valuesToBe[x][y] = values[x][y];
		}
	}
}

void advance() {
	for (int x = 0; x < MAXROWS; x++) {
		for (int y = 0; y < MAXROWS; y++) {

			int n = getNumberOfNeighbours(x, y);

			if (n == 3) valuesToBe[x][y] = 255;

			if (values[x][y] == 255) {
				if (n < 2) valuesToBe[x][y] = 0;
				if (n > 3) valuesToBe[x][y] = 0;
			}

		}
	}
}

void reset_level() {
	for (int x = 0; x < MAXROWS; x++) {
		for (int y = 0; y < MAXROWS; y++) {
			valuesToBe[x][y] = 0;
		}
	}
	editing = 1;
}

int prev_mouse_x;
int prev_mouse_y;
int prev_x_pos;
int prev_y_pos;

void mouse_panning() {

	if (middle_mouse_down) {
		int _x = 0;
		int _y = 0;
		SDL_GetMouseState(&_x, &_y);
		x_pos = prev_x_pos + float(float(_x - prev_mouse_x) / float(zoom));
		y_pos = prev_y_pos + float(float(_y - prev_mouse_y) / float(zoom));
	}
	else {
		SDL_GetMouseState(&prev_mouse_x, &prev_mouse_y);
		prev_x_pos = x_pos;
		prev_y_pos = y_pos;
	}
}

bool alt = 0;

void input() {
	mouse_panning();
	const Uint8* keyboard_state_array = SDL_GetKeyboardState(NULL);
	SDL_Event e;
	while (SDL_PollEvent(&e)) {
		switch (e.type) {
		case SDL_QUIT:
			running = false;
			editing = false;
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (e.button.button == SDL_BUTTON_LEFT) {
				int _x = 0;
				int _y = 0;
				SDL_GetMouseState(&_x, &_y);
				if ((_x / zoom) - x_pos < MAXROWS && (_y / zoom) - y_pos < MAXROWS) {
					valuesToBe[(_x / zoom) - x_pos][(_y / zoom) - y_pos] = 255;
				}
			}
			else if (e.button.button == SDL_BUTTON_RIGHT) {
				int _x = 0;
				int _y = 0;
				SDL_GetMouseState(&_x, &_y);
				if ((_x / zoom) - x_pos < MAXROWS && (_y / zoom) - y_pos < MAXROWS) {
					valuesToBe[(_x / zoom) - x_pos][(_y / zoom) - y_pos] = 0;
				}
			}
			else if (e.button.button == SDL_BUTTON_MIDDLE) {
				middle_mouse_down = 1;
			}
			break;
		case SDL_MOUSEBUTTONUP:
			if (e.button.button == SDL_BUTTON_MIDDLE) {
				middle_mouse_down = 0;
			}
			break;
		case SDL_KEYDOWN:
			if (e.key.keysym.sym == SDLK_RALT) { alt = 1; }
			else if (e.key.keysym.sym == SDLK_LALT) { alt = 1; }
			else if (e.key.keysym.sym == SDLK_p) { editing = !editing; clean_valuesToBe(); }
			else if (e.key.keysym.sym == SDLK_r && alt == 1) { reset_level(); }
			else if (e.key.keysym.sym == SDLK_g) { show_grid = !show_grid; }
			else if (e.key.keysym.sym == SDLK_EQUALS) { if (zoom + 1 < 200) { zoom++; } }
			else if (e.key.keysym.sym == SDLK_MINUS) { if (zoom - 1 > 0) { zoom--; } }
			else if (e.key.keysym.sym == SDLK_UP) { y_pos += 1; }
			else if (e.key.keysym.sym == SDLK_DOWN) { y_pos -= 1; }
			else if (e.key.keysym.sym == SDLK_RIGHT) { x_pos -= 1; }
			else if (e.key.keysym.sym == SDLK_LEFT) { x_pos += 1; }
			else if (e.key.keysym.sym == SDLK_TAB) { show_UI = !show_UI; }

			break;
		case SDL_KEYUP:
			if (e.key.keysym.sym == SDLK_RALT) { alt = 0; }
			else if (e.key.keysym.sym == SDLK_LALT) { alt = 0; }
			break;
		case SDL_MOUSEWHEEL:
			if (e.wheel.y > 0)
			{
				if (zoom + 1 < 200) { zoom++; }
			}
			else if (e.wheel.y < 0)
			{
				if (zoom - 1 > 0) { zoom--; }
			}
			break;
		}

	}
	

}


void draw_text(string _text, int _x, int _y) {

	TTF_Font* font = TTF_OpenFont("Fonts/Asleepy.ttf", 30);

	SDL_Color textColor = { 255, 255, 255, 0 };
	SDL_Surface* textSurface = TTF_RenderText_Solid(font, _text.c_str(), textColor);
	SDL_Texture* text = SDL_CreateTextureFromSurface(renderer, textSurface);

	int text_width = textSurface->w;
	int text_height = textSurface->h;

	SDL_FreeSurface(textSurface);
	SDL_Rect renderQuad = { _x, _y, text_width, text_height };

	SDL_RenderCopy(renderer, text, NULL, &renderQuad);

	SDL_DestroyTexture(text);
	
}

void create_grid() {

	int dist = width / MAXROWS;

	int x = 0;
	int y = 0;


	int b = show_grid ? 255 : 0;
	for (int i = 0; i < (width / zoom) - x_pos; i++) {
		if (((x + ((x_pos - 1) * zoom)) / zoom) - x_pos < MAXROWS) {
			x += zoom;

			SDL_SetRenderDrawColor(renderer, b, b, b, 255);

			SDL_RenderDrawLine(renderer, x + ((x_pos - 1) * zoom), y_pos * zoom, x + ((x_pos - 1) * zoom), zoom * (MAXROWS + y_pos));
		}
	}

	
	for (int i = 0; i < (height / zoom) - y_pos; i++) {
		if (((y + ((y_pos - 1) * zoom)) / zoom) - y_pos < MAXROWS) {
			y += zoom;

			SDL_SetRenderDrawColor(renderer, b, b, b, 255);

			SDL_RenderDrawLine(renderer, x_pos * zoom, y + ((y_pos - 1) * zoom), zoom * (MAXROWS + x_pos), y + ((y_pos - 1) * zoom));
		}
	}

}

void draw() {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_Rect rect;
	rect.x = 0;
	rect.y = 0;
	rect.w = width;
	rect.h = height;
	SDL_RenderFillRect(renderer, &rect);


	int dist = width / MAXROWS;

	for (int x = 0; x < MAXROWS; x++) {
		for (int y = 0; y < MAXROWS; y++) {

			int b = values[x][y];

			if (b == 255 && x + x_pos >= 0 && y + y_pos >= 0) {
				SDL_SetRenderDrawColor(renderer, b, b, b, 255);
				rect.x = (x+x_pos) * zoom;
				rect.y = (y+y_pos) * zoom;
				rect.w = zoom;
				rect.h = zoom;
				SDL_RenderFillRect(renderer, &rect);
			}

		}
	}

	create_grid();
}


int main(int argc, char* argv[]) {

	editing = 1;
	running = 1;
	int lastFrameTime = 0;
	int timeSinceUpdate = 0;
	SDL_Init(SDL_INIT_EVENTS);
	SDL_CreateWindowAndRenderer(600, 1200, SDL_WINDOW_MAXIMIZED | SDL_WINDOW_RESIZABLE, &window, &renderer);
	SDL_SetWindowTitle(window, "Game Of Life");
	SDL_ShowCursor(1);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");

	SDL_GetWindowSize(window, &width, &height);

	TTF_Init();

	while (running) {

		input();

		if (!editing) {
			int deltaTime = (SDL_GetTicks() - lastFrameTime);
			lastFrameTime = SDL_GetTicks();
			timeSinceUpdate += deltaTime;

			if (timeSinceUpdate > 1000) {
				advance();
				timeSinceUpdate = 0;
			}
		}

		draw();

		update();

		if (show_UI) {
			draw_text("Left Click - place square", 10, 0);
			draw_text("Right Click - delete square", 10, 30);
			draw_text("G - toggle grid", 10, 60);
			draw_text("P - play/pause", 10, 90);
			draw_text("Mouse Scroll - zoom in/out", 10, 120);
			draw_text("Middle Mouse - pan", 10, 150);
			draw_text("alt + r - clear level", 10, 180);
		}

		draw_text("Tab - Show UI", 10, height - 50);

		SDL_RenderPresent(renderer);

	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();


	return 0;

}