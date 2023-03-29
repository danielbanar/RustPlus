#include "render.h"

void Render_Map()
{
	SDL_Rect mapRect{
		(g.fWindowWidth/2.f) - g.fMapX * g.fMapZoom,
		(g.fWindowHeight/2.f) - g.fMapY * g.fMapZoom,
		g.fWindowHeight* g.fMapZoom,
		g.fWindowHeight* g.fMapZoom };
	SDL_RenderCopy(g.mainRenderer, g.mapTexture, 0, &mapRect);

}
void Render_CustomMapMarkers()
{
	for (auto& myMarker : g.vecMyMarkers)
	{
		auto rect = GetRect(myMarker.first, myMarker.second, g.appInfo.mapsize(), g.appMap.width(), false, 40, 40);
		SDL_RenderCopy(g.mainRenderer, Icons::myMarker, 0, &rect);
	}
}
void Render_Events()
{
	static float angle = 0.f;
	angle += 10.f;
	for (int i = 0; i < g.appMapMarkers.markers_size(); i++)
	{
		auto& marker = g.appMapMarkers.markers().Get(i);
		if (marker.type() == Crate)
		{
			auto rect = GetRect(marker.x(), marker.y(), g.appInfo.mapsize(), g.appMap.width(), false, 32, 32);
			SDL_RenderCopy(g.mainRenderer, Icons::crate, 0, &rect);
		}
		else if (marker.type() == Explosion)
		{
			auto rect = GetRect(marker.x(), marker.y(), g.appInfo.mapsize(), g.appMap.width(), false, 32, 32);
			SDL_RenderCopy(g.mainRenderer, Icons::explosion, 0, &rect);
		}
		else if (marker.type() == CargoShip)
		{
			auto rect = GetRect(marker.x(), marker.y(), g.appInfo.mapsize(), g.appMap.width(), true, 32, 32);
			SDL_RenderCopyEx(g.mainRenderer, Icons::cargo, 0, &rect, -marker.rotation() - 90.0, 0, SDL_FLIP_NONE);
		}
		else if (marker.type() == CH47 || marker.type() == PatrolHelicopter)
		{
			float x = marker.x() + 1000.f;
			float y = g.appInfo.mapsize() - marker.y() + 1000.f;
			float scale = g.appMap.width() / (g.fWindowWidth / 2.f);
			x /= scale;
			y /= scale;
			SDL_Rect rect{
				(g.fWindowWidth / 2.f) - (10.f * g.fMapZoom) + (x - g.fMapX) * g.fMapZoom,
				(g.fWindowHeight / 2.f) - (10.f * g.fMapZoom) + (y - g.fMapY) * g.fMapZoom,
				20.f * g.fMapZoom,
				20.f * g.fMapZoom };
			SDL_RenderCopyEx(g.mainRenderer, Icons::chinook, 0, &rect, -marker.rotation() - 90.0, 0, SDL_FLIP_NONE);

			SDL_Rect rectBlades1{
				(g.fWindowWidth / 2.f) - (6.f * g.fMapZoom) + (x + 6.f * sinf(DEG2RAD(marker.rotation())) - g.fMapX) * g.fMapZoom,
				(g.fWindowHeight / 2.f) - (6.f * g.fMapZoom) + (y + 6.f * cosf(DEG2RAD(marker.rotation())) - g.fMapY) * g.fMapZoom,
				12.f * g.fMapZoom,
				12.f * g.fMapZoom };
			SDL_Rect rectBlades2{
				(g.fWindowWidth / 2.f) - (6.f * g.fMapZoom) + (x + 6.f * -sinf(DEG2RAD(marker.rotation())) - g.fMapX) * g.fMapZoom,
				(g.fWindowHeight / 2.f) - (6.f * g.fMapZoom) + (y + 6.f * -cosf(DEG2RAD(marker.rotation())) - g.fMapY) * g.fMapZoom,
				12.f * g.fMapZoom,
				12.f * g.fMapZoom };
			SDL_RenderCopyEx(g.mainRenderer, Icons::blade, 0, &rectBlades1, angle, 0, SDL_FLIP_NONE);
			SDL_RenderCopyEx(g.mainRenderer, Icons::blade, 0, &rectBlades2, -angle, 0, SDL_FLIP_NONE);
		}
		else if (marker.type() == VendingMachine)
		{
			//marker.sellorders().Get(0).
		}
	}
}
void Render_Markers()
{
	//Draw Team markers
	for (int i = 0; i < g.appTeamInfo.leadermapnotes_size(); i++)
	{
		auto& teamMarker = g.appTeamInfo.leadermapnotes().Get(i);
		float x = teamMarker.x() + 1000.f;
		float y = g.appInfo.mapsize() - teamMarker.y() + 1000.f;
		float scale = g.appMap.height() / (g.fWindowHeight / 2.f);
		x /= scale;
		y /= scale;

		SDL_Rect rectMarker{
			(g.fWindowWidth / 2.f) - 16.f + (x - g.fMapX) * g.fMapZoom,
			(g.fWindowHeight / 2.f) - 32.f + (y - g.fMapY) * g.fMapZoom,
			32.f,
			32.f };
		SDL_RenderCopy(g.mainRenderer, Icons::marker, 0, &rectMarker);
	}
	//Draw markers
	for (int i = 0; i < g.appTeamInfo.mapnotes_size(); i++)
	{
		auto& teamMarker = g.appTeamInfo.mapnotes().Get(i);
		float x = teamMarker.x() + 1000.f;
		float y = g.appInfo.mapsize() - teamMarker.y() + 1000.f;
		float scale = g.appMap.height() / (g.fWindowHeight / 2.f);
		x /= scale;
		y /= scale;
		SDL_Rect rectMarker{
			(g.fWindowWidth / 2.f) - 16.f + (x - g.fMapX) * g.fMapZoom,
			(g.fWindowHeight / 2.f) - (teamMarker.type() ? 32.f : 16.f) + (y - g.fMapY) * g.fMapZoom,
			32.f,
			32.f };
		SDL_RenderCopy(g.mainRenderer, teamMarker.type() ? Icons::marker : Icons::death, 0, &rectMarker);
	}
}
void Render_TeamMembers()
{
	for (int i = 0; i < g.appTeamInfo.members_size(); i++)
	{
		auto& member = g.appTeamInfo.members().Get(i);
		if (g.bFocus && member.steamid() == g.jID)
		{
			float x = member.x() + 1000.f;
			float y = g.appInfo.mapsize() - member.y() + 1000.f;
			float scale = g.appMap.height() / (g.fWindowHeight / 2.f);
			x /= scale;
			y /= scale;
			g.fMapX = x;
			g.fMapY = y;
		}
		auto rect = GetRect(member.x(), member.y(), g.appInfo.mapsize(), g.appMap.width(), false, 16, 16);
		SDL_RenderCopy(g.mainRenderer, member.isalive() ? member.isonline() ? Icons::player : Icons::playerOff : Icons::playerDead, 0, &rect);

		auto nametag = g.nametags.find(member.name());
		if (nametag == g.nametags.end())//Add to map
		{
			TTF_SetFontOutline(g.fontTahoma, 1);
			auto sOutline = TTF_RenderText_Solid(g.fontTahoma, member.name().c_str(), { 0, 0, 0 });
			TTF_SetFontOutline(g.fontTahoma, 0);
			auto sText = TTF_RenderText_Solid(g.fontTahoma, member.name().c_str(), { 170, 255, 0 });
			if (!ignoreErrors)
				std::cout << TTF_GetError();
			SDL_Surface* sName = SDL_CreateRGBSurface(0, sOutline->w, sOutline->h, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
			SDL_Rect rectName = { 1, 1, sText->w, sText->h };
			SDL_BlitSurface(sOutline, 0, sName, 0);
			SDL_BlitSurface(sText, 0, sName, &rectName);
			SDL_FreeSurface(sOutline);
			SDL_FreeSurface(sText);
			g.nametags.emplace(std::make_pair(member.name(), Texture{ SDL_CreateTextureFromSurface(g.mainRenderer, sName), sName->w, sName->h }));
			nametag = g.nametags.find(member.name());
			SDL_FreeSurface(sName);
		}
		rect.x -= nametag->second.w / 2 - 8;
		rect.y -= nametag->second.h;
		rect.w = nametag->second.w;
		rect.h = nametag->second.h;
		SDL_RenderCopy(g.mainRenderer, nametag->second.m_Texture, 0, &rect);

	}
}
void Render_GUI()
{
	for (GUIElement* element : GUI::Elements()) {
		if (element->m_bEnabled) {
			if (Block* block = dynamic_cast<Block*>(element)) {
				SDL_RenderCopy(g.mainRenderer, block->m_Texture, 0, &block->rect);
			}
			if (Button* button = dynamic_cast<Button*>(element)) {
				SDL_RenderCopy(g.mainRenderer, button->m_Texture, 0, &button->rect);
			}
			if (Checkbox* checkbox = dynamic_cast<Checkbox*>(element)) {
				SDL_RenderCopy(g.mainRenderer, checkbox->m_bChecked ? checkbox->m_texChecked : checkbox->m_texUnChecked, 0, &checkbox->rect);
			}
			if (Label* label = dynamic_cast<Label*>(element)) {
				SDL_RenderCopy(g.mainRenderer, label->m_Texture, 0, &label->rect);
			}
			if (TextInput* textInput = dynamic_cast<TextInput*>(element)) {
				SDL_RenderCopy(g.mainRenderer, textInput->m_Texture, 0, &textInput->rect);
			}
		}
	}
}

extern SDL_Renderer* cameraRenderer;
void Render_CameraWindow()
{
	static SDL_Window* window = SDL_CreateWindow("Camera Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 160*PIXEL_SIZE, 90*PIXEL_SIZE, 0);
	SDL_SetWindowTitle(window, g.activeCamera.c_str());
	if(!cameraRenderer)
		cameraRenderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	SDL_ShowWindow(window);
	// Clear the renderer to black
	SDL_SetRenderDrawColor(cameraRenderer, 0, 0, 0, 255);
	SDL_RenderClear(cameraRenderer);

	// Update the window
	SDL_RenderPresent(cameraRenderer);
}