#ifndef INCLUDED_ISONOMALY_H
#define INCLUDED_ISONOMALY_H

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
	SDL_Rect mouse;
} Canvas;

int pallet_init(Pallet *pallet, SDL_Renderer *renderer, int x, int y, int w, int h);
void pallet_show(Pallet *pallet);
void gridtopixel(const Canvas *canvas, int i, int j, SDL_FRect *pixel);
void pixeltogrid(const Canvas *canvas, int x, int y, int *i, int *j);
int canvas_init(Canvas *canvas, SDL_Renderer *renderer, int x, int y, int w, int h, Pallet *pallet);
void canvas_show(Canvas *canvas);

#endif
