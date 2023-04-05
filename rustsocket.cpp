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
bool RustSocket::Send(const std::string& message)
{
	if (send(sock, message.data(), message.size(), 0) == SOCKET_ERROR) 
	{
		std::cerr << "Error sending data: " << WSAGetLastError() << std::endl;
		closesocket(sock);
		WSACleanup();
		return false;
	}
	return true;
}
std::string RustSocket::Receive()
{
	std::string receivedData;
	char buffer[1024];
	int bytesReceived;
	do 
	{
		bytesReceived = recv(sock, buffer, sizeof(buffer), 0);
		if (bytesReceived < 0) 
		{
			std::cerr << "Error receiving data: " << WSAGetLastError() << std::endl;
			closesocket(sock);
			WSACleanup();
			return "";
		}
		receivedData.append(buffer, bytesReceived);
	} while (bytesReceived == sizeof(buffer));
	return receivedData;
}
RustSocket::RustSocket(const char* ip, uint16_t port, uint64_t steamid, int32_t token) : iSteamID(steamid), iPlayerToken(token), iSeq(1)
{
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result)
		std::cerr << "WSAStartup failed: " << result << std::endl;

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET)
	{
		std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
		WSACleanup();
	}

	sockaddr_in serverAddress;
	memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET; 
	serverAddress.sin_port = htons(port);
	inet_pton(AF_INET, ip, &serverAddress.sin_addr);

	std::cout << "Connecting to " << ip << ':' << port << "..." << std::endl;
	result = connect(sock, (SOCKADDR*)&serverAddress, sizeof(serverAddress));
	if (result == SOCKET_ERROR) 
	{
		std::cerr << "Error connecting to server: " << WSAGetLastError() << std::endl;
		closesocket(sock);
		WSACleanup();
	}
	else
		std::cout << "Connected!\n";

	std::string request = "GET /chat HTTP/1.1\r\n"
		"Host: " + std::string(ip) + ":" + std::to_string(port) + "\r\n"
		"Upgrade: websocket\r\n"
		"Connection: Upgrade\r\n"
		"Sec-WebSocket-Key: x3JJHMbDL1EzLkh9GBhXDw==\r\n"
		"Sec-WebSocket-Version: 13\r\n"
		"Origin: ws://" + std::string(ip) + ":" + std::to_string(port) + "\r\n"
		"\r\n";
	result = send(sock, request.c_str(), request.size(), 0);
	if (result == SOCKET_ERROR)
	{
		std::cerr << "send failed with error: " << WSAGetLastError() << std::endl;
		closesocket(sock);
		WSACleanup();
		return 1;
	}

	// Receive the server response to complete the WebSocket handshake
	char buffer[1024];
	result = recv(sock, buffer, sizeof(buffer), 0);
	if (result == SOCKET_ERROR)
	{
		std::cerr << "recv failed with error: " << WSAGetLastError() << std::endl;
		closesocket(sock);
		WSACleanup();
	}

}
void RustSocket::SendTeamChatMessage(const char* message)
{
	AppSendMessage msg;
	msg.set_message(message);
	auto request = initProto();
	request.mutable_sendteammessage()->CopyFrom(msg);
	Send(request.SerializeAsString());
	AppMessage appMessage;
	do
	{
		appMessage.ParseFromString(Receive());
	} while (!appMessage.has_response());
	if (appMessage.response().has_error() && !ignoreErrors)
		std::cout << appMessage.response().error().DebugString() << "SendTeamChatMessage\n\n";
}
void RustSocket::SetSubscription()
{
	AppFlag flag;
	flag.set_value(1);
	auto request = initProto();
	request.mutable_setsubscription()->CopyFrom(flag);
	Send(request.SerializeAsString());
	AppMessage appMessage;
	do
	{
		appMessage.ParseFromString(Receive());
	} while (!appMessage.has_response());
	if (appMessage.response().has_error() && !ignoreErrors)
		std::cout << appMessage.response().error().DebugString() << "SetSubscription\n\n";
}
void RustSocket::CheckSubscription()
{
	auto request = initProto();
	request.mutable_checksubscription()->CopyFrom(AppEmpty());
	std::string data = request.SerializeAsString();
	Send(request.SerializeAsString());
	AppMessage appMessage;
	do
	{
		appMessage.ParseFromString(Receive());
	} while (!appMessage.has_response());
	if (appMessage.response().has_error() && !ignoreErrors)
		std::cout << appMessage.response().error().DebugString() << "CheckSubscription\n\n";
	if (appMessage.has_broadcast())
		std::cout << "CheckSubscription";
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
	Send(request.SerializeAsString());
	AppMessage appMessage;
	do
	{
		appMessage.ParseFromString(Receive());
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
	Send(request.SerializeAsString());
	AppMessage appMessage;
	do
	{
		appMessage.ParseFromString(Receive());
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
	Send(request.SerializeAsString());
	AppMessage appMessage;
	do
	{
		std::string rec = Receive();
		std::cout << rec << std::endl;
		appMessage.ParseFromString(rec);
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
	Send(request.SerializeAsString());
	AppMessage appMessage;
	do
	{
		appMessage.ParseFromString(Receive());
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
	Send(request.SerializeAsString());
	AppMessage appMessage;
	do
	{
		appMessage.ParseFromString(Receive());
	} while ((!appMessage.has_response() || !appMessage.response().has_teamchat()) && !(appMessage.has_response() && appMessage.response().has_error()));
	if (appMessage.response().has_error() && !ignoreErrors)
		std::cout << appMessage.response().error().DebugString() << "GetTeamChat\n\n";
	if (appMessage.has_broadcast())
		std::cout << "GetTeamChatBroadcast";
	return appMessage.response().teamchat();
}
Events RustSocket::GetEvents(AppMapMarkers markers)
{
	Events m_func;
	for (size_t i = 0; i < markers.markers_size(); i++)
	{
		AppMarker marker = markers.markers().Get(i);

		if (marker.type() == Explosion)
			m_func.cExplosion++;
		else if (marker.type() == CH47)
			m_func.cChinook++;
		else if (marker.type() == PatrolHelicopter)
			m_func.cPatrol++;
		else if (marker.type() == CargoShip)
		{
			m_func.cCargo++;
			m_func.i_cargo = marker;
		}
		else if (marker.type() == Crate)
		{
			m_func.cCrate++;
			m_func.v_crates.emplace_back(marker);
		}
	}
	return m_func;
}

AppTime RustSocket::GetTime()
{
	auto request = initProto();
	request.mutable_gettime()->CopyFrom(AppEmpty());
	std::string data = request.SerializeAsString();
	Send(request.SerializeAsString());
	AppMessage appMessage;
	do
	{
		appMessage.ParseFromString(Receive());
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
	Send(request.SerializeAsString());
	AppMessage appMessage;
	do
	{
		appMessage.ParseFromString(Receive());
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
	Send(request.SerializeAsString());
	AppMessage appMessage;
	do
	{
		appMessage.ParseFromString(Receive());
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