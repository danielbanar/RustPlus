#pragma once
#include "main.h"
#define MAX_FPS 60
extern bool ignoreErrors;
int main(int argc, char* argv[])
{
	if (argc != 6)
	{
		std::cerr << "Invalid Arguments: <ip> <port <steamid> <token> <camera>\n";
		return 1;
	}
	std::string ip = argv[1];
	std::string port = argv[2];
	uint64_t steamID64 = std::stoull(argv[3]);
	int32_t token = std::stoi(argv[4]);
	std::string camera = argv[5];

	if (SDL_Init(SDL_INIT_EVERYTHING) == -1 && !ignoreErrors)
		std::cerr << "Init error: " << SDL_GetError();

	SDL_Window* window = SDL_CreateWindow(camera.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 2 * 160, 2 * 80, SDL_WINDOW_SHOWN);
	if (!window && !ignoreErrors)
		std::cerr << "Window error: " << SDL_GetError();

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
	if (!renderer && !ignoreErrors)
		std::cerr << "Renderer error: " << SDL_GetError();

	TTF_Init();
	//Font init

	rs = new RustSocket(ip.c_str(), port.c_str(), steamID64, token);
	if (!rs->ws || rs->ws->getReadyState() == WebSocket::CLOSED)
		return 0;

	SDL_TimerID timer = SDL_AddTimer(0, SubscribeRepeat, const_cast<char*>(camera.c_str()));

	// Clear the renderer to black
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	SDL_RenderPresent(renderer);

	//Main loop
	bool bRunning = true;
	while (bRunning)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event) != 0)
		{
			switch (event.type)
			{
			case SDL_QUIT:
				bRunning = false;
				exit(0);
				break;
			}
		}

		AppMessage appMessage;
		rs->ws->poll();
		rs->ws->dispatch([&](const std::string& message) { appMessage.ParseFromString(message); });
		if (appMessage.has_broadcast() && appMessage.broadcast().has_camerarays())
		{
			DecodeCamera(160, 90, appMessage.broadcast().camerarays(), renderer);
		}

		Sleep(1000.f / (float)MAX_FPS);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	if (rs)
		rs->ws->close();
	TTF_Quit();
	SDL_Quit();
	return 0;
}

Uint32 SubscribeRepeat(Uint32 interval, void* name)
{
	rs->Subscribe(static_cast<char*>(name));
	std::cout << "sub\n";
	return 30000;
}