#pragma once
#include <tuple>
#include <array>

void GUIInteraction(const SDL_Event& ev);
void PollEvents();
void NetLoop();
bool Connect(std::string serverName);
void Disconnect();
void Render();
void ResetServerButtons();
extern bool ignoreErrors;