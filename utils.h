#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <nlohmann/json.hpp>
#include "rustplus.pb.h"
#include "rustsocket.h"
#include "main.h"
#include "resources.h"
#include "global.h"

void listen_for_input();
std::vector<std::string> Filter(std::string str);
void Import();
inline bool file_exists(const std::string& name);
std::string string_to_hex(const std::string input);
void set_pixel(SDL_Surface* surface, int x, int y, Uint32 pixel);
AppTeamInfo_Member GetLocalPlayer();
std::vector<std::string> GetServerlist();
SDL_Texture* CreateMap();
void CreateTemplateFile();
void LoadMarkersFromJson();
void SaveMarkersToJson(int x, int y);
SDL_Rect GetRect(int X, int Y, int mapsize, int mapwidth, bool relative, float w, float h);
std::pair<float, float> GetWorldPos(float X, float Y, float mapsize, float mapwidth, float zoom, float mx, float my);