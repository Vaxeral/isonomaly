#include "isonomaly.h"

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
