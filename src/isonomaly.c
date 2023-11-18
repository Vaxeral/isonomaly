#include "isonomaly.h"

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
