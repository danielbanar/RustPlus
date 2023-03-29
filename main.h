#pragma once
#include <tuple>
#include <array>

void GUIInteraction(const SDL_Event& m_func);
void PollEvents();
void NetLoop();
bool Connect(std::string serverName);
void Disconnect();
void Render();
void ResetServerButtons();
extern bool ignoreErrors;