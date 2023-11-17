#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>

#define GRID_COLUMNS 64
#define GRID_ROWS 64
#define GRID_CELLS (GRID_COLUMNS * GRID_ROWS)

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
	SDL_Rect tile;
	SDL_Rect grid[GRID_CELLS];
} Canvas;

SDL_Window *window;
SDL_Renderer *renderer;

void gridtopixel(const Canvas *canvas, int i, int j, SDL_FRect *pixel)
{
	Pallet *pallet = canvas->pallet;
	pixel->x = (i-j) * pallet->cell.w / 2.0 + canvas->view.x;
	pixel->y = (i+j) * pallet->cell.h / 2.0 + canvas->view.y;
}

void pixeltogrid(const Canvas *canvas, int x, int y, int *i, int *j)
{
	Pallet *pallet = canvas->pallet;
	x = x - canvas->view.x - pallet->cell.w / 2.0;
	y = y - canvas->view.y;
	*i = (float)y / pallet->cell.h + (float)x / pallet->cell.w;
	*j = (float)y / pallet->cell.h - (float)x / pallet->cell.w;
}

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 512

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
	surface = IMG_Load("res/spritesheet.png");
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
	memset(canvas, 0, sizeof(*canvas));

	canvas->pallet = pallet;
	canvas->step.y = 4.0;
	canvas->step.x = 4.0;

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

	canvas->tile.x = 0;
	canvas->tile.y = 0;
	canvas->tile.w = 32;
	canvas->tile.h = 32;

	for (int i = 0; i < GRID_CELLS; i++)
		canvas->grid[i] = (SDL_Rect){0};

	return 0;
}

void canvas_show(Canvas *canvas)
{
	const Pallet *pallet = canvas->pallet;
	const float m = pallet->cell.w / pallet->cell.h;

	const int nLines =
		canvas->port.w / pallet->cell.w +
		canvas->port.h / pallet->cell.h + 16;

	SDL_SetRenderTarget(canvas->renderer, canvas->target);

	/* Clear Canvas */
	SDL_SetRenderDrawColor(canvas->renderer, 85, 107, 47, 255);
	SDL_RenderClear(canvas->renderer);

	/* Update Canvas */
	SDL_SetRenderDrawColor(canvas->renderer, 150, 150, 150, 255);

	int x1, y1, x2, y2;
	int dx, dy;
	int width, height;

	width = canvas->port.w + 2 * pallet->cell.w;
	height = canvas->port.h;

	dx = canvas->view.x % pallet->cell.w;
	dy = canvas->view.y % pallet->cell.h;

	x2 = -pallet->cell.w + dx;
	y2 = -pallet->cell.h + dy + pallet->cell.h / 2.0 + height;
	x1 = -pallet->cell.w + dx + width;
	y1 = -pallet->cell.h + dy + pallet->cell.h / 2.0 + height
			+ width * (1 / m);

	for (int i = 0; i < nLines; i++) {
		y1 -= pallet->cell.h;
		y2 -= pallet->cell.h;
		SDL_RenderDrawLine(canvas->renderer, x1, y1, x2, y2);
	}

	x2 = -pallet->cell.w + dx;
	y2 = -pallet->cell.h + dy + pallet->cell.h / 2.0;
	x1 = -pallet->cell.w + dx + width;
	y1 = -pallet->cell.h + dy + pallet->cell.h / 2.0
			+ width * -(1 / m);

	for (int i = 0; i < nLines; i++) {
		y1 += pallet->cell.h;
		y2 += pallet->cell.h;
		SDL_RenderDrawLine(canvas->renderer, x1, y1, x2, y2);
	}

	for (int i = 0; i < GRID_COLUMNS; i++)
	for (int j = 0; j < GRID_ROWS; j++) {
		SDL_FRect pixel;

		if (canvas->grid[i + j * GRID_COLUMNS].w == 0)
			continue;

		gridtopixel(canvas, i, j, &pixel);

		SDL_FRect dstrect;
		SDL_Rect srcrect;

		srcrect = canvas->grid[i + j * GRID_COLUMNS];

		dstrect.x = pixel.x;
		dstrect.y = pixel.y - (canvas->tile.h - pallet->cell.h);
		dstrect.w = canvas->tile.w;
		dstrect.h = canvas->tile.h;
		SDL_RenderCopyF(canvas->renderer, pallet->atlas, &srcrect, &dstrect);
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
