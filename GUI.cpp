#pragma once
#include "GUI.h"

Button::Button(std::string _key, std::string _text, TTF_Font* font, SDL_Rect _rect, Relativity _rel, SDL_Color textColor, SDL_Color backgroundColor, bool defaultEnabled)
{
	key = _key;
	enabled = defaultEnabled;
	clickable = true;
	switch (_rel)
	{
	case TopLeft:
		rect = _rect;
		break;
	case Top:
		rect = { GUI::Get().windowRect.w / 2 + _rect.x - _rect.w / 2, _rect.y, _rect.w, _rect.h };
		break;
	case TopRight:
		rect = { GUI::Get().windowRect.w + _rect.x, _rect.y, _rect.w, _rect.h };
		break;
	case Left:
		rect = { _rect.x, GUI::Get().windowRect.h / 2 + _rect.y, _rect.w, _rect.h };
		break;
	case Center:
		rect = { GUI::Get().windowRect.w / 2 + _rect.x, GUI::Get().windowRect.h / 2 + _rect.y, _rect.w, _rect.h };
		break;
	case Right:
		rect = { GUI::Get().windowRect.w + _rect.x, GUI::Get().windowRect.h / 2 + _rect.y, _rect.w, _rect.h };
		break;
	case BottomLeft:
		rect = { _rect.x, GUI::Get().windowRect.h + _rect.y, _rect.w, _rect.h };
		break;
	case Bottom:
		rect = { GUI::Get().windowRect.w / 2 + _rect.x, GUI::Get().windowRect.h + _rect.y, _rect.w, _rect.h };
		break;
	case BottomRight:
		rect = { GUI::Get().windowRect.w + _rect.x - _rect.w, GUI::Get().windowRect.h - _rect.h + rect.y + _rect.y, _rect.w, _rect.h };
		break;
	}
	rel = _rel;
	text = _text;
	SDL_Surface* surfText = TTF_RenderText_Blended(font, text.c_str(), textColor);
	SDL_Surface* surfBackground = SDL_CreateRGBSurface(SDL_SWSURFACE, rect.w, rect.h, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
	SDL_FillRect(surfBackground, 0, _byteswap_ulong(*reinterpret_cast<Uint32*>(&backgroundColor)));
	SDL_Rect rc{ rect.w / 2 - surfText->w / 2,rect.h / 2 - surfText->h / 2,0,0 };
	SDL_BlitSurface(surfText, 0, surfBackground, &rc);

	texture = SDL_CreateTextureFromSurface(g.pRenderer, surfBackground);
	SDL_FreeSurface(surfText);
	SDL_FreeSurface(surfBackground);
	GUI::Add(this);
}
Button::~Button()
{
	SDL_DestroyTexture(texture);
	std::cout << "removing" << this->key;
}

Checkbox::Checkbox(std::string _key, std::string _text, TTF_Font* font, SDL_Rect _rect, Relativity _rel, SDL_Color textColor, SDL_Color backgroundColor, bool defaultState, bool defaultEnabled)
{
	key = _key;
	clickable = true;
	enabled = defaultEnabled;
	checked = defaultState;
	switch (_rel)
	{
	case TopLeft:
		rect = _rect;
		break;
	case Top:
		rect = { GUI::Get().windowRect.w / 2 + _rect.x, _rect.y, _rect.w, _rect.h };
		break;
	case TopRight:
		rect = { GUI::Get().windowRect.w + _rect.x, _rect.y, _rect.w, _rect.h };
		break;
	case Left:
		rect = { _rect.x, GUI::Get().windowRect.h / 2 + _rect.y, _rect.w, _rect.h };
		break;
	case Center:
		rect = { GUI::Get().windowRect.w / 2 + _rect.x, GUI::Get().windowRect.h / 2 + _rect.y, _rect.w, _rect.h };
		break;
	case Right:
		rect = { GUI::Get().windowRect.w + _rect.x, GUI::Get().windowRect.h / 2 + _rect.y, _rect.w, _rect.h };
		break;
	case BottomLeft:
		rect = { _rect.x, GUI::Get().windowRect.h + _rect.y, _rect.w, _rect.h };
		break;
	case Bottom:
		rect = { GUI::Get().windowRect.w / 2 + _rect.x, GUI::Get().windowRect.h + _rect.y, _rect.w, _rect.h };
		break;
	case BottomRight:
		rect = { GUI::Get().windowRect.w + _rect.x, GUI::Get().windowRect.h + _rect.y, _rect.w, _rect.h };
		break;
	}
	rel = _rel;
	text = _text;
	SDL_Surface* surfText = TTF_RenderText_Blended(font, text.c_str(), textColor);
	SDL_Surface* surfBackground = SDL_CreateRGBSurface(SDL_SWSURFACE, rect.w, rect.h, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
	SDL_FillRect(surfBackground, 0, 0);
	auto bgColor = _byteswap_ulong(*reinterpret_cast<unsigned long*>(&backgroundColor));
	Draw::Rect(surfBackground, rect.h, *reinterpret_cast<SDL_Color*>(&bgColor));
	SDL_Rect rc{ rect.h + 5,0,rect.w,rect.h };
	SDL_BlitSurface(surfText, 0, surfBackground, &rc);
	textureUnchecked = SDL_CreateTextureFromSurface(g.pRenderer, surfBackground);
	Draw::Checkmark(surfBackground, rect.h, *reinterpret_cast<SDL_Color*>(&bgColor));
	textureChecked = SDL_CreateTextureFromSurface(g.pRenderer, surfBackground);
	SDL_FreeSurface(surfText);
	SDL_FreeSurface(surfBackground);
	GUI::Add(this);
}
Checkbox::~Checkbox()
{
	SDL_DestroyTexture(textureChecked);
	SDL_DestroyTexture(textureUnchecked);
}

Slider::Slider(std::string _key, std::string _text, TTF_Font* font, SDL_Rect _rect, Relativity _rel, SDL_Color textColor, SDL_Color backgroundColor, int minValue, int maxValue, int defaultValue, bool defaultEnabled)
{
	key = _key;
	clickable = true;
	enabled = defaultEnabled;
	switch (_rel)
	{
	case TopLeft:
		rect = _rect;
		break;
	case Top:
		rect = { GUI::Get().windowRect.w / 2 + _rect.x, _rect.y, _rect.w, _rect.h };
		break;
	case TopRight:
		rect = { GUI::Get().windowRect.w + _rect.x, _rect.y, _rect.w, _rect.h };
		break;
	case Left:
		rect = { _rect.x, GUI::Get().windowRect.h / 2 + _rect.y, _rect.w, _rect.h };
		break;
	case Center:
		rect = { GUI::Get().windowRect.w / 2 + _rect.x, GUI::Get().windowRect.h / 2 + _rect.y, _rect.w, _rect.h };
		break;
	case Right:
		rect = { GUI::Get().windowRect.w + _rect.x, GUI::Get().windowRect.h / 2 + _rect.y, _rect.w, _rect.h };
		break;
	case BottomLeft:
		rect = { _rect.x, GUI::Get().windowRect.h + _rect.y, _rect.w, _rect.h };
		break;
	case Bottom:
		rect = { GUI::Get().windowRect.w / 2 + _rect.x, GUI::Get().windowRect.h + _rect.y, _rect.w, _rect.h };
		break;
	case BottomRight:
		rect = { GUI::Get().windowRect.w + _rect.x, GUI::Get().windowRect.h + _rect.y, _rect.w, _rect.h };
		break;
	}
	rel = _rel;
	text = _text;
	SDL_Surface* surfBackground = SDL_CreateRGBSurface(SDL_SWSURFACE, rect.w, rect.h, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
	SDL_FillRect(surfBackground, 0, 0);
	auto bgColor = _byteswap_ulong(*reinterpret_cast<unsigned long*>(&backgroundColor));
	Draw::Rect(surfBackground, rect.h, *reinterpret_cast<SDL_Color*>(&bgColor));
	SDL_Rect rc{ rect.h + 5,0,rect.w,rect.h };
	//SDL_BlitSurface(surfText, 0, surfBackground, &rc);
	//textureUnchecked = SDL_CreateTextureFromSurface(gVars.pRenderer, surfBackground);
	//SDL_FreeSurface(surfText);
	SDL_FreeSurface(surfBackground);
	GUI::Add(this);
}

Block::Block(std::string _key, SDL_Rect _rect, Relativity _rel, SDL_Color backgroundColor, bool defaultEnabled)
{
	key = _key;
	clickable = false;
	enabled = defaultEnabled;
	switch (_rel)
	{
	case TopLeft:
		rect = _rect;
		break;
	case Top:
		rect = { GUI::Get().windowRect.w / 2 + _rect.x, _rect.y, _rect.w, _rect.h };
		break;
	case TopRight:
		rect = { GUI::Get().windowRect.w + _rect.x, _rect.y, _rect.w, _rect.h };
		break;
	case Left:
		rect = { _rect.x, GUI::Get().windowRect.h / 2 + _rect.y, _rect.w, _rect.h };
		break;
	case Center:
		rect = { GUI::Get().windowRect.w / 2 + _rect.x, GUI::Get().windowRect.h / 2 + _rect.y, _rect.w, _rect.h };
		break;
	case Right:
		rect = { GUI::Get().windowRect.w + _rect.x, GUI::Get().windowRect.h / 2 + _rect.y, _rect.w, _rect.h };
		break;
	case BottomLeft:
		rect = { _rect.x, GUI::Get().windowRect.h + _rect.y, _rect.w, _rect.h };
		break;
	case Bottom:
		rect = { GUI::Get().windowRect.w / 2 + _rect.x, GUI::Get().windowRect.h + _rect.y, _rect.w, _rect.h };
		break;
	case BottomRight:
		rect = { GUI::Get().windowRect.w + _rect.x, GUI::Get().windowRect.h + _rect.y, _rect.w, _rect.h };
		break;
	}
	rel = _rel;
	SDL_Surface* surfBackground = SDL_CreateRGBSurface(SDL_SWSURFACE, rect.w, rect.h, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
	SDL_FillRect(surfBackground, 0, _byteswap_ulong(*reinterpret_cast<Uint32*>(&backgroundColor)));
	texture = SDL_CreateTextureFromSurface(g.pRenderer, surfBackground);
	SDL_FreeSurface(surfBackground);
	GUI::Add(this);
}
Block::~Block()
{
	SDL_DestroyTexture(texture);
}

Label::Label(std::string _key, std::string _text, TTF_Font* font, SDL_Rect _rect, Relativity _rel, SDL_Color textColor, bool defaultEnabled)
{
	key = _key;
	enabled = defaultEnabled;
	clickable = false;
	switch (_rel)
	{
	case TopLeft:
		rect = _rect;
		break;
	case Top:
		rect = { GUI::Get().windowRect.w / 2 + _rect.x - _rect.w / 2, _rect.y, _rect.w, _rect.h };
		break;
	case TopRight:
		rect = { GUI::Get().windowRect.w + _rect.x, _rect.y, _rect.w, _rect.h };
		break;
	case Left:
		rect = { _rect.x, GUI::Get().windowRect.h / 2 + _rect.y, _rect.w, _rect.h };
		break;
	case Center:
		rect = { GUI::Get().windowRect.w / 2 + _rect.x, GUI::Get().windowRect.h / 2 + _rect.y, _rect.w, _rect.h };
		break;
	case Right:
		rect = { GUI::Get().windowRect.w + _rect.x, GUI::Get().windowRect.h / 2 + _rect.y, _rect.w, _rect.h };
		break;
	case BottomLeft:
		rect = { _rect.x, GUI::Get().windowRect.h + _rect.y, _rect.w, _rect.h };
		break;
	case Bottom:
		rect = { GUI::Get().windowRect.w / 2 + _rect.x, GUI::Get().windowRect.h + _rect.y, _rect.w, _rect.h };
		break;
	case BottomRight:
		rect = { GUI::Get().windowRect.w + _rect.x - _rect.w, GUI::Get().windowRect.h - _rect.h + rect.y + _rect.y, _rect.w, _rect.h };
		break;
	}
	rel = _rel;
	text = _text;
	SDL_Surface* surfText = TTF_RenderText_Blended_Wrapped(font, text.c_str(), textColor, rect.w);
	SDL_Surface* surfBackground = SDL_CreateRGBSurface(SDL_SWSURFACE, rect.w, rect.h, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
	SDL_FillRect(surfBackground, 0, 0x00000000);
	SDL_Rect rc{ 0,0,0,0 };
	SDL_BlitSurface(surfText, 0, surfBackground, &rc);
	texture = SDL_CreateTextureFromSurface(g.pRenderer, surfBackground);
	SDL_FreeSurface(surfText);
	SDL_FreeSurface(surfBackground);
	GUI::Add(this);
}
Label::~Label()
{
	SDL_DestroyTexture(this->texture);
}
int getIndex(std::vector<GUIElement*> v, std::string K)
{
	for (size_t i = 0; i < v.size(); i++)
	{
		if (v[i]->key == K)
			return i;
	}
	return -1;
}
