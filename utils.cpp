#include "utils.h"

void listen_for_input()
{
	while (g.bRunning)
	{
		std::cin >> g.input;
	}
}

std::vector<std::string> Filter(std::string str)
{
	std::vector<std::string> servers;
	while (str.find('{') != -1 && str.find('}') != -1)
	{
		int first = str.find('{');
		int last = str.find('}') + 1;
		std::string tp = str.substr(first, last - first - 2);
		if (tp.find("type: 'server'") != std::string::npos)
			servers.push_back(tp);
		str = str.substr(last);
	}
	return servers;
}

void Import()
{
	std::string data;
	std::ifstream input;
	input.open("C:/listen.temp", std::ios::binary);
	if (input.is_open()) {
		std::string tp;
		while (std::getline(input, tp))
			data += tp;
		input.close();
	}
	auto servers = Filter(data);
	for (std::string tp : servers)
	{
		std::string sport, sip, sname, sid, stoken;
		//port
		int first = tp.find("port: '");
		tp = tp.substr(first + 7);
		first = tp.find("ip: ");
		sport = tp.substr(0, first - 4);
		//ip
		tp = tp.substr(first + 5);
		first = tp.find("name: ");
		sip = tp.substr(0, first - 4);
		//name
		tp = tp.substr(first + 7);
		first = tp.find("logo: ");
		sname = tp.substr(0, first - 4);
		//id
		first = tp.find("playerId: ");
		tp = tp.substr(first + 11);
		first = tp.find("playerToken: ");
		sid = tp.substr(0, first - 4);
		//token
		tp = tp.substr(first + 14);
		stoken = tp;
		std::string filename = std::string("servers\\") + sname.substr(0, 24) + ".json";
		std::string illegals = "/?%*:|\"<>;=";
		for (char character : illegals)
			filename.erase(remove(filename.begin(), filename.end(), character), filename.end());
		if (file_exists(filename))
			continue;

		std::ofstream outfile(filename, std::ios::binary);
		if (outfile.is_open())
		{
			nlohmann::json j;
			j["ip"] = sip;
			j["port"] = sport;
			j["name"] = sname;
			std::istringstream iss(sid);
			iss >> j["id"];
			iss = std::istringstream(stoken);
			iss >> j["token"];
			outfile << std::setw(8) << j << std::endl;
			outfile.close();
		}
	}
	ResetServerButtons();
}

inline bool file_exists(const std::string& name) 
{
	if (FILE* file = fopen(name.c_str(), "r")) 
	{
		fclose(file);
		return true;
	}
	else 
	{
		return false;
	}
}

std::string string_to_hex(const std::string input)
{
	static const char hex_digits[] = "0123456789ABCDEF";

	std::string output;
	output.reserve(input.length() * 2);
	for (unsigned char c : input)
	{
		output.push_back(hex_digits[c >> 4]);
		output.push_back(hex_digits[c & 15]);
	}
	return output;
}

void set_pixel(SDL_Surface* surface, int x, int y, Uint32 pixel)
{
	Uint32* const target_pixel = (Uint32*)((Uint8*)surface->pixels
		+ y * surface->pitch
		+ x * surface->format->BytesPerPixel);
	*target_pixel = pixel;
}

AppTeamInfo_Member GetLocalPlayer()
{
	for (int i = 0; i < g.appTeamInfo.members_size(); i++)
	{
		AppTeamInfo_Member member = g.appTeamInfo.members().Get(i);
		if (member.steamid() == g.jID)
			return member;
	}
}

std::vector<std::string> GetServerlist()
{
	system("dir /b servers\\*.json > files.txt");
	std::ifstream newfile;
	newfile.open("files.txt");
	std::vector<std::string> servers;
	if (newfile.is_open()) {
		std::string tp;
		while (std::getline(newfile, tp)) {
			servers.push_back(tp);
		}
		newfile.close();
		remove("files.txt");
	}
	return servers;
}

SDL_Texture* CreateMap()
{
	SDL_Surface* mapSurf = IMG_LoadTyped_RW(SDL_RWFromMem((void*)g.appMap.jpgimage().data(), g.appMap.jpgimage().capacity()), 1, "JPG");
	SDL_Surface* trainSurf = IMG_LoadTyped_RW(SDL_RWFromMem(Icons::pngTrain, 7618), 1, "PNG");
	for (int i = 0; i < g.appMap.monuments_size(); i++)
	{
		auto monument = g.appMap.monuments().Get(i);
		if (monument.token() == "DungeonBase" || monument.token() == "swamp_a" || monument.token() == "swamp_b" || monument.token() == "swamp_c")
			continue;
		auto [sx, sy] = FormatCoord(monument.x(), monument.y(), g.appInfo.mapsize());
		auto str = Garbage::dict.find(monument.token());
		if (str != Garbage::dict.end())
		{
			if (monument.token() == "train_tunnel_display_name")
			{
				SDL_Rect rc = { sx - 12, sy - 12, 24, 24 };
				SDL_BlitScaled(Icons::trainSurf, 0, mapSurf, &rc);
			}
			else
			{
				SDL_Surface* surfText = TTF_RenderText_Solid(g.fontPermanentMarker, str->second.c_str(), { 0, 0, 0 });
				SDL_Rect textRect = { sx - surfText->w / 2, sy - surfText->h / 2, 0, 0 };
				SDL_BlitSurface(surfText, 0, mapSurf, &textRect);
				SDL_FreeSurface(surfText);
			}
		}
		else
		{
			std::cout << "Unknown monument:" << monument.token() << std::endl;
		}
	}
	SDL_FreeSurface(trainSurf);
	IMG_SaveJPG(mapSurf, "map.jpg", 100);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
	auto mapTexture = SDL_CreateTextureFromSurface(g.pRenderer, mapSurf);
	SDL_FreeSurface(mapSurf);
	return mapTexture;
}

void CreateTemplateFile()
{
	std::ofstream outfile("servers\\template.json");
	g.json["ip"] = "69.69.69.69";
	g.json["port"] = "420";
	g.json["id"] = 123456789;
	g.json["token"] = 1488;
	outfile << std::setw(8) << g.json << std::endl;
	outfile.close();
	std::cout << "Template file created!" << std::endl;
}

void LoadMarkersFromJson()
{
	g.vecMyMarkers.clear();
	for (size_t i = 0; i < g.json["markers"].size(); i++)
	{
		g.vecMyMarkers.push_back({ g.json["markers"][i]["x"], g.json["markers"][i]["y"] });
	}
}

void SaveMarkersToJson(int x, int y)
{
	g.vecMyMarkers.push_back({ x, y });
	g.json["markers"].emplace_back(nlohmann::json::object({ {"x", x}, {"y",  y } }));
	std::ofstream o(g.connectedServerFile);
	if (o.is_open())
	{
		o << std::setw(4) << g.json << std::endl;
		o.close();
	}
}

SDL_Rect GetRect(int X, int Y, int mapsize, int mapwidth, bool relative, float w, float h)
{
	float x = (X + 1000.f) * MAP_SIZE_HALF / mapwidth;
	float y = (mapsize - Y + 1000.f) * MAP_SIZE_HALF / mapwidth;
	if (relative)
		return {
			int(MAP_SIZE_HALF - (w * .5f * g.fMapZoom) + (x - g.fMapX) * g.fMapZoom),
			int(MAP_SIZE_HALF - (h * .5f * g.fMapZoom) + (y - g.fMapY) * g.fMapZoom),
			int(w * g.fMapZoom),
			int(h * g.fMapZoom) };
	else
		return {
			int(MAP_SIZE_HALF - w * .5f + (x - g.fMapX) * g.fMapZoom),
			int(MAP_SIZE_HALF - h * .5f + (y - g.fMapY) * g.fMapZoom),
			int(w),
			int(h) };
	return { 0 };
}

std::pair<float, float> GetWorldPos(float X, float Y, float mapsize, float mapwidth, float zoom, float mx, float my)
{
	return
	{ (X * mapwidth + mx * zoom * mapwidth - MAP_SIZE_HALF * mapwidth) / (MAP_SIZE_HALF * zoom) - 1000.f,
		-(Y * mapwidth + my * zoom * mapwidth - MAP_SIZE_HALF * mapwidth + zoom * mapwidth) / (zoom * MAP_SIZE_HALF) + 1005.f + mapsize
	};
}