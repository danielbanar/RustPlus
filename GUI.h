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
	virtual void OnClick(std::function<void()> func) { m_func = func; };
	virtual void Click()
	{
		if (m_bClickable && m_func)
			m_func();
	}
	SDL_Rect orig;
	SDL_Rect rect;
	Relativity rel;
	bool m_bClickable = false;
	bool m_bEnabled = true;
	std::function<void()> m_func;
	std::string m_sKey;
	virtual void SetRect() = 0;
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
private:
	GUI() {}
	std::vector<GUIElement*> gui;
};

class Button : public GUIElement
{
public:
	Button() = delete;
	Button(std::string _key, std::string _text, TTF_Font* m_Font, SDL_Rect _rect, Relativity _rel, SDL_Color m_colText, SDL_Color m_colBackground, bool defaultEnabled);
	~Button();
	SDL_Texture* m_Texture;
	void SetRect();
private:
	std::string m_sText;
};

class TextInput : public GUIElement
{
public:
	TextInput() = delete;
	TextInput(std::string _key, std::string _text, std::string _textEmpty, TTF_Font* _font, SDL_Rect _rect, Relativity _rel, SDL_Color _textColor, SDL_Color _textColorEmpty, SDL_Color _backgroundColor, bool defaultEnabled);
	~TextInput();
	SDL_Texture* m_Texture;
	std::string m_sText;
	std::string m_sTextEmpty;
	void OnClick(std::function<void()> func) { m_func = func; };
	void Click()
	{
		if (m_func)
			m_func();
	}
	void Update();
	void SetRect();
private:
	TTF_Font* m_Font;
	SDL_Color m_colText, m_colTextEmpty, m_colBackground;
};

class Checkbox : public GUIElement
{
public:
	Checkbox() = delete;
	Checkbox(std::string _key, std::string _text, TTF_Font* m_Font, SDL_Rect _rect, Relativity _rel, SDL_Color m_colText, SDL_Color m_colBackground, bool defaultState, bool defaultEnabled);
	~Checkbox();
	void Click() override
	{
		m_bChecked ? Uncheck() : Check();
		m_bChecked = !m_bChecked;
	}
	void Check()
	{
		if (m_func)
			m_func();
	}
	void Uncheck()
	{
		if (m_func2)
			m_func2();
	}
	void OnCheck(std::function<void()> func) { m_func = func; };
	void OnUncheck(std::function<void()> func) { m_func2 = func; };
	void SetRect();
	SDL_Texture* m_texChecked;
	SDL_Texture* m_texUnChecked;
	bool m_bChecked;
private:
	std::string m_sText;
	std::function<void()> m_func2;
};


class Slider : public GUIElement
{
public:
	Slider() = delete;
	Slider(std::string _key, std::string _text, TTF_Font* m_Font, SDL_Rect _rect, Relativity _rel, SDL_Color m_colText, SDL_Color m_colBackground, int minValue, int maxValue, int defaultValue, bool defaultEnabled);
	void ChangeValue(int val)
	{
		m_iValue = val;
	}
	void SetRect();
	SDL_Texture* m_Texture;
	int m_iValue;
private:
	std::string m_sText;
};


class Block : public GUIElement
{
public:
	Block() = delete;
	Block(std::string _key, SDL_Rect _rect, Relativity _rel, SDL_Color m_colBackground, bool defaultEnabled);
	~Block();
	void SetRect();
	SDL_Texture* m_Texture;
};

class Label : public GUIElement
{
public:
	Label() = delete;
	Label(std::string _key, std::string _text, TTF_Font* m_Font, SDL_Rect _rect, Relativity _rel, SDL_Color m_colText, bool defaultEnabled);
	~Label();
	void SetRect();
	SDL_Texture* m_Texture;
private:
	std::string m_sText;
};