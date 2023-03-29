#pragma once
#include "GUI.h"

Button::Button(std::string _key, std::string _text, TTF_Font* m_Font, SDL_Rect _rect, Relativity _rel, SDL_Color m_colText, SDL_Color m_colBackground, bool defaultEnabled)
{
	m_sKey = _key;
	m_bEnabled = defaultEnabled;
	m_bClickable = true;
	rel = _rel;
	orig = _rect;
	SetRect();
	m_sText = _text;
	SDL_Surface* surfText = TTF_RenderText_Blended(m_Font, m_sText.c_str(), m_colText);
	SDL_Surface* surfBackground = SDL_CreateRGBSurface(SDL_SWSURFACE, rect.w, rect.h, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
	SDL_FillRect(surfBackground, 0, _byteswap_ulong(*reinterpret_cast<Uint32*>(&m_colBackground)));
	SDL_Rect rc{ rect.w / 2 - surfText->w / 2,rect.h / 2 - surfText->h / 2,0,0 };
	SDL_BlitSurface(surfText, 0, surfBackground, &rc);
	m_Texture = SDL_CreateTextureFromSurface(g.mainRenderer, surfBackground);
	SDL_FreeSurface(surfText);
	SDL_FreeSurface(surfBackground);
	GUI::Add(this);
}
void Button::SetRect()
{
	switch (rel)
	{
	case TopLeft:
		rect = orig;
		break;
	case Top:
		rect = { (int)g.fWindowWidth / 2 + orig.x - orig.w / 2, orig.y, orig.w, orig.h };
		break;
	case TopRight:
		rect = { (int)g.fWindowWidth + orig.x, orig.y, orig.w, orig.h };
		break;
	case Left:
		rect = { orig.x, (int)g.fWindowHeight / 2 + orig.y, orig.w, orig.h };
		break;
	case Center:
		rect = { (int)g.fWindowWidth / 2 + orig.x, (int)g.fWindowHeight / 2 + orig.y, orig.w, orig.h };
		break;
	case Right:
		rect = { (int)g.fWindowWidth + orig.x, (int)g.fWindowHeight / 2 + orig.y, orig.w, orig.h };
		break;
	case BottomLeft:
		rect = { orig.x, (int)g.fWindowHeight + orig.y, orig.w, orig.h };
		break;
	case Bottom:
		rect = { (int)g.fWindowWidth / 2 + orig.x, (int)g.fWindowHeight + orig.y, orig.w, orig.h };
		break;
	case BottomRight:
		rect = { (int)g.fWindowWidth + orig.x - orig.w, (int)g.fWindowHeight - orig.h + orig.y, orig.w, orig.h };
		break;
	}
}
Button::~Button()
{
	SDL_DestroyTexture(m_Texture);
	std::cout << "removing" << this->m_sKey;
}
void Checkbox::SetRect()
{
	switch (rel)
	{
	case TopLeft:
		rect = orig;
		break;
	case Top:
		rect = { (int)g.fWindowWidth / 2 + orig.x, orig.y, orig.w, orig.h };
		break;
	case TopRight:
		rect = { (int)g.fWindowWidth + orig.x, orig.y, orig.w, orig.h };
		break;
	case Left:
		rect = { orig.x, (int)g.fWindowHeight / 2 + orig.y, orig.w, orig.h };
		break;
	case Center:
		rect = { (int)g.fWindowWidth / 2 + orig.x, (int)g.fWindowHeight / 2 + orig.y, orig.w, orig.h };
		break;
	case Right:
		rect = { (int)g.fWindowWidth + orig.x, (int)g.fWindowHeight / 2 + orig.y, orig.w, orig.h };
		break;
	case BottomLeft:
		rect = { orig.x, (int)g.fWindowHeight + orig.y, orig.w, orig.h };
		break;
	case Bottom:
		rect = { (int)g.fWindowWidth / 2 + orig.x, (int)g.fWindowHeight + orig.y, orig.w, orig.h };
		break;
	case BottomRight:
		rect = { (int)g.fWindowWidth + orig.x, (int)g.fWindowHeight + orig.y, orig.w, orig.h };
		break;
	}
}
Checkbox::Checkbox(std::string _key, std::string _text, TTF_Font* m_Font, SDL_Rect _rect, Relativity _rel, SDL_Color m_colText, SDL_Color m_colBackground, bool defaultState, bool defaultEnabled)
{
	m_sKey = _key;
	m_bClickable = true;
	m_bEnabled = defaultEnabled;
	m_bChecked = defaultState;
	rel = _rel;
	orig = _rect;
	SetRect();
	m_sText = _text;
	SDL_Surface* surfText = TTF_RenderText_Blended(m_Font, m_sText.c_str(), m_colText);
	SDL_Surface* surfBackground = SDL_CreateRGBSurface(SDL_SWSURFACE, rect.w, rect.h, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
	SDL_FillRect(surfBackground, 0, 0);
	auto bgColor = _byteswap_ulong(*reinterpret_cast<unsigned long*>(&m_colBackground));
	Draw::Rect(surfBackground, rect.h, *reinterpret_cast<SDL_Color*>(&bgColor));
	SDL_Rect rc{ rect.h + 5,0,rect.w,rect.h };
	SDL_BlitSurface(surfText, 0, surfBackground, &rc);
	m_texUnChecked = SDL_CreateTextureFromSurface(g.mainRenderer, surfBackground);
	Draw::Checkmark(surfBackground, rect.h, *reinterpret_cast<SDL_Color*>(&bgColor));
	m_texChecked = SDL_CreateTextureFromSurface(g.mainRenderer, surfBackground);
	SDL_FreeSurface(surfText);
	SDL_FreeSurface(surfBackground);
	GUI::Add(this);
}
Checkbox::~Checkbox()
{
	SDL_DestroyTexture(m_texChecked);
	SDL_DestroyTexture(m_texUnChecked);
}
void Slider::SetRect()
{
	switch (rel)
	{
	case TopLeft:
		rect = orig;
		break;
	case Top:
		rect = { (int)g.fWindowWidth / 2 + orig.x, orig.y, orig.w, orig.h };
		break;
	case TopRight:
		rect = { (int)g.fWindowWidth + orig.x, orig.y, orig.w, orig.h };
		break;
	case Left:
		rect = { orig.x, (int)g.fWindowHeight / 2 + orig.y, orig.w, orig.h };
		break;
	case Center:
		rect = { (int)g.fWindowWidth / 2 + orig.x, (int)g.fWindowHeight / 2 + orig.y, orig.w, orig.h };
		break;
	case Right:
		rect = { (int)g.fWindowWidth + orig.x, (int)g.fWindowHeight / 2 + orig.y, orig.w, orig.h };
		break;
	case BottomLeft:
		rect = { orig.x, (int)g.fWindowHeight + orig.y, orig.w, orig.h };
		break;
	case Bottom:
		rect = { (int)g.fWindowWidth / 2 + orig.x, (int)g.fWindowHeight + orig.y, orig.w, orig.h };
		break;
	case BottomRight:
		rect = { (int)g.fWindowWidth + orig.x, (int)g.fWindowHeight + orig.y, orig.w, orig.h };
		break;
	}
}
Slider::Slider(std::string _key, std::string _text, TTF_Font* m_Font, SDL_Rect _rect, Relativity _rel, SDL_Color m_colText, SDL_Color m_colBackground, int minValue, int maxValue, int defaultValue, bool defaultEnabled)
{
	m_sKey = _key;
	m_bClickable = true;
	m_bEnabled = defaultEnabled;
	rel = _rel;
	orig = _rect;
	SetRect();
	m_sText = _text;
	SDL_Surface* surfBackground = SDL_CreateRGBSurface(SDL_SWSURFACE, rect.w, rect.h, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
	SDL_FillRect(surfBackground, 0, 0);
	auto bgColor = _byteswap_ulong(*reinterpret_cast<unsigned long*>(&m_colBackground));
	Draw::Rect(surfBackground, rect.h, *reinterpret_cast<SDL_Color*>(&bgColor));
	SDL_Rect rc{ rect.h + 5,0,rect.w,rect.h };
	//SDL_BlitSurface(surfText, 0, surfBackground, &rc);
	//textureUnchecked = SDL_CreateTextureFromSurface(gVars.pRenderer, surfBackground);
	//SDL_FreeSurface(surfText);
	SDL_FreeSurface(surfBackground);
	GUI::Add(this);
}
void Block::SetRect()
{
	switch (rel)
	{
	case TopLeft:
		rect = orig;
		break;
	case Top:
		rect = { (int)g.fWindowWidth / 2 + orig.x, orig.y, orig.w, orig.h };
		break;
	case TopRight:
		rect = { (int)g.fWindowWidth + orig.x, orig.y, orig.w, orig.h };
		break;
	case Left:
		rect = { orig.x, (int)g.fWindowHeight / 2 + orig.y, orig.w, orig.h };
		break;
	case Center:
		rect = { (int)g.fWindowWidth / 2 + orig.x, (int)g.fWindowHeight / 2 + orig.y, orig.w, orig.h };
		break;
	case Right:
		rect = { (int)g.fWindowWidth + orig.x, (int)g.fWindowHeight / 2 + orig.y, orig.w, orig.h };
		break;
	case BottomLeft:
		rect = { orig.x, (int)g.fWindowHeight + orig.y, orig.w, orig.h };
		break;
	case Bottom:
		rect = { (int)g.fWindowWidth / 2 + orig.x, (int)g.fWindowHeight + orig.y, orig.w, orig.h };
		break;
	case BottomRight:
		rect = { (int)g.fWindowWidth + orig.x, (int)g.fWindowHeight + orig.y, orig.w, orig.h };
		break;
	}
}
Block::Block(std::string _key, SDL_Rect _rect, Relativity _rel, SDL_Color m_colBackground, bool defaultEnabled)
{
	m_sKey = _key;
	m_bClickable = false;
	m_bEnabled = defaultEnabled;
	rel = _rel;
	orig = _rect;
	SetRect();
	SDL_Surface* surfBackground = SDL_CreateRGBSurface(SDL_SWSURFACE, rect.w, rect.h, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
	SDL_FillRect(surfBackground, 0, _byteswap_ulong(*reinterpret_cast<Uint32*>(&m_colBackground)));
	m_Texture = SDL_CreateTextureFromSurface(g.mainRenderer, surfBackground);
	SDL_FreeSurface(surfBackground);
	GUI::Add(this);
}
Block::~Block()
{
	SDL_DestroyTexture(m_Texture);
}
void TextInput::SetRect()
{
	switch (rel)
	{
	case TopLeft:
		rect = orig;
		break;
	case Top:
		rect = { (int)g.fWindowWidth / 2 + orig.x - orig.w / 2, orig.y, orig.w, orig.h };
		break;
	case TopRight:
		rect = { (int)g.fWindowWidth + orig.x, orig.y, orig.w, orig.h };
		break;
	case Left:
		rect = { orig.x, (int)g.fWindowHeight / 2 + orig.y, orig.w, orig.h };
		break;
	case Center:
		rect = { (int)g.fWindowWidth / 2 + orig.x, (int)g.fWindowHeight / 2 + orig.y, orig.w, orig.h };
		break;
	case Right:
		rect = { (int)g.fWindowWidth + orig.x, (int)g.fWindowHeight / 2 + orig.y, orig.w, orig.h };
		break;
	case BottomLeft:
		rect = { orig.x, (int)g.fWindowHeight + orig.y, orig.w, orig.h };
		break;
	case Bottom:
		rect = { (int)g.fWindowWidth / 2 + orig.x, (int)g.fWindowHeight + orig.y, orig.w, orig.h };
		break;
	case BottomRight:
		rect = { (int)g.fWindowWidth + orig.x - orig.w, (int)g.fWindowHeight - orig.h + orig.y, orig.w, orig.h };
		break;
	}
}
void TextInput::Update()
{
	SDL_Surface* surfText = TTF_RenderText_Blended(m_Font, m_sText.empty() ? m_sTextEmpty.c_str() : m_sText.c_str(), m_sText.empty() ? m_colTextEmpty : m_colText);
	SDL_Surface* surfBackground = SDL_CreateRGBSurface(SDL_SWSURFACE, rect.w, rect.h, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
	SDL_FillRect(surfBackground, 0, _byteswap_ulong(*reinterpret_cast<Uint32*>(&m_colBackground)));
	SDL_Rect rc{ 10,rect.h / 2 - surfText->h / 2,0,0 };
	SDL_BlitSurface(surfText, 0, surfBackground, &rc);
	if (m_Texture)
		SDL_DestroyTexture(m_Texture);
	m_Texture = SDL_CreateTextureFromSurface(g.mainRenderer, surfBackground);
	SDL_FreeSurface(surfText);
	SDL_FreeSurface(surfBackground);
}
TextInput::TextInput(std::string _key, std::string _text, std::string _textEmpty, TTF_Font* _font, SDL_Rect _rect, Relativity _rel, SDL_Color _textColor, SDL_Color _textColorEmpty, SDL_Color _backgroundColor, bool defaultEnabled)
{
	m_sKey = _key;
	m_bEnabled = defaultEnabled;
	m_bClickable = true;
	rel = _rel;
	orig = _rect;
	SetRect();
	m_sText = _text;
	m_sTextEmpty = _textEmpty;
	m_Font = _font;
	m_colText = _textColor;
	m_colTextEmpty = _textColorEmpty;
	m_colBackground = _backgroundColor;
	Update();
	GUI::Add(this);
}
TextInput::~TextInput()
{
	SDL_DestroyTexture(m_Texture);
	std::cout << "removing" << this->m_sKey;
}

void Label::SetRect()
{
	switch (rel)
	{
	case TopLeft:
		rect = orig;
		break;
	case Top:
		rect = { (int)g.fWindowWidth / 2 + orig.x - orig.w / 2, orig.y, orig.w, orig.h };
		break;
	case TopRight:
		rect = { (int)g.fWindowWidth + orig.x, orig.y, orig.w, orig.h };
		break;
	case Left:
		rect = { orig.x, (int)g.fWindowHeight / 2 + orig.y, orig.w, orig.h };
		break;
	case Center:
		rect = { (int)g.fWindowWidth / 2 + orig.x, (int)g.fWindowHeight / 2 + orig.y, orig.w, orig.h };
		break;
	case Right:
		rect = { (int)g.fWindowWidth + orig.x, (int)g.fWindowHeight / 2 + orig.y, orig.w, orig.h };
		break;
	case BottomLeft:
		rect = { orig.x, (int)g.fWindowHeight + orig.y, orig.w, orig.h };
		break;
	case Bottom:
		rect = { (int)g.fWindowWidth / 2 + orig.x, (int)g.fWindowHeight + orig.y, orig.w, orig.h };
		break;
	case BottomRight:
		rect = { (int)g.fWindowWidth + orig.x - orig.w, (int)g.fWindowHeight - orig.h + orig.y + orig.y, orig.w, orig.h };
		break;
	}
}
Label::Label(std::string _key, std::string _text, TTF_Font* m_Font, SDL_Rect _rect, Relativity _rel, SDL_Color m_colText, bool defaultEnabled)
{
	m_sKey = _key;
	m_bEnabled = defaultEnabled;
	m_bClickable = false;
	rel = _rel;
	orig = _rect;
	SetRect();
	m_sText = _text;
	SDL_Surface* surfText = TTF_RenderText_Blended_Wrapped(m_Font, m_sText.c_str(), m_colText, rect.w);
	SDL_Surface* surfBackground = SDL_CreateRGBSurface(SDL_SWSURFACE, rect.w, rect.h, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
	SDL_FillRect(surfBackground, 0, 0x00000000);
	SDL_Rect rc{ 0,0,0,0 };
	SDL_BlitSurface(surfText, 0, surfBackground, &rc);
	m_Texture = SDL_CreateTextureFromSurface(g.mainRenderer, surfBackground);
	SDL_FreeSurface(surfText);
	SDL_FreeSurface(surfBackground);
	GUI::Add(this);
}
Label::~Label()
{
	SDL_DestroyTexture(this->m_Texture);
}
int getIndex(std::vector<GUIElement*> v, std::string K)
{
	for (size_t i = 0; i < v.size(); i++)
	{
		if (v[i]->m_sKey == K)
			return i;
	}
	return -1;
}
