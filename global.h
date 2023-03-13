#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <nlohmann/json.hpp>
#include <vector>
#include "texture.h"
#include "rustplus.pb.h"
#include "rustsocket.h"
#define MAX_FPS 60
#define MAP_WINDOW_SIZE 800.f
#define MAP_SIZE_HALF (MAP_WINDOW_SIZE * .5f)
class GlobalVars
{
public:
	//
	nlohmann::json json;
	std::string jIP;
	std::string jPort;
	uint64_t jID;
	int32_t jToken;
	bool bRunning = true;
	bool bFocus = false;
	bool connected = false;
	std::string connectedServerName = "";
	std::string connectedServerFile = "servers\\unconnected";
	//GUI STUFF
	SDL_Renderer* pRenderer;
	TTF_Font* fontPermanentMarker;
	TTF_Font* fontTahoma;
	TTF_Font* fontTahomaBold;
	//GAME STUFF
	float fMapZoom = 1.f, fMapX = MAP_SIZE_HALF, fMapY = MAP_SIZE_HALF;
	int lastX = 0, lastY = 0;
	std::vector<std::pair<int, int>> vecMyMarkers;
	std::vector<rustplus::AppMarker> vecVendings;
	std::vector<rustplus::AppMarker_SellOrder> vecOrders;
	std::map<std::string, Texture> nametags;
	rustplus::AppTeamInfo_Member localPlayer;
	SDL_Texture* mapTexture;
	std::vector<std::string> servers;
	//APP STUFF
	rustplus::AppMap appMap;
	rustplus::AppInfo appInfo;
	rustplus::AppMapMarkers appMapMarkers;
	rustplus::AppTeamInfo appTeamInfo;
	rustplus::AppCameraInfo appCameraInfo;
	Events curEvents;
	Events lastEvents;
	rustplus::AppTeamChat curTeamChat;
	rustplus::AppTeamChat lastTeamChat;
	void* socket;
	//Networking
	NStatus NSmapMarkers;
	NStatus NSTeamChat;
	NStatus NSTeamInfo;
	NMessageType msgType;
	std::chrono::high_resolution_clock::time_point tLast = std::chrono::high_resolution_clock::now();
	std::chrono::high_resolution_clock::time_point tNow = std::chrono::high_resolution_clock::now();
	//Experimental
	bool skipRender = false;
	std::string input;
};

extern GlobalVars g;