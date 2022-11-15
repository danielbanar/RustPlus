#pragma once
#include <tuple>
#include <array>
void CreateTemplateFile();
void LoadMarkersFromJson();
void SaveMarkersToJson(int x, int y);
inline SDL_Rect GetRect(int X, int Y, int mapsize, int mapwidth, bool relative, float w, float h);
std::pair<float, float> GetWorldPos(float X, float Y, float mapsize, float mapwidth, float zoom, float mx, float my);
void GUIInteraction(const SDL_Event& ev);
SDL_Texture* CreateMap();
AppTeamInfo_Member GetLocalPlayer();
std::vector<std::string> GetServerlist();
void PollEvents();
void NetLoop();
bool Connect(std::string serverName);
void Disconnect();
void Import();
void Render();
void Render_Map();
void Render_CustomMapMarkers();
void Render_Events();
void Render_Markers();
void Render_TeamMembers();
void Render_GUI();
void ResetServerButtons();
extern bool ignoreErrors;
std::vector<std::string> Filter(std::string str);
void Tits(std::string server);