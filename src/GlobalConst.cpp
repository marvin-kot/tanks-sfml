#include "GlobalConst.h"

int globalVars::borderWidth = 0;
int globalVars::borderHeight = 0;
sf::IntRect globalVars::gameViewPort = sf::IntRect();

sf::IntRect globalVars::mapViewPort = sf::IntRect(0, 0, globalConst::viewPortWidthPx,  globalConst::viewPortHeightPx);
sf::Vector2i globalVars::mapSize = sf::Vector2i(0, 0);
bool globalVars::gameIsPaused = false;
bool globalVars::globalTimeFreeze = false;

sftools::Chronometer globalVars::globalChronometer = sftools::Chronometer();
sftools::Chronometer globalVars::globalFreezeChronometer = sftools::Chronometer();
int globalVars::globalFreezeTimeout = 10;
int globalVars::player1Lives = globalConst::InitialLives;
int globalVars::player1PowerLevel = globalConst::InitialPowerLevel;
int globalVars::player1XP = 0;
int globalVars::player1Level = 1;
bool globalVars::openLevelUpMenu = false;