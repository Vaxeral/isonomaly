#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>

/*
	TODO: move the grid.
*/

typedef struct pallet {
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *target;
	SDL_Rect port;
	SDL_Rect view;
	SDL_Rect cell;
	SDL_Texture *atlas;
	SDL_Rect *tiles;
	size_t nTiles;
} Pallet;

typedef struct canvas {
	SDL_Window *window;
	SDL_Texture *target;
	SDL_Renderer *renderer;
	Pallet *pallet;
	SDL_Rect port;
	SDL_Rect view;
	SDL_Rect step;

} Canvas;

SDL_Window *window;
SDL_Renderer *renderer;

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

int pallet_init(Pallet *pallet, SDL_Renderer *renderer, int x, int y, int w, int h)
{
	Uint32 format;
	SDL_Surface *surface;

	pallet->cell.w = 32;
	pallet->cell.h = 16;

	pallet->view.x = 0;
	pallet->view.y = 0;
	
	pallet->port.x = x;
	pallet->port.y = y;
	pallet->port.w = w;
	pallet->port.h = h;
	
	pallet->renderer = renderer;
	surface = IMG_Load("res/atlas.png");
	if (surface == NULL) {
		fprintf(stderr, "%s\n", IMG_GetError());
		goto failure;
	}
	pallet->atlas = SDL_CreateTextureFromSurface(pallet->renderer, surface);
	if (pallet->atlas == NULL) {
		fprintf(stderr, "%s\n", SDL_GetError());
		goto failure;
	}

	SDL_QueryTexture(pallet->atlas, &format, NULL, NULL, NULL);

	pallet->target = SDL_CreateTexture(pallet->renderer,
			format,
			SDL_TEXTUREACCESS_TARGET,
			pallet->port.w,
			pallet->port.h);
	pallet->window = SDL_RenderGetWindow(pallet->renderer);

	return 0;
failure:
	SDL_DestroyTexture(pallet->target);
	return 1;
}

void pallet_show(Pallet *pallet)
{
	SDL_SetRenderTarget(pallet->renderer, pallet->target);

	/* Clear Canvas */
	SDL_SetRenderDrawColor(pallet->renderer, 100, 100, 100, 255);
	SDL_RenderClear(pallet->renderer);

	/* Update Canvas */
	SDL_Rect rect;
	rect.x = 0;
	rect.y = 0;
	SDL_QueryTexture(pallet->atlas, NULL, NULL, &rect.w, &rect.h);
	SDL_RenderCopy(pallet->renderer, pallet->atlas, NULL, &rect);

	SDL_SetRenderTarget(pallet->renderer, NULL);
	SDL_RenderCopy(pallet->renderer, pallet->target, NULL, &pallet->port);
}

int canvas_init(Canvas *canvas,
	SDL_Renderer *renderer,
	int x, int y, int w, int h,
	Pallet *pallet)
{
	canvas->pallet = pallet;
	canvas->step.y = pallet->cell.h / 2.0;
	canvas->step.x = pallet->cell.w / 2.0;

	canvas->view.x = 0;
	canvas->view.y = 0;

	canvas->port.x = x;
	canvas->port.y = y;
	canvas->port.w = w;
	canvas->port.h = h;

	canvas->renderer = renderer;
	canvas->target = SDL_CreateTexture(canvas->renderer,
			SDL_PIXELFORMAT_ABGR8888,
			SDL_TEXTUREACCESS_TARGET,
			canvas->port.w,
			canvas->port.h);
	canvas->window = SDL_RenderGetWindow(canvas->renderer);
	return 0;
}

void canvas_show(Canvas *canvas)
{
	const Pallet *pallet = canvas->pallet;
	const float m = pallet->cell.w / pallet->cell.h;

	const int nLines =
		canvas->port.w / pallet->cell.w +
		canvas->port.h / pallet->cell.h;

	SDL_SetRenderTarget(canvas->renderer, canvas->target);

	/* Clear Canvas */
	SDL_SetRenderDrawColor(canvas->renderer, 85, 107, 47, 255);
	SDL_RenderClear(canvas->renderer);

	/* Update Canvas */
	SDL_SetRenderDrawColor(canvas->renderer, 200, 200, 200, 255);

	int x1, y1, x2, y2;
	x1 = 0;
	y1 = canvas->port.h;
	x2 = canvas->port.w;
	y2 = canvas->port.w * (1 / m) + y1;

	for (int i = 0; i < nLines; i++) {
		y1 -= pallet->cell.h;
		y2 -= pallet->cell.h;
		SDL_RenderDrawLine(canvas->renderer, x1, y1, x2, y2);
	}

	x1 = 0;
	y1 = 0;
	x2 = canvas->port.w;
	y2 = canvas->port.w * -(1 / m) + y1;

	for (int i = 0; i < nLines; i++) {
		y1 += pallet->cell.h;
		y2 += pallet->cell.h;
		SDL_RenderDrawLine(canvas->renderer, x1, y1, x2, y2);
	}

	SDL_SetRenderTarget(canvas->renderer, NULL);
	SDL_RenderCopy(canvas->renderer, canvas->target, NULL, &canvas->port);
}

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

	window = SDL_CreateWindow("isonomly",
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


	SDL_Surface *surface;
	surface = IMG_Load("res/atlas.png");
	if (surface == NULL) {
		fprintf(stderr, "%s\n", IMG_GetError());
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

	Uint64 ticks, start, end;
	bool doUpdate;

	while (1) {
		SDL_Event event;
		while (SDL_PollEvent(&event) != 0) {
			if (event.type == SDL_QUIT)
				goto quit;
		}

		end = SDL_GetTicks64();
		ticks += end - start;
		start = end;

		if (ticks > 1.0 / 8.0 * 1000.0)
			doUpdate = true;

		if (doUpdate) {
			if (keys[SDL_SCANCODE_UP])
				canvas.view.y -= canvas.step.y;
			if (keys[SDL_SCANCODE_DOWN])
				canvas.view.y += canvas.step.y;
			if (keys[SDL_SCANCODE_LEFT])
				canvas.view.x -= canvas.step.x;
			if (keys[SDL_SCANCODE_RIGHT])
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
