#include "isonomaly.h"

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
	const float m = (float)pallet->cell.h / pallet->cell.w;

	const int nLines =
		canvas->port.w / pallet->cell.w +
		canvas->port.h / pallet->cell.h + 32;

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
	height = canvas->port.h + 2 * pallet->cell.h;

	dx = canvas->view.x % pallet->cell.w;
	dy = canvas->view.y % pallet->cell.h;

	/* Lines Up */
	x2 = -pallet->cell.w + dx;
	y2 = -pallet->cell.h + dy + height;
	x1 = -pallet->cell.w + dx + width;
	y1 = -pallet->cell.h + dy + height
			+ width * m;

	for (int i = 0; i < nLines; i++) {
		y1 -= pallet->cell.h;
		y2 -= pallet->cell.h;
		SDL_RenderDrawLine(canvas->renderer, x1, y1, x2, y2);
	}

	/* Lines Down */
	x2 = -pallet->cell.w + dx;
	y2 = -pallet->cell.h + dy;
	x1 = -pallet->cell.w + dx + width;
	y1 = -pallet->cell.h + dy
			+ width * -m;

	for (int i = 0; i < nLines; i++) {
		y1 += pallet->cell.h;
		y2 += pallet->cell.h;
		SDL_RenderDrawLine(canvas->renderer, x1, y1, x2, y2);
	}

	/* Tiles */
	SDL_FRect dstrect;
	SDL_Rect srcrect;
	for (int i = 0; i < GRID_COLUMNS; i++)
	for (int j = 0; j < GRID_ROWS; j++) {
		SDL_FRect pixel;

		if (canvas->grid[i + j * GRID_COLUMNS].w == 0)
			continue;

		gridtopixel(canvas, i, j, &pixel);


		srcrect = canvas->grid[i + j * GRID_COLUMNS];

		dstrect.x = pixel.x;
		dstrect.y = pixel.y - (canvas->tile.h - pallet->cell.h);
		dstrect.w = canvas->tile.w;
		dstrect.h = canvas->tile.h;
		SDL_RenderCopyF(canvas->renderer, pallet->atlas, &srcrect, &dstrect);
	}

	/* Selected Tile */
	int x, y, i, j;
	SDL_FRect pixel;
	x = canvas->mouse.x;
	y = canvas->mouse.y;

	pixeltogrid(canvas, x, y, &i, &j);
	gridtopixel(canvas, i, j, &pixel);

	srcrect = canvas->tile;

	dstrect.x = pixel.x;
	dstrect.y = pixel.y - (canvas->tile.h - pallet->cell.h);
	dstrect.w = canvas->tile.w;
	dstrect.h = canvas->tile.h;

	SDL_SetTextureAlphaMod(pallet->atlas, 1.0 / 2.0 * 255.0);
	SDL_SetTextureBlendMode(pallet->atlas, SDL_BLENDMODE_BLEND);
	SDL_RenderCopyF(canvas->renderer, pallet->atlas, &srcrect, &dstrect);
	SDL_SetTextureAlphaMod(pallet->atlas, 255.0);


	SDL_SetRenderTarget(canvas->renderer, NULL);
	SDL_RenderCopy(canvas->renderer, canvas->target, NULL, &canvas->port);
}
