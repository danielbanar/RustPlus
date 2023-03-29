#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <nlohmann/json.hpp>
#include <vector>
#include "texture.h"
#include "rustplus.pb.h"
#include "rustsocket.h"
#include "GUI.h"
#define MAX_FPS 60
#define DEG2RAD(deg) ((deg) * 0.01745329238474369049072265625f)
#define PIXEL_SIZE 2
class GlobalVars
{
public:
	//Files
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

	//SDL
	SDL_Window* mainWindow;
	SDL_Renderer* mainRenderer;
	TTF_Font* fontPermanentMarker;
	TTF_Font* fontTahoma;
	TTF_Font* fontTahomaBold;

	//GAME STUFF
	float fWindowWidth = 1280.f, fWindowHeight=720.f;
	float fMapZoom = 1.f, fMapX = fWindowWidth/2.f, fMapY = fWindowHeight/2.f;
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

	//Networking
	void* socket;
	NStatus NSmapMarkers;
	NStatus NSTeamChat;
	NStatus NSTeamInfo;
	NMessageType msgType;
	std::chrono::high_resolution_clock::time_point tLastCamera = std::chrono::high_resolution_clock::now();
	std::chrono::high_resolution_clock::time_point tLast = std::chrono::high_resolution_clock::now();
	std::chrono::high_resolution_clock::time_point tNow = std::chrono::high_resolution_clock::now();
	//Experimental
	bool skipRender = false;
	std::string input;
	std::string activeCamera;
};

extern GlobalVars g;