#include "rustsocket.h"

bool ignoreErrors = false;
std::string buffer;
std::string trim(const std::string& str, const std::string& whitespace)
{
	const auto strBegin = str.find_first_not_of(whitespace);
	if (strBegin == std::string::npos)
		return ""; // no content

	const auto strEnd = str.find_last_not_of(whitespace);
	const auto strRange = strEnd - strBegin + 1;

	return str.substr(strBegin, strRange);
}
bool DifferentChats(const AppTeamChat& OLD, const AppTeamChat& NEW)
{
	//GetTeamChat returns only last 20 messages
	if (NEW.messages_size() != OLD.messages_size())
		return true;

	for (size_t i = 0; i < NEW.messages_size(); i++)
		if (NEW.messages().Get(i).time() != OLD.messages().Get(i).time())
			return true;

	return false;
	/*
	* Something wrong with this one too lazy to fix basically should return number of new messages
	if (NEW.messages_size() != OLD.messages_size())
		return NEW.messages_size() - OLD.messages_size();

	for (size_t i = 0; i < NEW.messages_size(); i++)
	{
		if (NEW.messages().Get(i).time() != OLD.messages().Get(i).time())
		{
			for (int j = 1; j <= OLD.messages_size(); j++)
			{
				if (NEW.messages().Get(NEW.messages_size() - 1).time() == OLD.messages().Get(OLD.messages_size()-1- j).time())
					return j;
			}
		}
	}

	return 0;*/
}
std::pair<int, int> FormatCoord(int x, int y, int map_size)
{
	y = map_size - y;
	x += 1000;
	y += 1000;
	//float scale = 5000.f / winSize;
	x /= 2;
	y /= 2;
	return std::make_pair(x, y);
}
void SaveMap(const AppMap& map, const char* filename)
{
	std::ofstream out(filename, std::ios::binary);
	out.write(map.jpgimage().data(), map.jpgimage().capacity());
}
Time FormatTime(float fTime)
{
	return { int(fTime), int((fTime - floorf(fTime)) * 60.f) };
}
Time FormatTime(AppTime time)
{
	return { int(time.time()), int((time.time() - floorf(time.time())) * 60.f) };
}
float Distance(float x1, float y1, float x2, float y2)
{
	return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}
bool VectorContains(std::vector<AppMarker>& vec, AppMarker element)
{
	for (const AppMarker& el : vec)
	{
		if (el.id() == element.id())
			return true;
	}
	return false;
}

RustSocket::RustSocket(const char* ip, const char* port, uint64_t steamid, int32_t token) : iSteamID(steamid), iPlayerToken(token), iSeq(1)
{
#ifdef _WIN32
	INT rc;
	WSADATA wsaData;
	rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (rc)
		std::cout << "WSAStartup Failed.\n";
#endif
	url += ip;
	url += ':';
	url += port;
	std::cout << "Connecting to " << url << "..." << std::endl;
	ws = WebSocket::from_url(url);
	if (ws)
		std::cout << "Connected!\n";
}
void RustSocket::SendTeamChatMessage(const char* message)
{
	AppSendMessage msg;
	msg.set_message(message);
	auto request = initProto();
	request.mutable_sendteammessage()->CopyFrom(msg);
	ws->sendBinary(request.SerializeAsString());
	AppMessage appMessage;
	do
	{
		ws->poll(-1);
		ws->dispatch([&](const std::string& message) { appMessage.ParseFromString(message); });
	} while (!appMessage.has_response());
	if (appMessage.response().has_error() && !ignoreErrors)
		std::cout << appMessage.response().error().DebugString() << "SendTeamChatMessage\n\n";
}
AppCameraInfo RustSocket::Subscribe(const char* camid)
{
	AppCameraSubscribe sub;
	sub.set_cameraid(camid);
	AppFlag flag;
	flag.set_value(true);
	auto request = initProto();
	//request.mutable_setsubscription()->CopyFrom(flag);
	request.mutable_camerasubscribe()->CopyFrom(sub);
	ws->sendBinary(request.SerializeAsString());
	std::cout << "Sent";
	AppMessage appMessage;
	do
	{
		ws->poll(-1);
		ws->dispatch([&](const std::string& message) { appMessage.ParseFromString(message); });
	} while (!(appMessage.has_response() || appMessage.has_broadcast()));
	if (appMessage.response().has_error() && !ignoreErrors)
		std::cout << appMessage.response().error().DebugString() << "Subscribe\n\n";
	return appMessage.response().camerasubscribeinfo();
}
AppInfo RustSocket::GetInfo()
{
	auto request = initProto();
	request.mutable_getinfo()->CopyFrom(AppEmpty());
	std::string data = request.SerializeAsString();
	ws->sendBinary(data);
	AppMessage appMessage;
	do
	{
		ws->poll(-1);
		ws->dispatch([&](const std::string& message) { appMessage.ParseFromString(message); });
	} while (!appMessage.has_response() || !appMessage.response().has_info());
	if (appMessage.response().has_error() && !ignoreErrors)
		std::cout << appMessage.response().error().DebugString() << "GetInfo\n\n";
	if (appMessage.has_broadcast())
		std::cout << "GetInfoBroadcast";
	return appMessage.response().info();
}
AppMap RustSocket::GetMap()
{
	auto request = initProto();
	request.mutable_getmap()->CopyFrom(AppEmpty());
	std::string data = request.SerializeAsString();
	ws->sendBinary(data);
	AppMessage appMessage;
	do
	{
		ws->poll(-1);
		ws->dispatch([&](const std::string& message) { appMessage.ParseFromString(message); });
	} while ((!appMessage.has_response() || !appMessage.response().has_map()) && !(appMessage.has_response() && appMessage.response().has_error()));
	if (appMessage.response().has_error() && !ignoreErrors)
		std::cout << appMessage.response().error().DebugString() << "GetMap\n\n";
	return appMessage.response().map();
}
AppMapMarkers RustSocket::GetMarkers()
{
	auto request = initProto();
	request.mutable_getmapmarkers()->CopyFrom(AppEmpty());
	std::string data = request.SerializeAsString();
	ws->sendBinary(data);
	AppMessage appMessage;
	do
	{
		ws->poll(-1);
		ws->dispatch([&](const std::string& message) { appMessage.ParseFromString(message); });
	} while (!appMessage.has_response() || !appMessage.response().has_mapmarkers());
	if (appMessage.response().has_error() && !ignoreErrors)
		std::cout << appMessage.response().error().DebugString() << "GetMarkers\n\n";
	if (appMessage.has_broadcast())
		std::cout << "GetMarkersBroadcast";
	return appMessage.response().mapmarkers();
}
AppTeamChat RustSocket::GetTeamChat()
{
	auto request = initProto();
	request.mutable_getteamchat()->CopyFrom(AppEmpty());
	std::string data = request.SerializeAsString();
	ws->sendBinary(data);
	AppMessage appMessage;
	do
	{
		ws->poll(-1);
		ws->dispatch([&](const std::string& message) { appMessage.ParseFromString(message); });
	} while ((!appMessage.has_response() || !appMessage.response().has_teamchat()) && !(appMessage.has_response() && appMessage.response().has_error()));
	if (appMessage.response().has_error() && !ignoreErrors)
		std::cout << appMessage.response().error().DebugString() << "GetTeamChat\n\n";
	if (appMessage.has_broadcast())
		std::cout << "GetTeamChatBroadcast";
	return appMessage.response().teamchat();
}
Events RustSocket::GetEvents(AppMapMarkers markers)
{
	Events ev;
	for (size_t i = 0; i < markers.markers_size(); i++)
	{
		AppMarker marker = markers.markers().Get(i);

		if (marker.type() == Explosion)
			ev.cExplosion++;
		else if (marker.type() == CH47)
			ev.cChinook++;
		else if (marker.type() == PatrolHelicopter)
			ev.cPatrol++;
		else if (marker.type() == CargoShip)
		{
			ev.cCargo++;
			ev.i_cargo = marker;
		}
		else if (marker.type() == Crate)
		{
			ev.cCrate++;
			ev.v_crates.emplace_back(marker);
		}
	}
	return ev;
}

AppTime RustSocket::GetTime()
{
	auto request = initProto();
	request.mutable_gettime()->CopyFrom(AppEmpty());
	std::string data = request.SerializeAsString();
	ws->sendBinary(data);
	AppMessage appMessage;
	do
	{
		ws->poll(-1);
		ws->dispatch([&](const std::string& message) { appMessage.ParseFromString(message); });
	} while (!appMessage.has_response() || !appMessage.response().has_time());
	if (appMessage.response().has_error() && !ignoreErrors)
		std::cout << appMessage.response().error().DebugString() << "GetTime\n\n";
	if (appMessage.has_broadcast())
		std::cout << "GetTimeBroadcast";
	return appMessage.response().time();
}
AppTeamInfo RustSocket::GetTeamInfo()
{
	auto request = initProto();
	request.mutable_getteaminfo()->CopyFrom(AppEmpty());
	std::string data = request.SerializeAsString();
	ws->sendBinary(data);
	AppMessage appMessage;
	do
	{
		ws->poll(-1);
		ws->dispatch([&](const std::string& message) { appMessage.ParseFromString(message); });
	} while (!appMessage.has_response() || !appMessage.response().has_teaminfo());
	if (appMessage.response().has_error() && !ignoreErrors)
		std::cout << appMessage.response().error().DebugString() << "GetTeamInfo\n\n";
	if (appMessage.has_broadcast())
		std::cout << "GetTeamInfoBroadcast";
	return appMessage.response().teaminfo();
}
void RustSocket::PromoteToTeamLeader(uint64_t steamid)
{
	AppPromoteToLeader leaderPacket;
	leaderPacket.set_steamid(steamid);

	auto request = initProto();
	request.mutable_promotetoleader()->CopyFrom(leaderPacket);
	std::string data = request.SerializeAsString();
	ws->sendBinary(data);
	AppMessage appMessage;
	do
	{
		ws->poll(-1);
		ws->dispatch([&](const std::string& message) { appMessage.ParseFromString(message); });
	} while (!appMessage.has_response());
	if (appMessage.response().has_error() && !ignoreErrors)
		std::cout << appMessage.response().error().DebugString() << "PromoteToTeamLeader\n\n";
}
AppRequest RustSocket::initProto()
{
	AppRequest request;
	request.set_seq(iSeq);
	iSeq++;
	request.set_playerid(iSteamID);
	request.set_playertoken(iPlayerToken);
	return request;
}
void RustSocket::Message(const std::string& msg, int flags)
{
	std::string message;
	if (flags & MsgIncludeTime)
	{
		char timeBuffer[64];
		auto time = std::time(0);
		std::strftime(timeBuffer, 64, "[%T] ", std::localtime(&time));
		message += timeBuffer;
	}
	message += msg;
	if (flags & MsgLocal)
		std::cout << message << std::endl;
	if (flags & MsgChat)
		SendTeamChatMessage(message.c_str());
}