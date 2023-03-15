#include <cstring>
#include <SDL.h>
#include "global.h"

namespace Icons
{
	extern char* pngTrain;
	extern char* pngCrate;
	extern char* pngExplosion;
	extern char* pngCargo;
	extern char* pngChinook;
	extern char* pngBlade;
	extern char* pngMarke;
	extern char* pngDeath;
	extern char* pngPlayer;
	extern char* pngPlayerDead;
	extern char* pngPlayerOffline;

	extern SDL_Surface* trainSurf;
	extern SDL_Surface* myMarkerSurf;

	extern SDL_Texture* crate;
	extern SDL_Texture* explosion;
	extern SDL_Texture* cargo;
	extern SDL_Texture* chinook;
	extern SDL_Texture* blade;
	extern SDL_Texture* marker;
	extern SDL_Texture* death;
	extern SDL_Texture* player;
	extern SDL_Texture* playerDead;
	extern SDL_Texture* playerOff;
	extern SDL_Texture* myMarker;

	SDL_Texture* CreateTexture(char* png, int size);
	SDL_Texture* CreateTextureFromSurf(SDL_Surface* surf);
	void CreateTextures();
};

namespace Fonts
{
	extern char* PermanentMarker;
	extern char* HelsinkiMedium;
};

namespace Garbage
{
	extern std::map<std::string, std::string> dict;
}