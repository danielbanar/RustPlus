#pragma once
#include <fstream>
#include <windowsx.h>
#include <inttypes.h>
#include <tuple>
#include <array>
#include <functional>
#include <iomanip>
#include <string>
#include <sstream>
#include <thread>
#include <chrono>
#include "rustsocket.h"
#include "GUI.h"
#include "main.h"
#include "resources.h"
#include "cameradecoder.h"
#include "render.h"
#include "utils.h"

auto& json = g.json;
float& fMapZoom = g.fMapZoom;
float& fMapX = g.fMapX;
float& fMapY = g.fMapY;

RustSocket* rs = (RustSocket*)g.socket;
SDL_Window* pWindow;
SDL_Renderer* cameraRenderer;
bool alertCargo = true, alertPatrol = true, alertCrate = true, alertExplosion = true, alertChinook = true;

int main(int argc, char* argv[])
{
	if (SDL_Init(SDL_INIT_EVERYTHING) == -1 && !ignoreErrors)
		std::cout << "Init error: " << SDL_GetError();
	pWindow = SDL_CreateWindow("Rust+ PC | unconnected", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, MAP_WINDOW_SIZE, MAP_WINDOW_SIZE, SDL_WINDOW_SHOWN);
	if (!pWindow && !ignoreErrors)
		std::cout << "Window error: " << SDL_GetError();
	SDL_SetWindowResizable(pWindow, SDL_FALSE);
	g.pRenderer = SDL_CreateRenderer(pWindow, -1, SDL_RENDERER_PRESENTVSYNC);
	if (!g.pRenderer && !ignoreErrors)
		std::cout << "Renderer error: " << SDL_GetError();
	TTF_Init();
	//Font init
	g.fontPermanentMarker = TTF_OpenFontRW(SDL_RWFromMem(Fonts::PermanentMarker, 73620), 1, 24);
	g.fontTahoma = TTF_OpenFont("C:\\Windows\\Fonts\\Tahoma.ttf", 11);
	g.fontTahomaBold = TTF_OpenFont("C:\\Windows\\Fonts\\tahomabd.ttf", 14);
	TTF_SetFontStyle(g.fontTahoma, TTF_STYLE_BOLD);
	TTF_SetFontHinting(g.fontTahoma, TTF_HINTING_MONO);
	TTF_SetFontHinting(g.fontTahomaBold, 1);

	Icons::CreateTextures();
	g.servers = GetServerlist();
	if (g.servers.size() == 0)
		CreateTemplateFile();

	//GUI Instancing
	SDL_GetWindowSize(pWindow, &GUI::Get().windowRect.w, &GUI::Get().windowRect.h); //Reativity garbage
	static bool bSettings = false;
	Button* focusButton = new Button("button_focus", "Focus", g.fontTahomaBold, { 10, 10, 80, 30 }, TopLeft, GUIColor::White, { 115,140,69,200 }, false);
	focusButton->OnClick([&]() { g.bFocus = !g.bFocus; });

	Button* cameraButton = new Button("button_camera", "OILRIG2HELI", g.fontTahomaBold, { 10, 10, 80, 30 }, Left, GUIColor::White, { 115,140,69,200 }, true);
	cameraButton->OnClick([&]() { g.appCameraInfo = rs->Subscribe("OILRIG2HELI");
	Render_CameraWindow(); });

	Block* settingsBlock = new Block("block_settings", { -200, 0, 200, 1000 }, TopRight, { 0,0,0,200 }, false);

	Button* registerButton = new Button("button_register", "Sign in", g.fontTahomaBold, { -105, -120, 80, 25 }, BottomRight, GUIColor::White, { 115,140,69,200 }, false);
	registerButton->OnClick([]()
		{
			ShellExecute(NULL, "runas", "cmd", "/c npx @liamcottle/rustplus.js fcm-register", NULL, SW_SHOW);
		});
	Button* listenButton = new Button("button_listen", "Listen", g.fontTahomaBold, { -15, -120, 80, 25 }, BottomRight, GUIColor::White, { 115,140,69,200 }, false);
	listenButton->OnClick([]()
		{
			ShellExecute(NULL, "runas", "cmd", "/c npx @liamcottle/rustplus.js fcm-listen > C:/listen.temp", NULL, SW_SHOW);
		});
	Button* importButton = new Button("button_import", "Import", g.fontTahomaBold, { -15, -85, 170, 25 }, BottomRight, GUIColor::White, { 115,140,69,200 }, false);
	importButton->OnClick([&]() { Import(); });
	Button* disconnectButton = new Button("button_disconnect", "Disconnect", g.fontTahomaBold, { -15, -50, 170, 25 }, BottomRight, GUIColor::White, { 150,40,40,200 }, false);
	disconnectButton->OnClick([&]() { Disconnect(); });

	Label* creditsLabel = new Label("label_credits", "Made by Bananik007\nCredits: liamcottle (Rust+ API)", g.fontTahoma, { 0,0,190,35 }, BottomRight, GUIColor::White, false);

	Checkbox* ignoreErrorsCheckbox = new Checkbox("checkbox_ignore_errors", "Ignore Errors", g.fontTahomaBold, { -180, 65,200,18 }, TopRight, GUIColor::White, GUIColor::White, false, false);
	ignoreErrorsCheckbox->OnCheck([&]() {ignoreErrors = true; });
	ignoreErrorsCheckbox->OnUncheck([&]() {ignoreErrors = false; });

	Checkbox* cargoCheckbox = new Checkbox("checkbox_alert_cargo", "Cargo alerts", g.fontTahomaBold, { -180, 100,200,18 }, TopRight, GUIColor::White, GUIColor::White, true, false);
	cargoCheckbox->OnCheck([&]() {alertCargo = true; });
	cargoCheckbox->OnUncheck([&]() {alertCargo = false; });
	Checkbox* patrolCheckbox = new Checkbox("checkbox_alert_patrol", "Patrol alerts", g.fontTahomaBold, { -180, 125,200,18 }, TopRight, GUIColor::White, GUIColor::White, true, false);
	patrolCheckbox->OnCheck([&]() {alertPatrol = true; });
	patrolCheckbox->OnUncheck([&]() {alertPatrol = false; });
	Checkbox* exploCheckbox = new Checkbox("checkbox_alert_explo", "Explosion alerts", g.fontTahomaBold, { -180, 150,200,18 }, TopRight, GUIColor::White, GUIColor::White, true, false);
	exploCheckbox->OnCheck([&]() {alertExplosion = true; });
	exploCheckbox->OnUncheck([&]() {alertExplosion = false; });
	Checkbox* chinookCheckbox = new Checkbox("checkbox_alert_chinook", "Chinook alerts", g.fontTahomaBold, { -180, 175,200,18 }, TopRight, GUIColor::White, GUIColor::White, true, false);
	chinookCheckbox->OnCheck([&]() {alertChinook = true; });
	chinookCheckbox->OnUncheck([&]() {alertChinook = false; });
	Checkbox* crateCheckbox = new Checkbox("checkbox_alert_crate", "Crate alerts", g.fontTahomaBold, { -180, 200,200,18 }, TopRight, GUIColor::White, GUIColor::White, true, false);
	crateCheckbox->OnCheck([&]() {alertCrate = true; });
	crateCheckbox->OnUncheck([&]() {alertCrate = false; });
	Button* settingsButton = new Button("button_settings", "Settings", g.fontTahomaBold, { -100,10,80,30 }, TopRight, GUIColor::White, GUIColor::Gray, true);
	settingsButton->OnClick([&]() {bSettings = !bSettings;
	settingsBlock->enabled = bSettings;
	ignoreErrorsCheckbox->enabled = bSettings;
	cargoCheckbox->enabled = bSettings;
	patrolCheckbox->enabled = bSettings;
	exploCheckbox->enabled = bSettings;
	chinookCheckbox->enabled = bSettings;
	crateCheckbox->enabled = bSettings;
	creditsLabel->enabled = bSettings;
	registerButton->enabled = bSettings;
	listenButton->enabled = bSettings;
	importButton->enabled = bSettings;
	disconnectButton->enabled = bSettings; /*&& g.connected;*/ });

	//TODO: fix mem leak
	for (int i = 0; i < g.servers.size(); i++)
	{
		Button* serverButton = new Button(std::string("button_serverconnect") + std::to_string(i), g.servers[i], g.fontTahomaBold, { 10, 10 + (i * 40), 250, 30 }, TopLeft, GUIColor::White, GUIColor::Gray, true);
		serverButton->OnClick([i]() { g.connected = Connect(g.servers[i]); });
	}

	SDL_SetRenderDrawColor(g.pRenderer, 18, 64, 77, 255);

	std::thread second(listen_for_input);

	//Main loop
	while (g.bRunning)
	{
		PollEvents();
		NetLoop();
		Render();
		Sleep(1000.f / (float)MAX_FPS);
	}
	//Cleaning up
	SDL_DestroyTexture(g.mapTexture);
	SDL_DestroyTexture(Icons::crate);
	SDL_DestroyTexture(Icons::explosion);
	SDL_DestroyTexture(Icons::cargo);
	SDL_DestroyTexture(Icons::chinook);
	SDL_DestroyTexture(Icons::blade);
	SDL_DestroyTexture(Icons::marker);
	SDL_DestroyTexture(Icons::death);
	SDL_DestroyTexture(Icons::player);
	SDL_DestroyTexture(Icons::playerDead);
	SDL_DestroyTexture(Icons::playerOff);
	SDL_DestroyTexture(Icons::myMarker);
	SDL_DestroyRenderer(g.pRenderer);
	SDL_DestroyWindow(pWindow);
	if (rs)
		rs->ws->close();
	TTF_Quit();
	SDL_Quit();
	return 0;
}

void GUIInteraction(const SDL_Event& ev)
{
	for (GUIElement* element : GUI::Elements())
	{
		if (g.skipRender)
		{
			g.skipRender = false;
			return;
		}
		if (!element->enabled)
			continue;
		if (Button* button = dynamic_cast<Button*>(element))
			if (ev.button.x > button->rect.x && ev.button.y > button->rect.y && ev.button.x < button->rect.x + button->rect.w && ev.button.y < button->rect.y + button->rect.h)
				button->Click();
		if (Checkbox* checkbox = dynamic_cast<Checkbox*>(element))
			if (ev.button.x > checkbox->rect.x && ev.button.y > checkbox->rect.y && ev.button.x < checkbox->rect.x + checkbox->rect.w && ev.button.y < checkbox->rect.y + checkbox->rect.h)
				checkbox->Click();
	}
}



void PollEvents()
{
	SDL_Event ev;
	while (SDL_PollEvent(&ev) != 0)
	{
		switch (ev.type)
		{
		case SDL_QUIT:
			g.bRunning = false;
			exit(0);//TODO: Fix error message due to second thread
			break;
		case SDL_MOUSEWHEEL:
			//Zooming
			if (ev.wheel.y > 0)
				fMapZoom *= 1.2f;
			else if (ev.wheel.y < 0)
				fMapZoom *= 1.f / 1.2f;
			if (fMapZoom < 1.f)
				fMapZoom = 1.f;
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (ev.button.button == SDL_BUTTON_LEFT)
				GUIInteraction(ev);
			if (ev.button.button == SDL_BUTTON_RIGHT)
			{
				if (!g.connected)
					continue;
				float dist = 9999;
				int closest = 0;
				for (size_t i = 0; i < g.vecMyMarkers.size(); i++)
				{
					auto screenpos = GetRect(g.vecMyMarkers[i].first, g.vecMyMarkers[i].second, g.appInfo.mapsize(), g.appMap.width(), false, 0, 0);
					if (Distance(screenpos.x, screenpos.y, ev.button.x, ev.button.y) < dist)
					{
						dist = Distance(screenpos.x, screenpos.y, ev.button.x, ev.button.y);
						closest = i;
					}
				}
				if (dist < 20.f)
				{
					std::cout << "- marker" << std::endl;
					g.vecMyMarkers.erase(g.vecMyMarkers.begin() + closest);
					json["markers"].erase(closest);
					std::ofstream o(g.connectedServerFile);
					if (o.is_open())
					{
						o << std::setw(4) << json << std::endl;
						o.close();
					}
				}
				else
				{
					std::cout << "+ marker" << std::endl;
					auto worldpos = GetWorldPos(ev.button.x, ev.button.y, g.appInfo.mapsize(), g.appMap.width(), fMapZoom, fMapX, fMapY);
					SaveMarkersToJson(worldpos.first, worldpos.second);
				}
			}
			break;
		case SDL_MOUSEMOTION:
			//Panning
			if (ev.button.button == SDL_BUTTON_LEFT)
			{
				if (!g.connected)
					continue;
				if (!g.bFocus)
				{
					fMapX -= (ev.motion.x - g.lastX) / fMapZoom;
					fMapY -= (ev.motion.y - g.lastY) / fMapZoom;
				}
			}
			g.lastX = ev.motion.x;
			g.lastY = ev.motion.y;
			break;
		case SDL_KEYDOWN:
			//Follow toggle
			if (!g.connected)
				continue;
			if (ev.key.keysym.scancode == SDL_SCANCODE_F)
			{
				g.bFocus = !g.bFocus;
				std::cout << "Focus: " << (g.bFocus ? "Enabled" : "Disabled") << std::endl;
			}
			if (ev.key.keysym.scancode == SDL_SCANCODE_M)
			{
				std::cout << "+ marker" << std::endl;
				SaveMarkersToJson(g.localPlayer.x(), g.localPlayer.y());
			}
			break;
		}
	}
}

static SDL_Surface* cameraImage = SDL_CreateRGBSurface(0, 160, 90, 32, 0, 0, 0, 0);

void NetLoop()
{
	if (!g.connected)
		return;
	Events& lastEvents = g.lastEvents;
	Events& curEvents = g.curEvents;
	AppTeamChat& lastTeamChat = g.lastTeamChat;
	AppTeamChat& curTeamChat = g.curTeamChat;

	NStatus& NSmapMarkers = g.NSmapMarkers;
	NStatus& NSTeamChat = g.NSTeamChat;
	NStatus& NSTeamInfo = g.NSTeamInfo;
	AppMessage appMessage;
	rs->ws->poll();
	rs->ws->dispatch([&](const std::string& message) { appMessage.ParseFromString(message); });

	if (appMessage.has_broadcast() && appMessage.broadcast().has_camerarays())
	{
		DecodeCamera(160, 90, appMessage.broadcast().camerarays(), cameraRenderer);
	}
	if (NSmapMarkers == NWaiting || NSTeamChat == NWaiting || NSTeamInfo == NWaiting)
	{
		if (appMessage.has_response())
		{
			if (appMessage.response().has_mapmarkers())
			{
				NSmapMarkers = NReady;
				g.appMapMarkers = appMessage.response().mapmarkers();
				//std::cout << "Received MapMarkers!" << std::endl;
			}
			if (appMessage.response().has_teamchat())
			{
				NSTeamChat = NReady;
				curTeamChat = appMessage.response().teamchat();
				//std::cout << "Received TeamChat!" << std::endl;
			}
			if (appMessage.response().has_teaminfo())
			{
				NSTeamInfo = NReady;
				g.appTeamInfo = appMessage.response().teaminfo();
				//std::cout << "Received TeamInfo!" << std::endl;
			}
			if (appMessage.response().has_error() && !ignoreErrors)
				std::cout << appMessage.response().error().DebugString();
		}

	}
	auto& tNow = g.tNow;
	auto& tLast = g.tLast;
	tNow = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> deltaTime = tNow - tLast;
	float tDelta = deltaTime.count();
	if (tDelta > .7f)
	{
		NMessageType& msgType = g.msgType;
		auto request = rs->initProto();
		if (msgType == 0)
		{
			request.mutable_getmapmarkers()->CopyFrom(AppEmpty());
			NSmapMarkers = NWaiting;
			msgType = NTeamChat;
			//std::cout << "Sent MapMarkers!" << std::endl;
		}
		else if (msgType == NTeamChat)
		{
			request.mutable_getteamchat()->CopyFrom(AppEmpty());
			NSTeamChat = NWaiting;
			msgType = NTeamInfo;
			//std::cout << "Sent TeamChat!" << std::endl;
		}
		else if (msgType == NTeamInfo)
		{
			request.mutable_getteaminfo()->CopyFrom(AppEmpty());
			NSTeamInfo = NWaiting;
			msgType = NMarkers;
			//std::cout << "Sent TeamInfo!" << std::endl;
		}
		rs->ws->sendBinary(request.SerializeAsString());
		//Events
		/*g.vecOrders.clear();
		for (int i = 0; i < g.appMapMarkers.markers_size(); i++)
		{
			auto& marker = g.appMapMarkers.markers().Get(i);
			if (marker.type() == VendingMachine)
				for (int o = 0; o < marker.sellorders_size(); o++)
					g.vecOrders.emplace_back(marker.sellorders().Get(o));
		}*/
		curEvents = rs->GetEvents(g.appMapMarkers);
		if (curEvents != lastEvents)
		{
			if (curEvents.cCargo > lastEvents.cCargo && alertCargo)
			{
				std::string msg;
				msg += (curEvents.i_cargo.y() < g.appMap.height() * .5f ? "Cargo Bottom-" : "Cargo Top-");
				msg += (curEvents.i_cargo.x() < g.appMap.width() * .5f ? "Left" : "Right");
				rs->Message(msg, MsgIncludeTime | MsgBoth);
			}

			if (curEvents.cPatrol > lastEvents.cPatrol && alertPatrol)
				rs->Message("Patrol", MsgIncludeTime | MsgBoth);

			if (curEvents.cChinook > lastEvents.cChinook && alertChinook)
				rs->Message("Chinook", MsgIncludeTime | MsgBoth);

			if (curEvents.cExplosion > lastEvents.cExplosion && alertExplosion)
				rs->Message("Explosion", MsgIncludeTime | MsgBoth);

#pragma region CrateSpawn
			if (curEvents.cCrate > lastEvents.cCrate)
			{
				for (int i = 0; i < curEvents.cCrate; i++)
				{
					//If new crate
					if (!VectorContains(lastEvents.v_crates, curEvents.v_crates[i]) && alertCrate)
					{
						float dist = 9999;
						int closest = 0;
						bool isCargo = false;
						//Get Closest Monument
						for (int j = 0; j < g.appMap.monuments_size(); j++)
						{
							auto monument = g.appMap.monuments().Get(j);
							if (monument.token() == "DungeonBase" || monument.token() == "train_tunnel_display_name" || monument.token() == "swamp_a" || monument.token() == "swamp_b" || monument.token() == "swamp_c")
								continue;
							if (Distance(curEvents.v_crates[i].x(), curEvents.v_crates[i].y(), monument.x(), monument.y()) < dist)
							{
								dist = Distance(curEvents.v_crates[i].x(), curEvents.v_crates[i].y(), monument.x(), monument.y());
								closest = j;
							}
						}
						//or cargo
						if (Distance(curEvents.v_crates[i].x(), curEvents.v_crates[i].y(), curEvents.i_cargo.x(), curEvents.i_cargo.y()) < dist)
						{
							dist = Distance(curEvents.v_crates[i].x(), curEvents.v_crates[i].y(), curEvents.i_cargo.x(), curEvents.i_cargo.y());
							isCargo = true;
						}
						std::string msg = "Crate spawned - ";
						if (isCargo)
						{
							msg += "CARGO";
						}
						else
						{
							auto str = Garbage::dict.find(g.appMap.monuments().Get(closest).token());
							msg += str->second.c_str();
						}
						rs->Message(msg, MsgIncludeTime | MsgBoth);
					}
				}
			}
#pragma endregion CrateSpawn
#pragma region CrateDespawn
			else if (curEvents.cCrate < lastEvents.cCrate)
			{
				for (int i = 0; i < lastEvents.cCrate; i++)
				{
					//If new crate
					if (!VectorContains(curEvents.v_crates, lastEvents.v_crates[i]) && alertCrate)
					{
						float dist = 9999;
						int closest = 0;
						bool isCargo = false;
						//Get Closest Monument
						for (int j = 0; j < g.appMap.monuments_size(); j++)
						{
							auto monument = g.appMap.monuments().Get(j);
							if (monument.token() == "DungeonBase" || monument.token() == "train_tunnel_display_name")
								continue;
							if (Distance(lastEvents.v_crates[i].x(), lastEvents.v_crates[i].y(), monument.x(), monument.y()) < dist)
							{
								dist = Distance(lastEvents.v_crates[i].x(), lastEvents.v_crates[i].y(), monument.x(), monument.y());
								closest = j;
							}
						}
						//or cargo
						if (Distance(lastEvents.v_crates[i].x(), lastEvents.v_crates[i].y(), lastEvents.i_cargo.x(), lastEvents.i_cargo.y()) < dist)
						{
							dist = Distance(lastEvents.v_crates[i].x(), lastEvents.v_crates[i].y(), lastEvents.i_cargo.x(), lastEvents.i_cargo.y());
							isCargo = true;
						}
						std::string msg = "Crate looted - ";
						if (isCargo)
						{
							msg += "CARGO";
						}
						else
						{
							auto str = Garbage::dict.find(g.appMap.monuments().Get(closest).token());
							msg += str->second.c_str();
						}
						rs->Message(msg, MsgIncludeTime | MsgBoth);
					}
				}
			}
			lastEvents = curEvents;
		}
#pragma endregion CrateDespawn
#pragma region Commands
		//Commands
		if (curTeamChat.ByteSizeLong() && DifferentChats(curTeamChat, lastTeamChat))
		{
			AppTeamMessage msg = curTeamChat.messages().Get(curTeamChat.messages().size() - 1);
			if (msg.message() == "!help")
				rs->SendTeamChatMessage("commands: !time !promoteme !info !id64");
			else if (msg.message() == "!promoteme")
				rs->PromoteToTeamLeader(msg.steamid());
			else if (msg.message() == "!history")
			{
				//Event history + times
			}
			else if (msg.message() == "!pop")
			{
				g.appInfo = rs->GetInfo();
				rs->SendTeamChatMessage((std::string("Players: ") + std::to_string(g.appInfo.players()) + "/" + std::to_string(g.appInfo.maxplayers()) + " Queue: " + std::to_string(g.appInfo.queuedplayers())).c_str());
			}
			else if (msg.message() == "!info")
			{
				g.appInfo = rs->GetInfo();
				rs->SendTeamChatMessage((std::string("Map size: ") + std::to_string(g.appInfo.mapsize())).c_str());
				rs->SendTeamChatMessage((std::string("Players: ") + std::to_string(g.appInfo.players()) + "/" + std::to_string(g.appInfo.maxplayers()) + " Queue: " + std::to_string(g.appInfo.queuedplayers())).c_str());
			}
			else if (msg.message() == "!time")
			{
				auto time = rs->GetTime();
				Time tTime = FormatTime(time);
				bool isDay = time.time() > time.sunrise() && time.time() < time.sunset();
				float fDif = isDay ? time.sunset() - time.time() : time.sunrise() - time.time();
				if (fDif < 0.f)
					fDif += 24.f;
				Time tDif = FormatTime(fDif);
				std::string out = std::to_string(tTime.hours) + ":" + (tTime.minutes < 10 ? "0" : "") + std::to_string(tTime.minutes) + (isDay ? ", day" : ", night") + " for " + std::to_string(tDif.hours) + ":" + (tDif.minutes < 10 ? "0" : "") + std::to_string(tDif.minutes);
				rs->SendTeamChatMessage(out.c_str());
			}
			else if (msg.message() == "!id64")
			{
				rs->SendTeamChatMessage(std::to_string(msg.steamid()).c_str());
			}
			//else if (strstr(msg.message().c_str(), "!find"))
			//{
			//	if (msg.message().length() > 6)
			//	{
			//		std::string itemName = msg.message().substr(6);
			//		std::transform(itemName.begin(), itemName.end(), itemName.begin(), [](unsigned char c) { return std::tolower(c); });
			//		for (AppMarker_SellOrder order : g.vecOrders)
			//		{
			//			auto str = idToNames.find(itemName);
			//			if (str != idToNames.end())
			//				if (order.itemid() == str->second)
			//					for (auto it = idToNames.begin(); it != idToNames.end(); ++it)
			//						if (it->second == order.currencyid())
			//							rs->Message(std::to_string(order.costperitem()) + "x " + it->first, MsgLocal);
			//			//rs.Message((std::to_string(order.costperitem()) + "x " + it->first).c_str());
			//		}
			//	}
			//}
			else
			{
				if (msg.message()[0] != '[')
					rs->Message(msg.name() + ": " + msg.message(), MsgLocal | MsgIncludeTime);
			}
			lastTeamChat = curTeamChat;
		}
		if (g.input.size())
		{
			rs->Message(g.input, MsgChat);
			g.input.clear();
		}
#pragma endregion Commands
		//End of loop
		tLast = tNow = std::chrono::high_resolution_clock::now();
	}
}
bool Connect(std::string serverName)
{
	std::string file_path = "servers\\";
	file_path += serverName;
	std::ifstream istream(file_path);
	if (!istream)
	{
		std::cout << "Reading server json failed!\n";
		system("pause");
		exit(0);
	}
	istream >> json;
	istream.close();
	g.jIP = json["ip"];
	g.jPort = json["port"];
	g.jID = json["id"];
	g.jToken = json["token"];
	if (json.count("name"))
		g.connectedServerName = json["name"];
	LoadMarkersFromJson();
	rs = new RustSocket(g.jIP.c_str(), g.jPort.c_str(), g.jID, g.jToken);
	if (!rs->ws || rs->ws->getReadyState() == WebSocket::CLOSED)
		return 0;
	g.appMap = rs->GetMap();
	g.appInfo = rs->GetInfo();
	g.appMapMarkers = rs->GetMarkers();
	g.appTeamInfo = rs->GetTeamInfo();
	g.localPlayer = GetLocalPlayer();
	g.mapTexture = CreateMap();
	g.lastEvents = rs->GetEvents(g.appMapMarkers);
	g.lastTeamChat = rs->GetTeamChat();
	g.bFocus = false;
	g.NSmapMarkers = NReady;
	g.NSTeamChat = NReady;
	g.NSTeamInfo = NReady;
	g.msgType = NMarkers;
	g.tLast = std::chrono::high_resolution_clock::now();
	g.tNow = std::chrono::high_resolution_clock::now();
	g.connectedServerFile = file_path;
	SDL_SetWindowTitle(pWindow, (std::string("Rust+ PC | ") + (g.connectedServerName.size() ? g.connectedServerName : g.connectedServerFile.substr(8))).c_str());
	return true;
}
void Disconnect()
{
	if (!g.connected)
		return;
	g.fMapZoom = 1.f;
	g.fMapX = g.fMapY = MAP_SIZE_HALF;
	g.lastX = g.lastY = 0;
	//((RustSocket*)g.socket)->ws->close();
	g.socket = nullptr;
	g.connected = false;
	//serverbuttons - destroy + create
	//localplayer - depends on which dc method use i think
	g.jIP = "";
	g.jPort = "";
	g.jID = 0;
	g.jToken = 0;
	g.localPlayer.Clear();
	g.curEvents = Events();// -optional
	g.curTeamChat.Clear();
	g.NSmapMarkers = NReady;
	g.NSTeamChat = NReady;
	g.NSTeamInfo = NReady;
	g.tLast = std::chrono::high_resolution_clock::now();
	g.tNow = std::chrono::high_resolution_clock::now();
	g.msgType = NMarkers;
	g.nametags.clear();// optional
	//rustplus::AppTeamInfo_Member localPlayer; -maybe required when dc
	SDL_DestroyTexture(g.mapTexture);
	g.mapTexture = nullptr;// maybe mem leak
	g.appMap.Clear();
	g.appInfo.Clear();
	g.appMapMarkers.Clear();
	g.appTeamInfo.Clear();
	g.servers = GetServerlist();
	ResetServerButtons();
	g.connectedServerFile = "servers\\unconnected";
	g.connectedServerName = "";
	SDL_SetWindowTitle(pWindow, "Rust+ PC | unconnected");
}


void Render()
{
	SDL_RenderClear(g.pRenderer);
	if (g.connected)
	{
		Render_Map();
		Render_CustomMapMarkers();
		Render_Events();
		Render_Markers();
		Render_TeamMembers();
	}
	for (GUIElement* element : GUI::Elements())
	{
		if (Button* button = dynamic_cast<Button*>(element))
		{
			if (button->key.find("button_serverconnect") != std::string::npos)
				button->enabled = !g.connected;
			if (button->key.find("button_focus") != std::string::npos)
				button->enabled = g.connected;
		}
	}
	Render_GUI();
	SDL_RenderPresent(g.pRenderer);
}


void ResetServerButtons()
{
	auto& els = GUI::Elements();
	for (auto& element : els)
	{
		if (Button* button = dynamic_cast<Button*>(element))
		{
			if (button->key.find("button_serverconnect") != std::string::npos)
			{
				delete(element);
				element = nullptr;
			}
		}
	}
	els.erase(std::remove(std::begin(els), std::end(els), nullptr),
		std::end(els));

	g.servers = GetServerlist();
	for (int i = 0; i < g.servers.size(); i++)
	{
		Button* serverButton = new Button(std::string("button_serverconnect") + std::to_string(i), g.servers[i], g.fontTahomaBold, { 10, 10 + (i * 40), 250, 30 }, TopLeft, GUIColor::White, GUIColor::Gray, true);
		serverButton->OnClick([i]() { g.connected = Connect(g.servers[i]); });
	}
	g.skipRender = true;
}