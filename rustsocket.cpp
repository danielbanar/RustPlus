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
bool useMask = true;
bool RustSocket::SendBinary(const std::string& message)
{
	uint8_t type = 2;
	const uint8_t masking_key[4] = { 0, 0, 0, 0 };
	std::vector<uint8_t> header;
	header.assign(2 + (message.size() >= 126 ? 2 : 0) + (message.size() >= 65536 ? 6 : 0) + (useMask ? 4 : 0), 0);
	header[0] = 0x80 | type;
	if (false) {}
	else if (message.size() < 126) {
		header[1] = (message.size() & 0xff) | (useMask ? 0x80 : 0);
		if (useMask) {
			header[2] = masking_key[0];
			header[3] = masking_key[1];
			header[4] = masking_key[2];
			header[5] = masking_key[3];
		}
	}
	else if (message.size() < 65536) {
		header[1] = 126 | (useMask ? 0x80 : 0);
		header[2] = (message.size() >> 8) & 0xff;
		header[3] = (message.size() >> 0) & 0xff;
		if (useMask) {
			header[4] = masking_key[0];
			header[5] = masking_key[1];
			header[6] = masking_key[2];
			header[7] = masking_key[3];
		}
	}
	else { // TODO: run coverage testing here
		header[1] = 127 | (useMask ? 0x80 : 0);
		header[2] = (message.size() >> 56) & 0xff;
		header[3] = (message.size() >> 48) & 0xff;
		header[4] = (message.size() >> 40) & 0xff;
		header[5] = (message.size() >> 32) & 0xff;
		header[6] = (message.size() >> 24) & 0xff;
		header[7] = (message.size() >> 16) & 0xff;
		header[8] = (message.size() >> 8) & 0xff;
		header[9] = (message.size() >> 0) & 0xff;
		if (useMask) {
			header[10] = masking_key[0];
			header[11] = masking_key[1];
			header[12] = masking_key[2];
			header[13] = masking_key[3];
		}
	}
	std::vector<uint8_t> txbuf;
	// N.B. - txbuf will keep growing until it can be transmitted over the socket:
	txbuf.insert(txbuf.end(), header.begin(), header.end());
	txbuf.insert(txbuf.end(), message.begin(), message.end());
	if (useMask) {
		size_t message_offset = txbuf.size() - message.size();
		for (size_t i = 0; i != message.size(); ++i) 
		{
			txbuf[message_offset + i] ^= masking_key[i & 0x3];
		}
	}
	return send(sock, (char*)txbuf.data(), txbuf.size(), 0);
}


struct wsheader_type {
	unsigned header_size;
	bool fin;
	bool mask;
	enum opcode_type {
		CONTINUATION = 0x0,
		TEXT_FRAME = 0x1,
		BINARY_FRAME = 0x2,
		CLOSE = 8,
		PING = 9,
		PONG = 0xa,
	} opcode;
	int N0;
	uint64_t N;
	uint8_t masking_key[4];
};
bool isRxBad = false;
std::string RustSocket::receive_binary()
{
	std::string rxbuf;
	std::string receivedData;
	char buffer[1024];
	int bytesReceived;
	do {
		bytesReceived = recv(sock, buffer, sizeof(buffer), 0);
		if (bytesReceived == 0) {
			// EOF received, connection closed by server
			break;
		}
		else if (bytesReceived < 0) {
			std::cerr << "Error receiving data: " << WSAGetLastError() << std::endl;
			closesocket(sock);
			WSACleanup();
		}
		std::cout << rxbuf.size() << std::endl;
		rxbuf.append(buffer, bytesReceived);
	} while (true);

	if (isRxBad) {
		return "";
	}
	while (true) {
		wsheader_type ws;
		if (rxbuf.size() < 2) { return ""; /* Need at least 2 */ }
		const uint8_t* data = (uint8_t*)&rxbuf[0]; // peek, but don't consume
		ws.fin = (data[0] & 0x80) == 0x80;
		ws.opcode = (wsheader_type::opcode_type)(data[0] & 0x0f);
		ws.mask = (data[1] & 0x80) == 0x80;
		ws.N0 = (data[1] & 0x7f);
		ws.header_size = 2 + (ws.N0 == 126 ? 2 : 0) + (ws.N0 == 127 ? 8 : 0) + (ws.mask ? 4 : 0);
		if (rxbuf.size() < ws.header_size) { return ""; /* Need: ws.header_size - rxbuf.size() */ }
		int i = 0;
		if (ws.N0 < 126) {
			ws.N = ws.N0;
			i = 2;
		}
		else if (ws.N0 == 126) {
			ws.N = 0;
			ws.N |= ((uint64_t)data[2]) << 8;
			ws.N |= ((uint64_t)data[3]) << 0;
			i = 4;
		}
		else if (ws.N0 == 127) {
			ws.N = 0;
			ws.N |= ((uint64_t)data[2]) << 56;
			ws.N |= ((uint64_t)data[3]) << 48;
			ws.N |= ((uint64_t)data[4]) << 40;
			ws.N |= ((uint64_t)data[5]) << 32;
			ws.N |= ((uint64_t)data[6]) << 24;
			ws.N |= ((uint64_t)data[7]) << 16;
			ws.N |= ((uint64_t)data[8]) << 8;
			ws.N |= ((uint64_t)data[9]) << 0;
			i = 10;
			if (ws.N & 0x8000000000000000ull) {
				// https://tools.ietf.org/html/rfc6455 writes the "the most
				// significant bit MUST be 0."
				//
				// We can't drop the frame, because (1) we don't we don't
				// know how much data to skip over to find the next header,
				// and (2) this would be an impractically long length, even
				// if it were valid. So just close() and return immediately
				// for now.
				isRxBad = true;
				fprintf(stderr, "ERROR: Frame has invalid frame length. Closing.\n");
				return "";
			}
		}
		if (ws.mask) {
			ws.masking_key[0] = ((uint8_t)data[i + 0]) << 0;
			ws.masking_key[1] = ((uint8_t)data[i + 1]) << 0;
			ws.masking_key[2] = ((uint8_t)data[i + 2]) << 0;
			ws.masking_key[3] = ((uint8_t)data[i + 3]) << 0;
		}
		else {
			ws.masking_key[0] = 0;
			ws.masking_key[1] = 0;
			ws.masking_key[2] = 0;
			ws.masking_key[3] = 0;
		}

		// Note: The checks above should hopefully ensure this addition
		//       cannot overflow:
		if (rxbuf.size() < ws.header_size + ws.N) { return ""; /* Need: ws.header_size+ws.N - rxbuf.size() */ }

		// We got a whole message, now do something with it:
		if (false) {}
		else if (
			ws.opcode == wsheader_type::TEXT_FRAME
			|| ws.opcode == wsheader_type::BINARY_FRAME
			|| ws.opcode == wsheader_type::CONTINUATION
			) {
			if (ws.mask) { for (size_t i = 0; i != ws.N; ++i) { rxbuf[i + ws.header_size] ^= ws.masking_key[i & 0x3]; } }
			receivedData.insert(receivedData.end(), rxbuf.begin() + ws.header_size, rxbuf.begin() + ws.header_size + (size_t)ws.N);// just feed
			if (ws.fin) 
			{
				receivedData.erase(receivedData.begin(), receivedData.end());
				std::string().swap(receivedData);// free memory
			}
		}
		rxbuf.erase(rxbuf.begin(), rxbuf.begin() + ws.header_size + (size_t)ws.N);
	}
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

	std::string request = "GET / HTTP/1.1\r\n"
		"Host: " + std::string(ip) + ":" + std::to_string(port) + "\r\n"
		"Upgrade: websocket\r\n"
		"Connection: Upgrade\r\n"
		"Sec-WebSocket-Key: x3JJHMbDL1EzLkh9GBhXDw==\r\n"
		"Sec-WebSocket-Version: 13\r\n"
		"\r\n";

	result = send(sock, request.c_str(), request.size(), 0);
	if (result == SOCKET_ERROR)
	{
		std::cerr << "send failed with error: " << WSAGetLastError() << std::endl;
		closesocket(sock);
		WSACleanup();
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
	int flag = 1;
	setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag)); // Disable Nagle's algorithm

	u_long on = 1;
	ioctlsocket(sock, FIONBIO, &on);
}
void RustSocket::SendTeamChatMessage(const char* message)
{
	AppSendMessage msg;
	msg.set_message(message);
	auto request = initProto();
	request.mutable_sendteammessage()->CopyFrom(msg);
	SendBinary(request.SerializeAsString());
	AppMessage appMessage;
	do
	{
		appMessage.ParseFromString(receive_binary());
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
	SendBinary(request.SerializeAsString());
	AppMessage appMessage;
	do
	{
		appMessage.ParseFromString(receive_binary());
	} while (!appMessage.has_response());
	if (appMessage.response().has_error() && !ignoreErrors)
		std::cout << appMessage.response().error().DebugString() << "SetSubscription\n\n";
}
void RustSocket::CheckSubscription()
{
	auto request = initProto();
	request.mutable_checksubscription()->CopyFrom(AppEmpty());
	std::string data = request.SerializeAsString();
	SendBinary(request.SerializeAsString());
	AppMessage appMessage;
	do
	{
		appMessage.ParseFromString(receive_binary());
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
	SendBinary(request.SerializeAsString());
	AppMessage appMessage;
	do
	{
		appMessage.ParseFromString(receive_binary());
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
	SendBinary(request.SerializeAsString());
	AppMessage appMessage;
	do
	{
		appMessage.ParseFromString(receive_binary());
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
	SendBinary(request.SerializeAsString());
	AppMessage appMessage;
	do
	{
		std::string rec = receive_binary();
		std::cout << rec.size() << std::endl;
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
	SendBinary(request.SerializeAsString());
	AppMessage appMessage;
	do
	{
		appMessage.ParseFromString(receive_binary());
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
	SendBinary(request.SerializeAsString());
	AppMessage appMessage;
	do
	{
		appMessage.ParseFromString(receive_binary());
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
	SendBinary(request.SerializeAsString());
	AppMessage appMessage;
	do
	{
		appMessage.ParseFromString(receive_binary());
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
	SendBinary(request.SerializeAsString());
	AppMessage appMessage;
	do
	{
		appMessage.ParseFromString(receive_binary());
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
	SendBinary(request.SerializeAsString());
	AppMessage appMessage;
	do
	{
		appMessage.ParseFromString(receive_binary());
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