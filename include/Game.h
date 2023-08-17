#pragma once

#include <string>

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

    GameState gameState = TitleScreen;
    int currentLevel;
    std::string _currentLevelName;
    std::string _currentLevelGoal;
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
};