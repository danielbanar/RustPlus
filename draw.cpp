#include "draw.h"
namespace Draw
{
	inline void Pixel(Uint32* pix, int x, int y, int w, int h, const Uint32& col)
	{
		if (x < 0 || y < 0 || x >= w || y >= h)
		{
#ifdef DEBUG
			std::cout << "[Warning] Drawing out of screen space: x=" << x << ", y=" << y << std::endl;
#endif
			return;
		}
		pix[(y * w) + x] = col;
	}
	void Pixel(SDL_Surface* surf, int x, int y, const SDL_Color& col)
	{
		if (x < 0 || y < 0 || x >= surf->w || y >= surf->h)
		{
#ifdef DEBUG
			std::cout << "[Warning] Drawing out of screen space: x=" << x << ", y=" << y << std::endl;
#endif
			return;
		}
		((Uint32*)surf->pixels)[(y * surf->w) + x] = *(Uint32*)&col;
	}

	void Circle(SDL_Surface* surf, int x, int y, int r, const SDL_Color& col)
	{
		float step = 1.f / r;
		int x2, y2;
		for (float angle = 0.0f; angle <= 2.f * M_PI; angle += step)
		{
			x2 = x + std::round(sinf(angle) * r);
			y2 = y + std::round(cosf(angle) * r);
			Pixel((Uint32*)surf->pixels, x2, y2, surf->w, surf->h, *(Uint32*)&col);
		}
	}
	void VLine(SDL_Surface* surf, int x, int y, int length, const SDL_Color& col)
	{
		for (size_t i = 0; i < length; i++)
			Pixel((Uint32*)surf->pixels, x, y + i, surf->w, surf->h, *(Uint32*)&col);
	}
	void HLine(SDL_Surface* surf, int x, int y, int length, const SDL_Color& col)
	{
		for (size_t i = 0; i < length; i++)
			Pixel((Uint32*)surf->pixels, x + i, y, surf->w, surf->h, *(Uint32*)&col);
	}
	void Line(SDL_Surface* surf, int x1, int y1, int x2, int y2, const SDL_Color& col)
	{
		Uint32* pixels = (Uint32*)surf->pixels;
		Uint32 color = *(Uint32*)&col;
		int dx = x2 - x1;
		int dy = y2 - y1;
		if (dx == 0)
		{
			VLine(surf, x1, y1 < y2 ? y1 : y2, abs(dy), col);
			return;
		}
		if (dy == 0)
		{
			HLine(surf, x1 < x2 ? x1 : x2, y1, abs(dx), col);
			return;
		}
		if (abs(dx) > abs(dy))
		{
			for (int x = 0; x <= abs(dx); x++)
			{
				float ratio = (float)(x) / (float)(abs(dx));
				int y = y1 + std::roundf(dy * ratio);
				Pixel(pixels, dx < 0 ? x1 - x : x1 + x, y, surf->w, surf->h, color);
			}
		}
		else
		{
			for (int y = 0; y <= abs(dy); y++)
			{
				float ratio = (float)(y) / (float)(abs(dy));
				int x = x1 + std::roundf(dx * ratio);
				Pixel(pixels, x, dy < 0 ? y1 - y : y1 + y, surf->w, surf->h, color);
			}
		}
	}
	void Rect(SDL_Surface* surf, int size, const SDL_Color& col)
	{
		HLine(surf, 0, 0, size - 1, col);
		HLine(surf, 0, size - 1, size, col);
		VLine(surf, 0, 0, size - 1, col);
		VLine(surf, size - 1, 0, size, col);
	}
	void Checkmark(SDL_Surface* surf, int size, const SDL_Color& col)
	{
		for (Uint32 i = 0; i < size - 6; i++)
		{
			HLine(surf, 3, 3 + i, size - 6, col);
		}
		//Line(surf, 0, 0, size-1, size-1, col);
		//Line(surf, size-1, 0, 0, size-1, col);
	}
}