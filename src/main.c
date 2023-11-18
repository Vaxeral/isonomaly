#include "isonomaly.h"

SDL_Window *window;
SDL_Renderer *renderer;

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 512

int main(int argc, char *argv[])
{
	int error;
	int flags;

	error = SDL_Init(SDL_INIT_VIDEO);
	if (error) {
		fprintf(stderr, "%s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	flags = IMG_INIT_PNG;
	if (flags != IMG_Init(flags)) {
		fprintf(stderr, "%s\n", IMG_GetError());
		exit(EXIT_FAILURE);
	}

	window = SDL_CreateWindow("isonomaly",
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			WINDOW_WIDTH,
			WINDOW_HEIGHT,
			0);
	if (window == NULL) {
		fprintf(stderr, "%s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	renderer = SDL_CreateRenderer(window,
			-1,
			SDL_RENDERER_PRESENTVSYNC |
			SDL_RENDERER_TARGETTEXTURE |
			SDL_RENDERER_ACCELERATED);
	if (renderer == NULL) {
		fprintf(stderr, "%s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	Pallet pallet;
	Canvas canvas;
	pallet_init(&pallet, renderer,
			0, 0,
			WINDOW_WIDTH / 2, WINDOW_HEIGHT);
	canvas_init(&canvas, renderer,
			WINDOW_WIDTH / 2, 0,
			WINDOW_WIDTH / 2, WINDOW_HEIGHT,
			&pallet);

	const Uint8 *keys =
		SDL_GetKeyboardState(NULL);

	int dx, dy;
	int startX, startY;
	int mouseX, mouseY;

	Uint64 ticks, start, end;
	bool doUpdate;
	bool buttonDown;

	buttonDown = false;

	while (1) {
		SDL_Event event;
		while (SDL_PollEvent(&event) != 0) {
			if (event.type == SDL_QUIT)
				goto quit;
			if (event.type == SDL_KEYDOWN)
				if (event.key.keysym.sym == SDLK_r)
					memset(canvas.grid, 0, sizeof(canvas.grid));
			if (event.type == SDL_MOUSEMOTION) {
				if (buttonDown) {
					dx = event.motion.x - mouseX,
					dy = event.motion.y - mouseY;
					canvas.view.x = startX + dx;
					canvas.view.y = startY + dy;
				}
			} else if (event.type == SDL_MOUSEBUTTONDOWN) {
				if (event.button.button == SDL_BUTTON_RIGHT) {
					startX = canvas.view.x;
					startY = canvas.view.y;
					mouseX = event.button.x;
					mouseY = event.button.y;
					buttonDown = true;
				}
			} else if (event.type == SDL_MOUSEBUTTONUP) {
				if (event.button.button == SDL_BUTTON_RIGHT) {
					buttonDown = false;
				}
			}
		}

		end = SDL_GetTicks64();
		ticks += end - start;
		start = end;


		SDL_Rect mouse;
		const Uint32 button = SDL_GetMouseState(&mouse.x, &mouse.y);
		canvas.mouse.x = mouse.x - canvas.port.x;
		canvas.mouse.y = mouse.y - canvas.port.y;


		if (button & SDL_BUTTON(1)) {
			if (mouse.x >= canvas.port.x &&
					mouse.x < canvas.port.x + canvas.port.w &&
					mouse.y >= canvas.port.y &&
					mouse.y < canvas.port.y + canvas.port.h) {
				int x, y, i, j;
				x = mouse.x - canvas.port.x;
				y = mouse.y - canvas.port.y;
				pixeltogrid(&canvas, x, y, &i, &j);
				if (i >= 0 && i < GRID_COLUMNS && j >= 0 && j < GRID_ROWS)
					canvas.grid[i + j * GRID_COLUMNS] = canvas.tile;
			} else {
				int i, j;
				i = mouse.x / canvas.tile.w;
				j = mouse.y / canvas.tile.h;
				canvas.tile.x = i * canvas.tile.w;
				canvas.tile.y = j * canvas.tile.h;
			}
		}

		if (ticks > 1.0 / 32.0 * 1000.0)
			doUpdate = true;

		if (doUpdate) {
			if (keys[SDL_SCANCODE_DOWN])
				canvas.view.y -= canvas.step.y;
			if (keys[SDL_SCANCODE_UP])
				canvas.view.y += canvas.step.y;
			if (keys[SDL_SCANCODE_RIGHT])
				canvas.view.x -= canvas.step.x;
			if (keys[SDL_SCANCODE_LEFT])
				canvas.view.x += canvas.step.x;
			doUpdate = false;
			ticks = 0;
		}

		pallet_show(&pallet);
		canvas_show(&canvas);
		SDL_RenderPresent(renderer);
	}
quit:
	return 0;
}
