#pragma once

#include "MapCreator.h"
#include "GlobalTypes.h"
#include "NetGameTypes.h"

#include <SFML/Graphics.hpp>
#include <SFML/System/Time.hpp>
#include <string>

namespace allinone
{

class Game
{
private:
    globalTypes::GameState gameState;
    int currentLevel;
    Level::Properties _currentLevelProperties;
    int framesToDie, framesToWin;

    int _killsCount;
    sf::Time _finishTime;
    bool _won = false;

public:
    Game();

    bool loadAssets();
    bool initializeWindow();
    void initializeVariables();
    bool update();

    bool isWindowOpen() const;

private:
    void updateFrameClock();
    void processWindowEvents();
    int processStateChange();
    bool buildLevelMap(std::string);
    void updateAllObjectControllers();
    void processDeletedObjects();
    void drawGameScreen();
    void drawBorders();
    void recalculateViewPort();
    void drawObjects();
    void updateDisplay();
    void checkStatePostFrame();
    void pause(bool);

    void drawStartLevelScreen();

    bool winConditionsMet() const;
    bool failConditionsMet() const;
};

}