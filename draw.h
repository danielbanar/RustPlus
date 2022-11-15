#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <intrin.h>
#include <iostream>
namespace Draw
{
	inline void Pixel(Uint32* pix, int x, int y, int w, int h, const Uint32& col);
	void Pixel(SDL_Surface* surf, int x, int y, const SDL_Color& col);
	void Circle(SDL_Surface* surf, int x, int y, int r, const SDL_Color& col);
	void VLine(SDL_Surface* surf, int x, int y, int length, const SDL_Color& col);
	void HLine(SDL_Surface* surf, int x, int y, int length, const SDL_Color& col);
	void Line(SDL_Surface* surf, int x1, int y1, int x2, int y2, const SDL_Color& col);
	void Rect(SDL_Surface* surf, int size, const SDL_Color& col);
	void Checkmark(SDL_Surface* surf, int size, const SDL_Color& col);
}