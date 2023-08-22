#pragma once

#include "MapCreator.h"
#include "NetGameTypes.h"


#include <string>

namespace allinone
{

class Game
{
private:
    enum GameState
    {
        TitleScreen = 0,
        LoadNextLevel,
        StartLevelScreen,
        StartLevel,
        PlayingLevel,
        GameOver
    };

    GameState gameState;
    int currentLevel;
    Level::Properties _currentLevelProperties;
    int _surviveTimeoutSec;

    int framesToDie, framesToWin;

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
    void recalculateViewPort();
    void drawObjects();
    void updateDisplay();
    void checkStatePostFrame();
    void pause(bool);

    void drawTitleScreen();
    void drawStartLevelScreen();
    void drawGameOverScreen();

    bool winConditionsMet() const;
    bool failConditionsMet() const;
};

}