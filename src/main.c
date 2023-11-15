#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdlib.h>
#include <time.h>
/*
	TODO: Fix shading on iso stair tile.
*/
typedef struct window {
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *pallet;
	int palletWidth, palletHeight;
	int palletRows, palletColumns;
	int palletEntryWidth, palletEntryHeight;
	int palletEntrySelected;
	int palletSize;
	int offsetX, offsetY;
	int mouseX, mouseY;
} Window;

#define SDL_FAILURE 1
#define IMG_FAILURE 2

#define GRID_COLUMNS 16
#define GRID_ROWS 16
#define GRID_LAYERS 16
#define GRID_SPACES (GRID_COLUMNS * GRID_ROWS * GRID_LAYERS)
char grid[GRID_SPACES];

int window_init(Window *window)
{
	int error;
	int flags;

	srand(time(NULL));

	error = SDL_Init(SDL_INIT_VIDEO);
	if (error) {
		fprintf(stderr, "%s\n", SDL_GetError());
		exit(SDL_FAILURE);
	}

	flags = IMG_INIT_PNG;
	if (flags != IMG_Init(flags)) {
		fprintf(stderr, "%s\n", IMG_GetError());
		exit(IMG_FAILURE);
	}

	window->window = SDL_CreateWindow("",
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			800,
			600,
			0);
	if (window->window == NULL) {
		fprintf(stderr, "%s\n", SDL_GetError());
		exit(SDL_FAILURE);
	}

	window->renderer = SDL_CreateRenderer(window->window,
			-1,
			SDL_RENDERER_PRESENTVSYNC |
			SDL_RENDERER_ACCELERATED);
	if (window->renderer == NULL) {
		fprintf(stderr, "%s\n", SDL_GetError());
		exit(SDL_FAILURE);
	}

	SDL_Surface *surface = IMG_Load("res/atlas.png");
	if (surface == NULL) {
		fprintf(stderr, "%s\n", IMG_GetError());
		exit(IMG_FAILURE);
	}
	window->pallet = SDL_CreateTextureFromSurface(window->renderer,
			surface);
	if (window->pallet == NULL) {
		fprintf(stderr, "%s\n", SDL_GetError());
		exit(IMG_FAILURE);	
	}
	SDL_QueryTexture(window->pallet,
		NULL,
		NULL,
		&window->palletWidth,
		&window->palletHeight);
	window->palletEntryWidth = 32;
	window->palletEntryHeight = 32;
	window->palletEntrySelected = 0;
	window->palletRows = window->palletHeight / window->palletEntryHeight;
	window->palletColumns = window->palletWidth / window->palletEntryWidth;
	window->palletSize = window->palletColumns * window->palletRows;
	window->offsetX = 0;
	window->offsetY = 0;
}

void gridtopixel(Window *window, int x, int y, int z, int *a, int *b)
{
	*a = (x-y) * window->palletEntryWidth / 2;
	*b = (x+y) * window->palletEntryHeight / 4;
}

void pixeltogrid(Window *window, int x, int y, int *a, int *b)
{
	*a = (2.0 * y) / window->palletEntryHeight
			+ (float)x / window->palletEntryWidth;

	*b = (2.0 * y) / window->palletEntryHeight
			- (float)x / window->palletEntryWidth;
}

void window_show(Window *window)
{
	SDL_Renderer *renderer = window->renderer;
	SDL_Rect dstrect;
	SDL_Rect srcrect;

	/* Clear Screen */
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	dstrect.x = window->mouseX;
	dstrect.y = window->mouseY;
	dstrect.w = window->palletEntryWidth;
	dstrect.h = window->palletEntryHeight;

	int selectedRow;
	int selectedColumn;
	selectedRow = window->palletEntrySelected / window->palletColumns;
	selectedColumn = window->palletEntrySelected % window->palletColumns;
	srcrect.x = selectedColumn * window->palletEntryWidth;
	srcrect.y = selectedRow * window->palletEntryHeight;
	srcrect.w = window->palletEntryWidth;
	srcrect.h = window->palletEntryHeight;

	/* Render Pallet Selection */
	// SDL_RenderCopy(renderer, window->pallet, &srcrect, &dstrect);

	float m = 1.0 / 2.0;
	int width;
	int height;
	SDL_GetWindowSize(window->window, &width, &height);

	/* Render Grid */
	SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
	int lines = height / (window->palletEntryHeight / 2);
	for (int i = -lines; i < lines; i++) {
		int x1, y1, x2, y2;
		x1 = 0;
		y1 = window->palletEntryHeight / 4
			+ i * window->palletEntryHeight / 2;
		x2 = width;
		y2 = window->palletEntryHeight / 4
			+ m * (i * window->palletEntryWidth + width);
		SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
	}

	SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
	for (int i = 0; i < 2 * lines; i++) {
		int x1, y1, x2, y2;
		x1 = 0;
		y1 = window->palletEntryHeight / 4
			+ i * window->palletEntryHeight / 2;
		x2 = width;
		y2 = window->palletEntryHeight / 4
			+ -m * (-i * window->palletEntryWidth + width);
		SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
	}

	/* Render Pallet */
	dstrect.x = 0;
	dstrect.y = 0;
	dstrect.w = window->palletWidth;
	dstrect.h = window->palletHeight;
	SDL_RenderCopy(renderer, window->pallet, NULL, &dstrect);

	for (int i = 0; i < GRID_COLUMNS; i++)
	for (int j = 0; j < GRID_ROWS; j++)
	for (int k = 0; k < GRID_LAYERS; k++) {
		if (grid[i + j * GRID_COLUMNS + k * GRID_COLUMNS * GRID_ROWS])
			continue;
		int x, y, z, a, b;
		x = i + window->offsetX;
		y = j + window->offsetY;
		z = k;
		gridtopixel(window, x, y, z, &a, &b);
		
		dstrect.x = a;
		dstrect.y = b;
		dstrect.w = window->palletEntryWidth;
		dstrect.h = window->palletEntryHeight;
		SDL_RenderCopy(renderer, window->pallet, &srcrect, &dstrect);
	}

	/* Swap Buffer */
	SDL_RenderPresent(renderer);
}

int main(int argc, char *argv[])
{
	Window window;
	window_init(&window);

	const Uint8 *keys = SDL_GetKeyboardState(NULL);

	for (int i = 0; i < GRID_SPACES; i++)
		grid[i] = 1;

	Uint64 start, end, ticks;
	while (1) {
		SDL_Event event;
		while (SDL_PollEvent(&event) != 0) {
			if (event.type == SDL_QUIT)
				goto quit;
		}
		end = SDL_GetTicks64();
		ticks += end - start;
		start = end;

		int update;
		int keydown;

		if (ticks > 256)
			update = 1;

		if (update || !keydown) {
			if (keys[SDL_SCANCODE_UP]) {
				window.offsetX--;
				window.offsetY--;
				keydown = 1;
			} else if (keys[SDL_SCANCODE_DOWN]) {
				window.offsetY++;
				window.offsetX++;
				keydown = 1;
			} else if (keys[SDL_SCANCODE_LEFT]) {
				window.offsetX--;
				keydown = 1;
			} else if (keys[SDL_SCANCODE_RIGHT]) {
				window.offsetX++;
				keydown = 1;
			} else {
				keydown = 0;
			}
			update = 0;
			ticks = 0;
		}
		Uint32 mouse = SDL_GetMouseState(&window.mouseX, &window.mouseY);

		if (mouse & SDL_BUTTON(1)) {
			int x, y;
			pixeltogrid(&window, window.mouseX, window.mouseY, &x, &y);
			grid[x + y * GRID_COLUMNS] = 0;
		}

		window_show(&window);
	}
quit:
	return 0;
}
