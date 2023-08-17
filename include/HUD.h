#pragma once

#include <SFML/Graphics.hpp>


class HUD
{
    HUD();
    HUD(HUD &) = delete;
    void operator=(const HUD&) = delete;

    void drawPlayerXP(int baseY);
    void drawTankLives(int baseY);
    void drawTankUpgrades(int baseY);
    void drawBaseLives(int baseY);
    void drawBaseUpgrades(int baseY);
    void drawGlobalTimer();
    void drawWinScreen();
    void drawFailScreen();

    bool _showWin = false;
    bool _showFail = false;

    std::string _surviveTimeStr;
public:
    static HUD& instance();

    void draw();
    void showWin(bool val);
    void showFail(bool val);
};