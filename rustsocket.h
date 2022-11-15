#pragma once
#include <iostream>
#include <fstream>
#include "easywsclient.hpp"
#include "rustplus.pb.h"
#include <map>
#ifdef _WIN32
#pragma comment( lib, "ws2_32" )
#include <WinSock2.h>
#include <assert.h>
#endif

#ifdef __WIN32__
# include <winsock2.h>
#else
# include <sys/types.h>
#endif

using namespace easywsclient;
using namespace rustplus;
//std::vector<char> buffer;
std::string trim(const std::string& str, const std::string& whitespace = " \t");
struct Time
{
	int hours, minutes;
};
enum MessageFlags
{
	MsgLocal = 0x01,
	MsgChat = 0x02,
	MsgBoth = 0x03,
	MsgIncludeTime = 0x04
};
struct Events
{
	int cCrate = 0;
	int cCargo = 0;
	int cExplosion = 0;
	int cChinook = 0;
	int cPatrol = 0;
	std::vector<AppMarker> v_crates;
	AppMarker i_cargo;
	bool operator==(const Events& other)
	{
		return cCrate == other.cCrate && cCargo == other.cCargo && cExplosion == other.cExplosion && cChinook == other.cChinook && cPatrol == other.cPatrol;
	}
	bool operator!=(const Events& other)
	{
		return cCrate != other.cCrate || cCargo != other.cCargo || cExplosion != other.cExplosion || cChinook != other.cChinook || cPatrol != other.cPatrol;
	}
	bool empty()
	{
		return !cCrate && !cCargo && !cExplosion && !cChinook && !cPatrol && v_crates.empty() && !i_cargo.ByteSizeLong();
	}
};
enum NStatus
{
	NReady,
	NWaiting
};
enum NMessageType
{
	NMarkers,
	NTeamChat,
	NTeamInfo
};
bool DifferentChats(const AppTeamChat& OLD, const AppTeamChat& NEW);
std::pair<int, int> FormatCoord(int x, int y, int map_size);
void SaveMap(const AppMap& map, const char* filename);
Time FormatTime(float fTime);
Time FormatTime(AppTime time);
float Distance(float x1, float y1, float x2, float y2);
bool VectorContains(std::vector<AppMarker>& vec, AppMarker element);
class RustSocket
{
public:
	WebSocket::pointer ws = nullptr;

	RustSocket(const char* ip, const char* port, uint64_t steamid, int32_t token);
	void SendTeamChatMessage(const char* message);
	AppInfo GetInfo();
	AppMap GetMap();
	std::string GetCameraFrame(std::string id, uint32_t frame);
	AppMapMarkers GetMarkers();
	AppTeamChat GetTeamChat();
	Events GetEvents(AppMapMarkers markers);
	AppTime GetTime();
	AppTeamInfo GetTeamInfo();
	void PromoteToTeamLeader(uint64_t steamid);
	AppRequest initProto();
	void Message(const std::string& msg, int flags);
private:
	std::string url = "ws://";
	uint32_t iSeq;
	uint64_t iSteamID;
	int32_t iPlayerToken;
};
