#pragma once
#include <iostream>
#include <vector>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <intrin.h>
#include "draw.h"
#include "global.h"
struct Position
{
	int x, y;
};
struct Size
{
	int w, h;
};
enum Relativity
{
	TopLeft,
	Top,
	TopRight,
	Left,
	Center,
	Right,
	BottomLeft,
	Bottom,
	BottomRight
};
namespace GUIColor
{
	static SDL_Color Red{ 255,0,0,255 };
	static SDL_Color Green{ 0,255,0,255 };
	static SDL_Color Blue{ 0,0,255,255 };
	static SDL_Color White{ 255,255,255,255 };
	static SDL_Color Black{ 0,0,0,255 };
	static SDL_Color Gray{ 45,45,45,255 };
	static SDL_Color Yellow{ 255,255,0,255 };
	static SDL_Color Cyan{ 0,255,255,255 };
	static SDL_Color Purple{ 255,0,255,255 };
	static SDL_Color Transparent{ 0,0,0,0 };
};
class GUIElement
{
public:
	GUIElement() {}
	~GUIElement() {}
	virtual void OnClick(std::function<void()> func) { ev = func; };
	virtual void Click()
	{
		if (clickable && ev)
			ev();
	}
	SDL_Rect rect;
	Relativity rel;
	bool clickable = false;
	bool enabled = true;
	std::function<void()> ev;
	std::string key;
};
int getIndex(std::vector<GUIElement*> v, std::string K);

class GUI
{
public:
	GUI(const GUI&) = delete;

	static GUI& Get()
	{
		static GUI instance;
		return instance;
	}
	static void Add(GUIElement* element) { Get().gui.push_back(element); }
	//static void Remove(std::string _key) { Get().gui.erase(Get().gui.begin() + getIndex(Get().gui, _key)); }
	static std::vector<GUIElement*>& Elements() { return Get().gui; }
	SDL_Rect windowRect;
private:
	GUI() {}
	std::vector<GUIElement*> gui;
};

class Button : public GUIElement
{
public:
	Button() = delete;
	Button(std::string _key, std::string _text, TTF_Font* font, SDL_Rect _rect, Relativity _rel, SDL_Color textColor, SDL_Color backgroundColor, bool defaultEnabled);
	~Button();
	SDL_Texture* texture;
private:
	std::string text;
};


class Checkbox : public GUIElement
{
public:
	Checkbox() = delete;
	Checkbox(std::string _key, std::string _text, TTF_Font* font, SDL_Rect _rect, Relativity _rel, SDL_Color textColor, SDL_Color backgroundColor, bool defaultState, bool defaultEnabled);
	~Checkbox();
	void Click() override
	{
		checked ? Uncheck() : Check();
		checked = !checked;
	}
	void Check()
	{
		if (ev)
			ev();
	}
	void Uncheck()
	{
		if (ev2)
			ev2();
	}
	void OnCheck(std::function<void()> func) { ev = func; };
	void OnUncheck(std::function<void()> func) { ev2 = func; };
	SDL_Texture* textureChecked;
	SDL_Texture* textureUnchecked;
	bool checked;
private:
	std::string text;
	std::function<void()> ev2;
};


class Slider : public GUIElement
{
public:
	Slider() = delete;
	Slider(std::string _key, std::string _text, TTF_Font* font, SDL_Rect _rect, Relativity _rel, SDL_Color textColor, SDL_Color backgroundColor, int minValue, int maxValue, int defaultValue, bool defaultEnabled);
	void ChangeValue(int val)
	{
		value = val;
	}
	SDL_Texture* texture;
	int value;
private:
	std::string text;
};


class Block : public GUIElement
{
public:
	Block() = delete;
	Block(std::string _key, SDL_Rect _rect, Relativity _rel, SDL_Color backgroundColor, bool defaultEnabled);
	~Block();
	SDL_Texture* texture;
};

class Label : public GUIElement
{
public:
	Label() = delete;
	Label(std::string _key, std::string _text, TTF_Font* font, SDL_Rect _rect, Relativity _rel, SDL_Color textColor, bool defaultEnabled);
	~Label();
	SDL_Texture* texture;
private:
	std::string text;
};