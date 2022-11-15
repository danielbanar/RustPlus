#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
struct Texture
{
	SDL_Texture* texture;
	int w, h;
};