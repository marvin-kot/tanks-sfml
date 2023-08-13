#include "GlobalConst.h"

int globalVars::borderWidth = 0;
int globalVars::borderHeight = 0;
sf::IntRect globalVars::gameViewPort = sf::IntRect();

sf::IntRect globalVars::mapViewPort = sf::IntRect(0, 0, globalConst::viewPortWidthPx,  globalConst::viewPortHeightPx);
sf::Vector2i globalVars::mapSize = sf::Vector2i(0, 0);
bool globalVars::globalTimeFreeze = false;
sf::Clock globalVars::globalFreezeClock = sf::Clock();
int globalVars::player1Lives = globalConst::InitialLives;
int globalVars::player1PowerLevel = globalConst::InitialPowerLevel;
int globalVars::player1XP = 0;
int globalVars::player1Level = 1;
bool globalVars::openLevelUpMenu = false;