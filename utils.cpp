#include "utils.h"

void Utils::ListenForInput()
{
	while (g.bRunning)
	{
		char str[128] = {0};
		fgets(str, 128, stdin);
		int len = strlen(str);
		if (str[len - 1] == '\n') 
			str[len - 1] = '\0';
		//if(scanf("%[^\n]%*c", str))
			g.input = str;
	}
}

std::vector<std::string> Utils::Filter(std::string str)
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

void Utils::Import()
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
		std::string filename = std::string("servers\\") + sname.substr(0, 64) + ".json";
		std::string illegals = "/?%*:|\"<>;=";
		for (char character : illegals)
			filename.erase(remove(filename.begin(), filename.end(), character), filename.end());
		if (FileExists(filename))
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

inline bool Utils::FileExists(const std::string& name)
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


AppTeamInfo_Member Utils::GetLocalPlayer()
{
	for (int i = 0; i < g.appTeamInfo.members_size(); i++)
	{
		AppTeamInfo_Member member = g.appTeamInfo.members().Get(i);
		if (member.steamid() == g.jID)
			return member;
	}
	return AppTeamInfo_Member();
}

std::vector<std::string> Utils::GetServerlist()
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

SDL_Texture* Utils::CreateMap()
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
	auto mapTexture = SDL_CreateTextureFromSurface(g.mainRenderer, mapSurf);
	SDL_FreeSurface(mapSurf);
	return mapTexture;
}

void Utils::CreateTemplateFile()
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

void Utils::LoadMarkersFromJson()
{
	g.vecMyMarkers.clear();
	for (size_t i = 0; i < g.json["markers"].size(); i++)
	{
		g.vecMyMarkers.push_back({ g.json["markers"][i]["x"], g.json["markers"][i]["y"] });
	}
}

void Utils::SaveMarkersToJson(int x, int y)
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

SDL_Rect Utils::GetRect(int X, int Y, int mapsize, int mapwidth, bool relative, float w, float h)
{
	float x = (X + 1000.f) * (g.fWindowHeight / 2.f) / mapwidth;
	float y = (mapsize - Y + 1000.f) * (g.fWindowHeight / 2.f) / mapwidth;
	if (relative)
		return {
			int((g.fWindowWidth / 2.f) - (w * .5f * g.fMapZoom) + (x - g.fMapX) * g.fMapZoom),
			int((g.fWindowHeight / 2.f) - (h * .5f * g.fMapZoom) + (y - g.fMapY) * g.fMapZoom),
			int(w * g.fMapZoom),
			int(h * g.fMapZoom) };
	else
		return {
			int((g.fWindowWidth / 2.f) - w * .5f + (x - g.fMapX) * g.fMapZoom),
			int((g.fWindowHeight / 2.f) - h * .5f + (y - g.fMapY) * g.fMapZoom),
			int(w),
			int(h) };
	return { 0 };
}

std::pair<float, float> Utils::GetWorldPos(float X, float Y, float mapsize, float mapwidth, float zoom, float mx, float my)
{
	return
	{ (g.fWindowWidth /g.fWindowHeight)*(X * mapwidth + mx * zoom * mapwidth - (g.fWindowWidth / 2.f) * mapwidth) / ((g.fWindowWidth / 2.f) * zoom) - 1000.f,
		-(Y * mapwidth + my * zoom * mapwidth - (g.fWindowHeight / 2.f) * mapwidth + zoom * mapwidth) / (zoom * (g.fWindowHeight / 2.f)) + 1005.f + mapsize
	};
}
bool Utils::CreateNametagTexture(const std::string& name, SDL_Renderer* renderer, std::map<std::string, Texture>& nametags, SDL_Color color)
{
	if (name.empty())
		return 1;
	TTF_SetFontOutline(g.fontTahoma, 1);
	auto surfOutline = TTF_RenderText_Solid(g.fontTahoma, name.c_str(), { 0, 0, 0 });
	TTF_SetFontOutline(g.fontTahoma, 0);
	auto surfText = TTF_RenderText_Solid(g.fontTahoma, name.c_str(), color);
	if (!ignoreErrors)
		std::cerr << TTF_GetError();
	SDL_Surface* surfName = SDL_CreateRGBSurface(0, surfOutline->w, surfOutline->h, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
	SDL_Rect rectName = { 1, 1, surfText->w, surfText->h };
	SDL_BlitSurface(surfOutline, 0, surfName, 0);
	SDL_BlitSurface(surfText, 0, surfName, &rectName);
	SDL_FreeSurface(surfOutline);
	SDL_FreeSurface(surfText);
	nametags.emplace(std::make_pair(name.c_str(), Texture{ SDL_CreateTextureFromSurface(renderer, surfName), surfName->w, surfName->h }));
	SDL_FreeSurface(surfName);
	return 0;
}
